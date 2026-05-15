#include "quanta_mon.h"

/**
 * Cleanup profiler state
 *
 * @author kannan, veeve
 */
void hp_clean_profiler_state(void) {
  ZVAL_NULL(&hp_globals.stats_count);
  hp_globals.entries = NULL;
  hp_globals.profiler_level = 1;
  hp_globals.ever_enabled = 0;
  efree(hp_globals.request_uri);
  if (hp_globals.profiled_application)
    clean_profiled_application(hp_globals.profiled_application);
}
