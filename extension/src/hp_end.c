#include "quanta_mon.h"

/**
 * Called at request shutdown time. Cleans the profiler's global state.
 */
void hp_end(TSRMLS_D) {
  /* Bail if not ever enabled */
  if (!hp_globals.ever_enabled) {
    return;
  }

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
  hp_globals.monitored_function_tsc_stop[POS_ENTRY_PHP_TOTAL] = cycle_timer();
  while (hp_globals.entries)
    hp_end_profiling(&hp_globals.entries, -1, NULL TSRMLS_CC);
  send_metrics(TSRMLS_C);
  hp_restore_original_zend_execute();
  /* Resore cpu affinity. */
  restore_cpu_affinity(&hp_globals.prev_mask);
  /* Stop profiling */
  hp_globals.enabled = 0;
}
