#include "quanta_mon.h"

void register_application(void);

/**
 * Initialize profiler state
 *
 * @author kannan, veeve
 */
void hp_init_profiler_state(int level TSRMLS_DC) {
  /* Setup globals */
  if (!hp_globals.ever_enabled) {
    hp_globals.ever_enabled  = 1;
    hp_globals.entries = NULL;
  }
  hp_globals.profiler_level  = (int) level;
  hp_globals.profiled_application = NULL;
  hp_globals.app_events = NULL;

  // TODO! TMP until we find something better
  register_application();
  if (hp_globals.profiled_application)
    init_profiled_application(hp_globals.profiled_application);

  if (level != QUANTA_MON_MODE_EVENTS_ONLY) {

    array_init(&hp_globals.stats_count);
    /* NOTE(cjiang): some fields such as cpu_frequencies take relatively longer
     * to initialize, (5 milisecond per logical cpu right now), therefore we
     * calculate them lazily. */
    if (hp_globals.cpu_frequencies == NULL) {
      get_all_cpu_frequencies();
      restore_cpu_affinity(&hp_globals.prev_mask);
    }

    /* bind to a random cpu so that we can use rdtsc instruction. */
    bind_to_cpu((int) (rand() % hp_globals.cpu_num));
  }

  /* Call current mode's init cb */
  hp_globals.mode_cb.init_cb(TSRMLS_C);
}
