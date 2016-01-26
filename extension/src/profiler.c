#include "quanta_mon.h"

#if PHP_VERSION_ID < 50500
/* Pointer to the original execute function */
static ZEND_DLEXPORT void (*_zend_execute) (zend_op_array *ops TSRMLS_DC);

/* Pointer to the origianl execute_internal function */
static ZEND_DLEXPORT void (*_zend_execute_internal) (zend_execute_data *data,
                           int ret TSRMLS_DC);
#else
/* Pointer to the original execute function */
static void (*_zend_execute_ex) (zend_execute_data *execute_data TSRMLS_DC);

/* Pointer to the origianl execute_internal function */
static void (*_zend_execute_internal) (zend_execute_data *data,
                      struct _zend_fcall_info *fci, int ret TSRMLS_DC);
#endif

/* Pointer to the original compile function */
static zend_op_array * (*_zend_compile_file) (zend_file_handle *file_handle,
                                              int type TSRMLS_DC);

/* Pointer to the original compile string function (used by eval) */
static zend_op_array * (*_zend_compile_string) (zval *source_string, char *filename TSRMLS_DC);


static void restore_original_zend_execute(void) {
  /* Remove proxies, restore the originals */
  #if PHP_VERSION_ID < 50500
    zend_execute          = _zend_execute;
  #else
    zend_execute_ex       = _zend_execute_ex;
  #endif
    zend_execute_internal = _zend_execute_internal;
    zend_compile_file     = _zend_compile_file;
    zend_compile_string   = _zend_compile_string;
}

/**
 * This function gets called once when quanta_mon gets enabled.
 * It replaces all the functions like zend_execute, zend_execute_internal,
 * etc that needs to be instrumented with their corresponding proxies.
 */
static void hp_begin(long level, long quanta_mon_flags TSRMLS_DC) {
  if (!hp_globals.enabled) {
    int hp_profile_flag = 1;

    hp_globals.enabled      = 1;
    hp_globals.quanta_mon_flags = (uint32)quanta_mon_flags;

    /* Replace zend_compile with our proxy */
    _zend_compile_file = zend_compile_file;
    zend_compile_file  = hp_compile_file;

    /* Replace zend_compile_string with our proxy */
    _zend_compile_string = zend_compile_string;
    zend_compile_string = hp_compile_string;

    /* Replace zend_execute with our proxy */
#if PHP_VERSION_ID < 50500
    _zend_execute = zend_execute;
    zend_execute  = hp_execute;
#else
    _zend_execute_ex = zend_execute_ex;
    zend_execute_ex  = hp_execute_ex;
#endif

    /* Replace zend_execute_internal with our proxy */
    _zend_execute_internal = zend_execute_internal;
    if (!(hp_globals.quanta_mon_flags & QUANTA_MON_FLAGS_NO_BUILTINS)) {
      /* if NO_BUILTINS is not set (i.e. user wants to profile builtins),
       * then we intercept internal (builtin) function calls.
       */
      zend_execute_internal = hp_execute_internal;
    }

    /* Initialize with the dummy mode first Having these dummy callbacks saves
     * us from checking if any of the callbacks are NULL everywhere. */
    hp_globals.mode_cb.init_cb     = hp_mode_dummy_init_cb;
    hp_globals.mode_cb.exit_cb     = hp_mode_dummy_exit_cb;
    hp_globals.mode_cb.begin_fn_cb = hp_mode_dummy_beginfn_cb;
    hp_globals.mode_cb.end_fn_cb   = hp_mode_dummy_endfn_cb;

    /* Register the appropriate callback functions Override just a subset of
     * all the callbacks is OK. */
    switch(level) {
      case QUANTA_MON_MODE_HIERARCHICAL:
        hp_globals.mode_cb.begin_fn_cb = hp_mode_hier_beginfn_cb;
        hp_globals.mode_cb.end_fn_cb   = hp_mode_hier_endfn_cb;
        break;
      case QUANTA_MON_MODE_SAMPLED:
        hp_globals.mode_cb.init_cb     = hp_mode_sampled_init_cb;
        hp_globals.mode_cb.begin_fn_cb = hp_mode_sampled_beginfn_cb;
        hp_globals.mode_cb.end_fn_cb   = hp_mode_sampled_endfn_cb;
        break;
      case QUANTA_MON_MODE_MAGENTO_PROFILING:
        hp_globals.mode_cb.begin_fn_cb = hp_mode_magento_profil_beginfn_cb;
        hp_globals.mode_cb.end_fn_cb   = hp_mode_magento_profil_endfn_cb;
      case QUANTA_MON_MODE_EVENTS_ONLY:
        hp_globals.mode_cb.begin_fn_cb = hp_mode_events_only_beginfn_cb;
        hp_globals.mode_cb.end_fn_cb   = hp_mode_events_only_endfn_cb;
        break;
    }

    /* one time initializations */
    hp_init_profiler_state(level TSRMLS_CC);

    /* start profiling from fictitious main() */
    BEGIN_PROFILING(&hp_globals.entries, ROOT_SYMBOL, hp_profile_flag, "main", NULL);
  }
}


/**
 * Called at request shutdown time. Cleans the profiler's global state.
 */
static void hp_end(TSRMLS_D) {
  /* Bail if not ever enabled */
  if (!hp_globals.ever_enabled) {
    return;
  }

  /* Stop profiler if enabled */
  if (hp_globals.enabled) {
    hp_stop(TSRMLS_C);
  }

  /* Clean up state */
  hp_clean_profiler_state(TSRMLS_C);
}

/**
 * Called from quanta_mon_disable(). Removes all the proxies setup by
 * hp_begin() and restores the original values.
 */
