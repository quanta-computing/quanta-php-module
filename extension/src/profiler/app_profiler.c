#include "quanta_mon.h"

int qm_begin_profiling(const char *curr_func, zend_execute_data *execute_data TSRMLS_DC) {
  profiled_function_t *function;
  uint64_t match_start;
  uint64_t match_end;

  if (!hp_globals.profiled_application) {
    hp_globals.profiled_application = qm_match_first_app_function(curr_func, execute_data TSRMLS_CC);
    if (!hp_globals.profiled_application)
      return -1;
    PRINTF_QUANTA("APPLICATION FOUND: %s\n", hp_globals.profiled_application->name);
    init_profiled_application(hp_globals.profiled_application);
  }
  match_start = cycle_timer();
  function = hp_globals.profiled_application->match_function(curr_func, execute_data TSRMLS_CC);
  match_end = cycle_timer();
  hp_globals.internal_match_counters.cycles += match_end - match_start;
  if (!function || function->options.min_profiling_level < hp_globals.profiler_level)
    return -1;
  function->tsc.last_start = cycle_timer();
  if (!function->tsc.first_start)
    function->tsc.first_start = function->tsc.last_start;
  if (function->begin_callback
  && function->begin_callback(hp_globals.profiled_application, function, execute_data TSRMLS_CC)) {
    return -1;
  }
  return function->index;
}

int qm_end_profiling(int function_idx, zend_execute_data *execute_data TSRMLS_DC) {
  profiled_application_t *app = hp_globals.profiled_application;
  profiled_function_t *function;

  if (function_idx < 0)
    return -1;
  function = &app->functions[function_idx];
  function->tsc.last_stop = cycle_timer();
  if (!function->tsc.first_stop)
    function->tsc.first_stop = function->tsc.last_stop;
  if (function->end_callback
  && function->end_callback(hp_globals.profiled_application, function, execute_data TSRMLS_CC)) {
    function_idx = -1;
  }
  return function_idx;
}
