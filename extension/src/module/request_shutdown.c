#include "quanta_mon.h"

/**
 * Request shutdown callback. Stop profiling and return.
 */
PHP_RSHUTDOWN_FUNCTION(quanta_mon) {
  PRINTF_QUANTA("REQUEST SHUTDOWN();\n");
  hp_globals.internal_match_counters.shutdown_cycles = cycle_timer();
  hp_end(TSRMLS_C);
  if (!hp_globals.cpu_frequencies) {
    PRINTF_QUANTA("CPU frequencies not initialized, cannot profile myself\n");
    return SUCCESS;
  }
  PRINTF_QUANTA("MATCH COUNTERS:\n - Total: %zu\n - Function: %zu\n - Full: %zu\n - Class unmatch: %zu\n",
    hp_globals.internal_match_counters.total,
    hp_globals.internal_match_counters.function,
    hp_globals.internal_match_counters.function - hp_globals.internal_match_counters.class_unmatched,
    hp_globals.internal_match_counters.class_unmatched
  );
  PRINTF_QUANTA("TREE TIME %fms\nHASH TIME %fms\n",
    cpu_cycles_to_ms(hp_globals.cpu_frequencies[hp_globals.cur_cpu_id],
      hp_globals.internal_match_counters.cycles),
    cpu_cycles_to_ms(hp_globals.cpu_frequencies[hp_globals.cur_cpu_id],
      hp_globals.internal_match_counters.hash_cycles)
  );
  PRINTF_QUANTA("PROFILING TIME %fms\nINIT TIME %fms\nSHUTDOWN TIME %fms\nTOTAL TIME %fms\n",
    cpu_cycles_to_ms(hp_globals.cpu_frequencies[hp_globals.cur_cpu_id],
      hp_globals.internal_match_counters.profiling_cycles),
    cpu_cycles_to_ms(hp_globals.cpu_frequencies[hp_globals.cur_cpu_id],
      hp_globals.internal_match_counters.init_cycles),
    cpu_cycles_to_ms(hp_globals.cpu_frequencies[hp_globals.cur_cpu_id],
      hp_globals.internal_match_counters.shutdown_cycles),
    cpu_cycles_to_ms(hp_globals.cpu_frequencies[hp_globals.cur_cpu_id],
      hp_globals.internal_match_counters.profiling_cycles
      + hp_globals.internal_match_counters.init_cycles
      + hp_globals.internal_match_counters.shutdown_cycles)
  );
  return SUCCESS;
}
