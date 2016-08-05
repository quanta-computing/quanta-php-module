#include "quanta_mon.h"

/**
 * This function gets called once when quanta_mon gets enabled.
 * It replaces all the functions like zend_execute, zend_execute_internal,
 * etc that needs to be instrumented with their corresponding proxies.
 */
void hp_begin(long level TSRMLS_DC) {
  if (!hp_globals.enabled) {
    PRINTF_QUANTA("hp_begin();\n");
    hp_globals.enabled      = 1;

    hp_hijack_zend_execute(level);
    hp_init_profiler_state(level TSRMLS_CC);

    /* start profiling from fictitious main() */
    hp_begin_profiling(&hp_globals.entries, ROOT_SYMBOL, NULL TSRMLS_CC);
  }
}
