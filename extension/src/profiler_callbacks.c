#include "quanta_mon.h"

/**
 * ***************************
 * QUANTA_MON DUMMY CALLBACKS
 * ***************************
 */
void hp_mode_dummy_init_cb(TSRMLS_D) { }


void hp_mode_dummy_exit_cb(TSRMLS_D) { }


void hp_mode_dummy_beginfn_cb(hp_entry_t **entries,
                              hp_entry_t *current  TSRMLS_DC) { }

void hp_mode_dummy_endfn_cb(hp_entry_t **entries   TSRMLS_DC) { }


/**
 * ****************************
 * QUANTA_MON COMMON CALLBACKS
 * ****************************
 */
/**
 * QUANTA_MON universal begin function.
 * This function is called for all modes before the
 * mode's specific begin_function callback is called.
 *
 * @param  hp_entry_t **entries  linked list (stack)
 *                                  of hprof entries
 * @param  hp_entry_t  *current  hprof entry for the current fn
 * @return void
 * @author kannan, veeve
 */
void hp_mode_common_beginfn(hp_entry_t **entries,
                            hp_entry_t  *current  TSRMLS_DC) {
  hp_entry_t   *p;

  /* This symbol's recursive level */
  int    recurse_level = 0;

  if (hp_globals.func_hash_counters[current->hash_code] > 0) {
    /* Find this symbols recurse level */
    for(p = (*entries); p; p = p->prev_hprof) {
      if (!strcmp(current->name_hprof, p->name_hprof)) {
        recurse_level = (p->rlvl_hprof) + 1;
        break;
      }
    }
  }
  hp_globals.func_hash_counters[current->hash_code]++;

  /* Init current function's recurse level */
  current->rlvl_hprof = recurse_level;
}

/**
 * QUANTA_MON universal end function.  This function is called for all modes after
 * the mode's specific end_function callback is called.
 *
 * @param  hp_entry_t **entries  linked list (stack) of hprof entries
 * @return void
 * @author kannan, veeve
 */
void hp_mode_common_endfn(hp_entry_t **entries, hp_entry_t *current TSRMLS_DC) {
  hp_globals.func_hash_counters[current->hash_code]--;
}


/**
 * *********************************
 * QUANTA_MON INIT MODULE CALLBACKS
 * *********************************
 */
/**
 * QUANTA_MON_MODE_SAMPLED's init callback
 *
 * @author veeve
 */
void hp_mode_sampled_init_cb(TSRMLS_D) {
  struct timeval  now;
  uint64 truncated_us;
  uint64 truncated_tsc;
  double cpu_freq = hp_globals.cpu_frequencies[hp_globals.cur_cpu_id];

  /* Init the last_sample in tsc */
  hp_globals.last_sample_tsc = cycle_timer();

  /* Find the microseconds that need to be truncated */
  gettimeofday(&hp_globals.last_sample_time, 0);
  now = hp_globals.last_sample_time;
  hp_trunc_time(&hp_globals.last_sample_time, QUANTA_MON_SAMPLING_INTERVAL);

  /* Subtract truncated time from last_sample_tsc */
  truncated_us  = get_us_interval(&hp_globals.last_sample_time, &now);
  truncated_tsc = get_tsc_from_us(truncated_us, cpu_freq);
  if (hp_globals.last_sample_tsc > truncated_tsc) {
    /* just to be safe while subtracting unsigned ints */
    hp_globals.last_sample_tsc -= truncated_tsc;
  }

  /* Convert sampling interval to ticks */
  hp_globals.sampling_interval_tsc =
    get_tsc_from_us(QUANTA_MON_SAMPLING_INTERVAL, cpu_freq);
}


/**
 * ************************************
 * QUANTA_MON BEGIN FUNCTION CALLBACKS
 * ************************************
 */

/**
 * QUANTA_MON_MODE_HIERARCHICAL's begin function callback
 *
 * @author kannan
 */
void hp_mode_hier_beginfn_cb(hp_entry_t **entries,
                             hp_entry_t  *current  TSRMLS_DC) {
  /* Get start tsc counter */
  current->tsc_start = cycle_timer();

  /* Get CPU usage */
  if (hp_globals.quanta_mon_flags & QUANTA_MON_FLAGS_CPU) {
    getrusage(RUSAGE_SELF, &(current->ru_start_hprof));
  }

  /* Get memory usage */
  if (hp_globals.quanta_mon_flags & QUANTA_MON_FLAGS_MEMORY) {
    current->mu_start_hprof  = zend_memory_usage(0 TSRMLS_CC);
    current->pmu_start_hprof = zend_memory_peak_usage(0 TSRMLS_CC);
  }
}

