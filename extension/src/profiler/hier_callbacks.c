#include "quanta_mon.h"

/**
 * QUANTA_MON_MODE_HIERARCHICAL's begin function callback
 *
 * @author kannan
 */
void hp_mode_hier_beginfn_cb(hp_entry_t **entries, hp_entry_t *current  TSRMLS_DC) {
  (void)entries;

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
 * QUANTA_MON_MODE_HIERARCHICAL's end function callback
 *
 * @author kannan
 */
void hp_mode_hier_endfn_cb(hp_entry_t **entries  TSRMLS_DC) {
  hp_entry_t       *top = (*entries);
  zval             *counts;
  struct rusage    ru_end;
  char             symbol[SCRATCH_BUF_LEN];
  long int         mu_end;
  long int         pmu_end;
  uint64_t         tsc_end;
  HashTable        *ht;
#if PHP_MAJOR_VERSION >= 7
  zval             counts_val;
#endif


  tsc_end = cycle_timer();
  hp_get_function_stack(top, 2, symbol, sizeof(symbol));
  if (Z_TYPE(hp_globals.stats_count) != IS_ARRAY
  || !(ht = HASH_OF(&hp_globals.stats_count))) {
    return;
  }
  if (!(counts = zend_hash_find_compat(ht, symbol, strlen(symbol)))) {
#if PHP_MAJOR_VERSION < 7
    MAKE_STD_ZVAL(counts);
    array_init(counts);
    zend_hash_update(ht, symbol, strlen(symbol) + 1, &counts, sizeof(counts), NULL);
#else
    counts = &counts_val;
    array_init(counts);
    zend_hash_str_update(ht, symbol, strlen(symbol), counts);
#endif
  }

  hp_inc_count(counts, "ct", 1  TSRMLS_CC);
  hp_inc_count(counts, "wt", get_us_from_tsc(tsc_end - top->tsc_start,
        hp_globals.cpu_frequencies[hp_globals.cur_cpu_id]) TSRMLS_CC);

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
