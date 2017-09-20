#include "quanta_mon.h"

static void print_selfprofiling_metrics(void) {
  float profiling_time;
  float request_time;

  // if (!hp_globals.cpu_frequencies) {
  //   PRINTF_QUANTA("CPU frequencies not initialized, cannot profile myself\n");
  //   return;
  // }
  PRINTF_QUANTA("MATCH COUNTERS:\n - Total: %zu\n - Function: %zu\n - Full: %zu\n - Class unmatch: %zu\n",
    hp_globals.internal_match_counters.total,
    hp_globals.internal_match_counters.function,
    hp_globals.internal_match_counters.function - hp_globals.internal_match_counters.class_unmatched,
    hp_globals.internal_match_counters.class_unmatched
  );
  PRINTF_QUANTA("TREE TIME %fms\nHASH TIME %fms\n",
    timer_to_ms(hp_globals.internal_match_counters.cycles),
    timer_to_ms(hp_globals.internal_match_counters.hash_cycles)
  );
  profiling_time = timer_to_ms(
    hp_globals.internal_match_counters.profiling_cycles
    + hp_globals.internal_match_counters.init_cycles
    + hp_globals.internal_match_counters.shutdown_cycles);
  PRINTF_QUANTA("PROFILING TIME %fms\nINIT TIME %fms\nSHUTDOWN TIME %fms\nTOTAL TIME %fms\n",
    timer_to_ms(hp_globals.internal_match_counters.profiling_cycles),
    timer_to_ms(hp_globals.internal_match_counters.init_cycles),
    timer_to_ms(hp_globals.internal_match_counters.shutdown_cycles),
    profiling_time
  );
  request_time = timer_to_ms(hp_globals.global_tsc.stop - hp_globals.global_tsc.start);
  PRINTF_QUANTA("REQUEST TOTAL TIME %fms\nOVERHEAD %.3f%%\n",
    request_time, 100.0 * (profiling_time / request_time));
}

/**
 * Called at request shutdown time. Cleans the profiler's global state.
 */
void hp_end(TSRMLS_D) {
  /* Stop profiler if enabled */
  if (hp_globals.enabled) {
    hp_stop(TSRMLS_C);
    hp_clean_profiler_state(TSRMLS_C);
  }
}

/**
 * Called from quanta_mon_disable(). Removes all the proxies setup by
 * hp_begin() and restores the original values.
 */
void hp_stop(TSRMLS_D) {
  /* End any unfinished calls */
  while (hp_globals.entries)
    hp_end_profiling(&hp_globals.entries, -1, NULL TSRMLS_CC);
  send_metrics(TSRMLS_C);
  print_selfprofiling_metrics();
  hp_restore_original_zend_execute();
  /* Resore cpu affinity. */
  // restore_cpu_affinity(&hp_globals.prev_mask);
  /* Stop profiling */
  hp_globals.enabled = 0;
}