/**
 * QUANTA_MON_MODE_MAGENTO_PROFILING's begin function callback
 *
 * @author ch
 */
void hp_mode_magento_profil_beginfn_cb(hp_entry_t **entries,
                             hp_entry_t  *current  TSRMLS_DC) {
}
/**
 * QUANTA_MON_MODE_EVENTS_ONLY's begin function callback
 *
 * @author ch
 */
void hp_mode_events_only_beginfn_cb(hp_entry_t **entries,
                             hp_entry_t  *current  TSRMLS_DC) {
}

/**
 * QUANTA_MON_MODE_SAMPLED's begin function callback
 *
 * @author veeve
 */
void hp_mode_sampled_beginfn_cb(hp_entry_t **entries,
                                hp_entry_t  *current  TSRMLS_DC) {
  /* See if its time to take a sample */
  hp_sample_check(entries  TSRMLS_CC);
}


/**
 * **********************************
 * QUANTA_MON END FUNCTION CALLBACKS
 * **********************************
 */

/**
 * QUANTA_MON shared end function callback
 *
 * @author kannan
 */
zval * hp_mode_shared_endfn_cb(hp_entry_t *top,
                               char          *symbol  TSRMLS_DC) {
  zval    *counts;
  uint64   tsc_end;

  /* Get end tsc counter */
  tsc_end = cycle_timer();

  /* Get the stat array */
  if (!(counts = hp_hash_lookup(symbol TSRMLS_CC))) {
    return (zval *) 0;
  }

  /* Bump stats in the counts hashtable */
  hp_inc_count(counts, "ct", 1  TSRMLS_CC);

  hp_inc_count(counts, "wt", get_us_from_tsc(tsc_end - top->tsc_start,
        hp_globals.cpu_frequencies[hp_globals.cur_cpu_id]) TSRMLS_CC);
  return counts;
}

/**
 * QUANTA_MON_MODE_HIERARCHICAL's end function callback
 *
 * @author kannan
 */
void hp_mode_hier_endfn_cb(hp_entry_t **entries  TSRMLS_DC) {
  hp_entry_t   *top = (*entries);
  zval            *counts;
  struct rusage    ru_end;
  char             symbol[SCRATCH_BUF_LEN];
  long int         mu_end;
  long int         pmu_end;

  /* Get the stat array */
  hp_get_function_stack(top, 2, symbol, sizeof(symbol));
  if (!(counts = hp_mode_shared_endfn_cb(top,
                                         symbol  TSRMLS_CC))) {
    return;
  }

  if (hp_globals.quanta_mon_flags & QUANTA_MON_FLAGS_CPU) {
    /* Get CPU usage */
    getrusage(RUSAGE_SELF, &ru_end);

    /* Bump CPU stats in the counts hashtable */
    hp_inc_count(counts, "cpu", (get_us_interval(&(top->ru_start_hprof.ru_utime),
                                              &(ru_end.ru_utime)) +
                              get_us_interval(&(top->ru_start_hprof.ru_stime),
                                              &(ru_end.ru_stime)))
              TSRMLS_CC);
  }

  if (hp_globals.quanta_mon_flags & QUANTA_MON_FLAGS_MEMORY) {
    /* Get Memory usage */
    mu_end  = zend_memory_usage(0 TSRMLS_CC);
    pmu_end = zend_memory_peak_usage(0 TSRMLS_CC);

    /* Bump Memory stats in the counts hashtable */
    hp_inc_count(counts, "mu",  mu_end - top->mu_start_hprof    TSRMLS_CC);
    hp_inc_count(counts, "pmu", pmu_end - top->pmu_start_hprof  TSRMLS_CC);
  }
}

/**
 * QUANTA_MON_MODE_SAMPLED's end function callback
 *
 * @author veeve
 */
void hp_mode_sampled_endfn_cb(hp_entry_t **entries  TSRMLS_DC) {
  /* See if its time to take a sample */
  hp_sample_check(entries  TSRMLS_CC);
}

/**
 * QUANTA_MON_MODE_MAGENTO_PROFILING's end function callback
 *
 * @author ch
 */
void hp_mode_magento_profil_endfn_cb(hp_entry_t **entries  TSRMLS_DC) {
}


/**
 * QUANTA_MON_MODE_EVENTS_ONLY's end function callback
 *
 * @author ch
 */
void hp_mode_events_only_endfn_cb(hp_entry_t **entries  TSRMLS_DC) {
}
