#include "quanta_mon.h"


// int qm_record_sql_timers(void) {
//   magento_block_t *block;
//   uint64_t cycles = hp_globals.monitored_function_tsc_stop[POS_ENTRY_PDO_EXECUTE]
//     - hp_globals.monitored_function_tsc_start[POS_ENTRY_PDO_EXECUTE];
//
//   hp_globals.monitored_function_sql_cpu_cycles[0] += cycles;
//   hp_globals.monitored_function_sql_queries_count[0]++;
//   if ((block = block_stack_top())) {
//     block->sql_cpu_cycles += cycles;
//     block->sql_queries_count++;
//     PRINTF_QUANTA("SQL QUERY IN BLOCK\n");
//   }
//   if (hp_globals.current_monitored_function > 0
//   && hp_globals.current_monitored_function < QUANTA_MON_MAX_MONITORED_FUNCTIONS) {
//     hp_globals.monitored_function_sql_cpu_cycles[hp_globals.current_monitored_function] += cycles;
//     hp_globals.monitored_function_sql_queries_count[hp_globals.current_monitored_function]++;
//     if (!(block = block_stack_top()))
//       PRINTF_QUANTA("SQL QUERY IN FUNCTION [%d] %s\n", hp_globals.current_monitored_function,
//         hp_globals_monitored_function_names()[hp_globals.current_monitored_function]);
//     return 0;
//   } else if (hp_globals.last_monitored_function > 0
//   && hp_globals.last_monitored_function < QUANTA_MON_MAX_MONITORED_FUNCTIONS) {
//     hp_globals.monitored_function_sql_cpu_cycles_after[hp_globals.last_monitored_function] += cycles;
//     hp_globals.monitored_function_sql_queries_count_after[hp_globals.last_monitored_function]++;
//     if (!(block = block_stack_top()))
//       PRINTF_QUANTA("SQL QUERY AFTER FUNCTION [%d] %s\n", hp_globals.last_monitored_function,
//         hp_globals_monitored_function_names()[hp_globals.last_monitored_function]);
//     return 0;
//   } else {
//     PRINTF_QUANTA("SQL query outside monitored function\n");
//     return -1;
//   }
// }

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

  if (!hp_globals.profiled_application)
    return -1;
  start = cycle_timer();
  function = hp_globals.profiled_application->match_function(curr_func, execute_data TSRMLS_CC);
  end = cycle_timer();
  hp_globals.internal_match_counters.cycles += end - start;
  if (!function)
    return -1;
  hp_globals.profiled_application->current_function = function;
  function->tsc.last_start = cycle_timer();
  if (!function->tsc.first_start)
    function->tsc.first_start = function->tsc.last_start;
  PRINTF_QUANTA("BEGIN FUNCTION %d %s\n", function->index, function->name);
  // TODO! Check profiler level for callbacks
  if (function->begin_callback
  && function->begin_callback(hp_globals.profiled_application, execute_data TSRMLS_CC)) {
    return -1;
  }
  return function->index;
  // if (i == POS_ENTRY_APP_RUN) {
  //   fetch_magento_version(TSRMLS_C);
  // }
  // if (i != POS_ENTRY_PDO_EXECUTE && i != POS_ENTRY_TOHTML && *hp_globals_monitored_function_names()[i]) {
  //   PRINTF_QUANTA("BEGIN FUNCTION %d %s\n", i, hp_globals_monitored_function_names()[i]);
  //   hp_globals.current_monitored_function = i;
  //   hp_globals.last_monitored_function = -1;
  // }
  // if (i == POS_ENTRY_TOHTML) {
  //   qm_before_tohtml(i, execute_data TSRMLS_CC);
  //   return i;
  // }
  // if (i == POS_ENTRY_EV_CACHE_FLUSH)
  //   return qm_record_cache_flush_event(i, execute_data TSRMLS_CC);
  // if (i == POS_ENTRY_EV_CLEAN_TYPE)
  //   return qm_record_cache_clean_event(i, execute_data TSRMLS_CC);
  // if (i == POS_ENTRY_EV_MAGE_CLEAN)
  //   return qm_record_cache_system_flush_event(i, execute_data TSRMLS_CC);
}

int qm_end_profiling(int function_idx, zend_execute_data *execute_data TSRMLS_DC) {
  profiled_function_t *function;

  if (function_idx < 0)
    return -1;
  function = hp_globals.profiled_application->functions[function_idx];
  PRINTF_QUANTA("END FUNCTION %d %s\n", function->index, function->name);
  function->tsc.last_stop = cycle_timer();
  if (!function->tsc.first_stop)
    function->tsc.first_stop = function->tsc.last_stop;
  if (function->end_callback
  && function->end_callback(hp_globals.profiled_application, execute_data TSRMLS_CC)) {
    function_idx = -1;
  }
  hp_globals.profiled_application->current_function = NULL;
  hp_globals.profiled_application->last_function = function;
  return function_idx;
  // if (profile_curr != POS_ENTRY_PDO_EXECUTE && profile_curr != POS_ENTRY_TOHTML && *hp_globals_monitored_function_names()[profile_curr]) {
  //   PRINTF_QUANTA("END FUNCTION %d %s\n", profile_curr, hp_globals_monitored_function_names()[profile_curr]);
  //   hp_globals.current_monitored_function = -1;
  //   hp_globals.last_monitored_function = profile_curr;
  // }
  //
  // if (profile_curr == POS_ENTRY_TOHTML)
  //   qm_after_tohtml(execute_data TSRMLS_CC);
  // if (profile_curr == POS_ENTRY_PDO_EXECUTE)
  //   qm_record_sql_timers();
  // if (profile_curr == POS_ENTRY_EV_BEFORE_SAVE)
  //   return qm_record_reindex_event(profile_curr, execute_data TSRMLS_CC);
}
