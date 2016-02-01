#include "quanta_mon.h"

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
size_t hp_get_entry_name(hp_entry_t  *entry, char *result_buf, size_t result_len) {

  /* Validate result_len */
  if (result_len <= 1) {
    /* Insufficient result_bug. Bail! */
    return 0;
  }

  /* Add '@recurse_level' if required */
  /* NOTE:  Dont use snprintf's return val as it is compiler dependent */
  if (entry->rlvl_hprof) {
    snprintf(result_buf, result_len, "%s@%d", entry->name_hprof, entry->rlvl_hprof);
  }
  else {
    snprintf(result_buf, result_len, "%s", entry->name_hprof);
  }

  /* Force null-termination at MAX */
  result_buf[result_len - 1] = 0;

  return strlen(result_buf);
}

int hp_begin_profiling(hp_entry_t **entries, char *symbol, char *pathname, zend_execute_data *data) {
  uint8_t hash_code = hp_inline_hash(symbol);
  int profile_curr;

  if(hp_ignore_entry(hash_code, symbol))
    profile_curr = -1;
  else
    profile_curr = qm_record_timers_loading_time(hash_code, symbol, data);
  if (hp_globals.profiler_level <= QUANTA_MON_MODE_SAMPLED) { //TODO! Check profile_curr too
    hp_entry_t *cur_entry = hp_fast_alloc_hprof_entry(); //TODO! Check NULL pointers here
    cur_entry->hash_code = hash_code;
    cur_entry->name_hprof = symbol;
    cur_entry->pathname_hprof = pathname;
    cur_entry->prev_hprof = *entries;
    /* Call the universal callback */
    hp_mode_common_beginfn(entries, cur_entry TSRMLS_CC);
    /* Call the mode's beginfn callback */
    hp_globals.mode_cb.begin_fn_cb(entries, cur_entry TSRMLS_CC);
    /* Update entries linked list */
    *entries = cur_entry;
  }
  return profile_curr;
}

void hp_end_profiling(hp_entry_t **entries, int profile_curr, zend_execute_data *data) {
  if (profile_curr >= 0) {
    hp_globals.monitored_function_tsc_stop[profile_curr] = cycle_timer();
    if (profile_curr == POS_ENTRY_GENERATEBLOCK) {
      hp_globals.monitored_function_generate_renderize_block_last_linked_list->tsc_generate_stop =
        hp_globals.monitored_function_tsc_stop[profile_curr];
    } else if (profile_curr == POS_ENTRY_AFTERTOHTML) {
      qm_after_tohmtl(data);
    }
  }
  if (hp_globals.profiler_level <= QUANTA_MON_MODE_SAMPLED) {
    hp_entry_t *cur_entry;
    /* Call the mode's endfn callback. */
    /* NOTE(cjiang): we want to call this 'end_fn_cb' before */
    /* 'hp_mode_common_endfn' to avoid including the time in */
    /* 'hp_mode_common_endfn' in the profiling results.      */
    hp_globals.mode_cb.end_fn_cb((entries) TSRMLS_CC);
    cur_entry = *entries;
    /* Call the universal callback */
    hp_mode_common_endfn(entries, cur_entry TSRMLS_CC);
    /* Free top entry and update entries linked list */
    *entries = (*entries)->prev_hprof;
    hp_fast_free_hprof_entry(cur_entry);
  }
}