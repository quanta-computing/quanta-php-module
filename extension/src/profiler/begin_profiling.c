#include "quanta_mon.h"

int hp_begin_profiling(hp_entry_t **entries, const char *symbol, zend_execute_data *data TSRMLS_DC) {
  int profile_curr;

  profile_curr = qm_begin_profiling(symbol, data TSRMLS_CC);
  if (hp_globals.profiler_level <= QUANTA_MON_MODE_SAMPLED) {
    hp_entry_t *cur_entry = hp_fast_alloc_hprof_entry();
    if (!cur_entry)
      return profile_curr;
    cur_entry->hash_code = hp_inline_hash(symbol);
    cur_entry->name_hprof = symbol;
    cur_entry->prev_hprof = *entries;
    /* Call the universal callback */
    hp_mode_common_beginfn(entries, cur_entry TSRMLS_CC);
    /* Call the mode's beginfn callback */
    hp_globals.mode_cb.begin_fn_cb(entries, cur_entry TSRMLS_CC);
    /* Update entries linked list */
    *entries = cur_entry;
  }
  return profile_curr;
}

/**
 * Check if this entry should be monitored, first with a conservative Bloomish
 * filter then with an exact check against the function names.
 *
 * @author ch
 * return -1 if we don't monitor specifically this function, -2 if we don't monitor at all
 */
int qm_begin_profiling(const char *curr_func, zend_execute_data *execute_data TSRMLS_DC) {
  profiled_function_t *function;
  uint64_t start;
  uint64_t end;

  if (!hp_globals.profiled_application) {
    hp_globals.profiled_application = qm_match_first_app_function(curr_func, execute_data TSRMLS_CC);
    if (!hp_globals.profiled_application)
      return -1;
    PRINTF_QUANTA("APPLICATION FOUND: %s\n", hp_globals.profiled_application->name);
    init_profiled_application(hp_globals.profiled_application);
  }
  start = cycle_timer();
  function = hp_globals.profiled_application->match_function(curr_func, execute_data TSRMLS_CC);
  end = cycle_timer();
  hp_globals.internal_match_counters.cycles += end - start;
  if (!function)
    return -1;
  if (!function->options.ignore_in_stack)
    hp_globals.profiled_application->current_function = function;
  function->tsc.last_start = cycle_timer();
  if (!function->tsc.first_start)
    function->tsc.first_start = function->tsc.last_start;
  if (!function->options.ignore_in_stack)
    PRINTF_QUANTA("BEGIN FUNCTION %zu %s\n", function->index, function->name);
  // TODO! Check profiler level for callbacks
  if (function->begin_callback
  && function->begin_callback(hp_globals.profiled_application, function, execute_data TSRMLS_CC)) {
    return -1;
  }
  return function->index;
}
