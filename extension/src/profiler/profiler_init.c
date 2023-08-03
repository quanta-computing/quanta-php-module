#include "quanta_mon.h"

/**
 * Initialize profiler state
 *
 * @author kannan, veeve
 */
void hp_init_profiler_state(int level) {
  /* Setup globals */
  if (!hp_globals.ever_enabled) {
    hp_globals.ever_enabled  = 1;
    hp_globals.entries = NULL;
  }
  hp_globals.profiler_level  = (int) level;
  hp_globals.profiled_application = NULL;
  hp_globals.app_events = NULL;

  hp_globals.profiled_application = NULL;

  memset(&hp_globals.internal_match_counters, 0, sizeof(hp_globals.internal_match_counters));

  if (level != QUANTA_MON_MODE_EVENTS_ONLY) {

    array_init(&hp_globals.stats_count);

    /* bind to a random cpu so that we can use rdtsc instruction. */
    bind_to_cpu((uint32_t)(rand() % hp_globals.cpu_num));
  }
}
