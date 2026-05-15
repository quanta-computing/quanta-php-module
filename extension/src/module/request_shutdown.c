#include "quanta_mon.h"

/**
 * Request shutdown callback. Stop profiling and return.
 */
PHP_RSHUTDOWN_FUNCTION(quanta_mon) {
  hp_globals.global_tsc.stop = cycle_timer();
  hp_globals.internal_match_counters.shutdown_cycles = hp_globals.global_tsc.stop;
  hp_end();
  return SUCCESS;
}
