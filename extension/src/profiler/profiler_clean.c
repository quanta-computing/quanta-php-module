#include "quanta_mon.h"

/**
 * Cleanup profiler state
 *
 * @author kannan, veeve
 */
void hp_clean_profiler_state(TSRMLS_D) {
  /* Call current mode's exit cb */
  hp_globals.mode_cb.exit_cb(TSRMLS_C);

  ZVAL_NULL(&hp_globals.stats_count);
  hp_globals.entries = NULL;
  hp_globals.profiler_level = 1;
  hp_globals.ever_enabled = 0;

  efree(hp_globals.request_uri);
  efree(hp_globals.magento_version);
  efree(hp_globals.magento_edition);

  /* Pop all blocks still present in the stack (should be zero) */
  while (block_stack_pop());

}
