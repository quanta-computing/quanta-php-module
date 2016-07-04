#include "quanta_mon.h"

/**
 * Request shutdown callback. Stop profiling and return.
 */
PHP_RSHUTDOWN_FUNCTION(quanta_mon) {
  PRINTF_QUANTA("REQUEST SHUTDOWN();\n");
  PRINTF_QUANTA("MATCH COUNTERS:\n - Total: %zu\n - Function: %zu\n - Full: %zu\n - Class unmatch:%zu\n",
    hp_globals.internal_match_counters.total,
    hp_globals.internal_match_counters.function,
    hp_globals.internal_match_counters.function - hp_globals.internal_match_counters.class_unmatched,
    hp_globals.internal_match_counters.class_unmatched
  );
  hp_end(TSRMLS_C);
  return SUCCESS;
}
