#include "quanta_mon.h"

/**
 * Request shutdown callback. Stop profiling and return.
 */
PHP_RSHUTDOWN_FUNCTION(quanta_mon) {
  printf("REQUEST SHUTDOWN();\n");
  hp_globals.global_tsc.stop = cycle_timer();
  hp_globals.internal_match_counters.shutdown_cycles = hp_globals.global_tsc.stop;
  hp_end(TSRMLS_C);
  return SUCCESS;
}
