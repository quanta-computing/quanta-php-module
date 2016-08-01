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

int qm_end_profiling(int function_idx, zend_execute_data *execute_data TSRMLS_DC) {
  profiled_function_t *function;

  if (function_idx < 0)
    return -1;
  function = &hp_globals.profiled_application->functions[function_idx];
  if (!function->options.ignore_in_stack)
    PRINTF_QUANTA("END FUNCTION %zu %s\n", function->index, function->name);
  function->tsc.last_stop = cycle_timer();
  if (!function->tsc.first_stop)
    function->tsc.first_stop = function->tsc.last_stop;
  // TODO! Check profiler level for callbacks
  if (function->end_callback
  && function->end_callback(hp_globals.profiled_application, function, execute_data TSRMLS_CC)) {
    function_idx = -1;
  }
  if (!function->options.ignore_in_stack) {
    hp_globals.profiled_application->current_function = NULL;
    hp_globals.profiled_application->last_function = function;
  }
  return function_idx;
}
