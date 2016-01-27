#include "quanta_mon.h"

static void register_appropriate_callbacks(long level) {
  /* Register the appropriate callback functions Override just a subset of
   * all the callbacks is OK. */
  switch (level) {
    case QUANTA_MON_MODE_HIERARCHICAL:
      hp_globals.mode_cb.begin_fn_cb = hp_mode_hier_beginfn_cb;
      hp_globals.mode_cb.end_fn_cb   = hp_mode_hier_endfn_cb;
      break;
    case QUANTA_MON_MODE_SAMPLED:
      hp_globals.mode_cb.init_cb     = hp_mode_sampled_init_cb;
      hp_globals.mode_cb.begin_fn_cb = hp_mode_sampled_beginfn_cb;
      hp_globals.mode_cb.end_fn_cb   = hp_mode_sampled_endfn_cb;
      break;
    case QUANTA_MON_MODE_MAGENTO_PROFILING:
      hp_globals.mode_cb.begin_fn_cb = hp_mode_magento_profil_beginfn_cb;
      hp_globals.mode_cb.end_fn_cb   = hp_mode_magento_profil_endfn_cb;
      break;
    case QUANTA_MON_MODE_EVENTS_ONLY:
      hp_globals.mode_cb.begin_fn_cb = hp_mode_events_only_beginfn_cb;
      hp_globals.mode_cb.end_fn_cb   = hp_mode_events_only_endfn_cb;
      break;
  }
}

/**
 * This function gets called once when quanta_mon gets enabled.
 * It replaces all the functions like zend_execute, zend_execute_internal,
 * etc that needs to be instrumented with their corresponding proxies.
 */
void hp_begin(long level, long quanta_mon_flags TSRMLS_DC) {
  if (!hp_globals.enabled) {

    hp_globals.enabled      = 1;
    hp_globals.quanta_mon_flags = (uint32_t)quanta_mon_flags;

    /* Replace various zend functions with our proxies
    */
    hp_hijack_zend_execute((uint32_t)quanta_mon_flags);

    /* Initialize with the dummy mode first Having these dummy callbacks saves
     * us from checking if any of the callbacks are NULL everywhere. */
    hp_globals.mode_cb.init_cb     = hp_mode_dummy_init_cb;
    hp_globals.mode_cb.exit_cb     = hp_mode_dummy_exit_cb;
    hp_globals.mode_cb.begin_fn_cb = hp_mode_dummy_beginfn_cb;
    hp_globals.mode_cb.end_fn_cb   = hp_mode_dummy_endfn_cb;

    /* We then register the appropriate callbacks for the profiling level
    */
    register_appropriate_callbacks(level);

    /* one time initializations */
    hp_init_profiler_state(level TSRMLS_CC);

    /* start profiling from fictitious main() */
    hp_begin_profiling(&hp_globals.entries, ROOT_SYMBOL, "main", NULL);
  }
}