static void hp_stop(TSRMLS_D) {
  char  *bufout;
  int   hp_profile_flag = 1, fd_log_out;

  /* End any unfinished calls */
  while (hp_globals.entries)
    END_PROFILING(&hp_globals.entries, hp_profile_flag, NULL);
  restore_original_zend_execute();
  /* Resore cpu affinity. */
  restore_cpu_affinity(&hp_globals.prev_mask);
  /* Stop profiling */
  hp_globals.enabled = 0;
  if ((bufout = emalloc(OUTBUF_QUANTA_SIZE))) {
    if ((fd_log_out = init_output(bufout)) > 0) {
      float cpufreq;
      monikor_metric_list_t *metrics;
      struct timeval now;

      if (!hp_globals.cpu_frequencies
      || !(metrics = monikor_metric_list_new())) {
        PRINTF_QUANTA("Cannot initialize profling\n");
        return;
      }
      cpufreq = hp_globals.cpu_frequencies[hp_globals.cur_cpu_id];
      gettimeofday(&now, NULL);
      begin_output(fd_log_out);
      profiler_output(bufout, fd_log_out, &now, metrics, cpufreq);
      blocks_output(bufout, fd_log_out, cpufreq);
      end_output(bufout, fd_log_out);
      close(fd_log_out);
      send_data_to_monikor(metrics);
      monikor_metric_list_free(metrics);
    }
    efree(bufout);
  }
}

/**
 * Initialize profiler state
 *
 * @author kannan, veeve
 */
void hp_init_profiler_state(int level TSRMLS_DC) {
  /* Setup globals */
  if (!hp_globals.ever_enabled) {
    hp_globals.ever_enabled  = 1;
    hp_globals.entries = NULL;
  }
  hp_globals.profiler_level  = (int) level;

  if (level != QUANTA_MON_MODE_EVENTS_ONLY) {
	  /* Init stats_count */
	  if (hp_globals.stats_count) {
	    zval_dtor(hp_globals.stats_count);
	    FREE_ZVAL(hp_globals.stats_count);
	  }
	  MAKE_STD_ZVAL(hp_globals.stats_count);
	  array_init(hp_globals.stats_count);

	  /* NOTE(cjiang): some fields such as cpu_frequencies take relatively longer
	   * to initialize, (5 milisecond per logical cpu right now), therefore we
	   * calculate them lazily. */
	  if (hp_globals.cpu_frequencies == NULL) {
	    get_all_cpu_frequencies();
	    restore_cpu_affinity(&hp_globals.prev_mask);
	  }

	  /* bind to a random cpu so that we can use rdtsc instruction. */
	  bind_to_cpu((int) (rand() % hp_globals.cpu_num));
  }
  /* Call current mode's init cb */
  hp_globals.mode_cb.init_cb(TSRMLS_C);

  /* Set up filter of functions which may be ignored during profiling */
  hp_ignored_functions_filter_init();

  /* Set up filter of functions which are monitored */
  hp_monitored_functions_filter_init();
}


/**
 * Cleanup profiler state
 *
 * @author kannan, veeve
 */
void hp_clean_profiler_state(TSRMLS_D) {
  /* Call current mode's exit cb */
  hp_globals.mode_cb.exit_cb(TSRMLS_C);

  /* Clear globals */
  if (hp_globals.stats_count) {
    zval_dtor(hp_globals.stats_count);
    FREE_ZVAL(hp_globals.stats_count);
    hp_globals.stats_count = NULL;
  }
  hp_globals.entries = NULL;
  hp_globals.profiler_level = 1;
  hp_globals.ever_enabled = 0;

  /* Delete the array storing ignored function names */
  hp_array_del(hp_globals.ignored_function_names);
  hp_globals.ignored_function_names = NULL;

  /* We don't need to delete the array of monitored function names,
   * it's just pointers to static memory, unlike ignored function names
   * which are emalloced.
   *
   * Delete the array storing monitored function names
   hp_array_del(hp_globals.monitored_function_names);
   hp_globals.monitored_function_names = NULL;
  */
}

/**
 * Increment the count of the given stat with the given count
 * If the stat was not set before, inits the stat to the given count
 *
 * @param  zval *counts   Zend hash table pointer
 * @param  char *name     Name of the stat
 * @param  long  count    Value of the stat to incr by
 * @return void
 * @author kannan
 */
void hp_inc_count(zval *counts, char *name, long count TSRMLS_DC) {
  HashTable *ht;
  void *data;

  if (!counts) return;
  ht = HASH_OF(counts);
  if (!ht) return;

  if (zend_hash_find(ht, name, strlen(name) + 1, &data) == SUCCESS) {
    ZVAL_LONG(*(zval**)data, Z_LVAL_PP((zval**)data) + count);
  } else {
    add_assoc_long(counts, name, count);
  }
}

/**
 * Returns formatted function name
 *
 * @param  entry        hp_entry
 * @param  result_buf   ptr to result buf
 * @param  result_len   max size of result buf
 * @return total size of the function name returned in result_buf
 * @author veeve
 */
size_t hp_get_entry_name(hp_entry_t  *entry,
                         char           *result_buf,
                         size_t          result_len) {

  /* Validate result_len */
  if (result_len <= 1) {
    /* Insufficient result_bug. Bail! */
    return 0;
  }

  /* Add '@recurse_level' if required */
  /* NOTE:  Dont use snprintf's return val as it is compiler dependent */
  if (entry->rlvl_hprof) {
    snprintf(result_buf, result_len,
             "%s@%d",
             entry->name_hprof, entry->rlvl_hprof);
  }
  else {
    snprintf(result_buf, result_len,
             "%s",
             entry->name_hprof);
  }

  /* Force null-termination at MAX */
  result_buf[result_len - 1] = 0;

  return strlen(result_buf);
}
