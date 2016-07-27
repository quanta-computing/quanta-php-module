#include "quanta_mon.h"

void hp_end_profiling(hp_entry_t **entries, int profile_curr, zend_execute_data *data TSRMLS_DC) {
  if (profile_curr >= 0) {
    qm_end_profiling(profile_curr, data TSRMLS_CC);
  }
  if (hp_globals.profiler_level <= QUANTA_MON_MODE_SAMPLED && entries && *entries) {
    hp_entry_t *cur_entry;
    /* Call the mode's endfn callback. */
    /* NOTE(cjiang): we want to call this 'end_fn_cb' before */
    /* 'hp_mode_common_endfn' to avoid including the time in */
    /* 'hp_mode_common_endfn' in the profiling results.      */
    hp_globals.mode_cb.end_fn_cb((entries) TSRMLS_CC);
    cur_entry = *entries;
    /* Call the universal callback */
    hp_mode_common_endfn(entries, cur_entry TSRMLS_CC);
    /* Free top entry and update entries linked list */
    *entries = (*entries)->prev_hprof;
    hp_fast_free_hprof_entry(cur_entry);
  }
}
