#include "quanta_mon.h"

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
  hp_globals.magento_events = NULL;
  hp_globals.magento_version = NULL;
  hp_globals.magento_edition = NULL;

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

  hp_globals.block_stack = NULL;
  hp_globals.magento_blocks_first = NULL;
}
