#include "quanta_mon.h"

int qm_record_sql_timers(void) {
  magento_block_t *block;
  uint64_t cycles = hp_globals.monitored_function_tsc_stop[POS_ENTRY_PDO_EXECUTE]
    - hp_globals.monitored_function_tsc_start[POS_ENTRY_PDO_EXECUTE];

  hp_globals.monitored_function_sql_cpu_cycles[0] += cycles;
  hp_globals.monitored_function_sql_queries_count[0]++;
  if ((block = block_stack_top())) {
    block->sql_cpu_cycles += cycles;
    block->sql_queries_count++;
    return 0;
  } else if (hp_globals.current_monitored_function > 0
  && hp_globals.current_monitored_function < QUANTA_MON_MAX_MONITORED_FUNCTIONS) {
    hp_globals.monitored_function_sql_cpu_cycles[hp_globals.current_monitored_function] += cycles;
    hp_globals.monitored_function_sql_queries_count[hp_globals.current_monitored_function]++;
    return 0;
  } else {
    return -1;
  }
}

/**
 * Check if this entry should be ignored, first with a conservative Bloomish
 * filter then with an exact check against the function names.
 *
 * @author ch
 * return -1 if we don't monitor specifically this function, -2 if we don't monitor at all
 */
int qm_record_timers_loading_time(uint8_t hash_code, char *curr_func, zend_execute_data *execute_data TSRMLS_DC) {
  int i;

  /* Search quickly if we may have a match */
  if (!hp_monitored_functions_filter_collision(hash_code))
    return -1;

  if (hp_globals.profiler_level == QUANTA_MON_MODE_EVENTS_ONLY)
    i = POS_ENTRY_EVENTS_ONLY;
  else
    i = 0;

  /* We MAY have a match, enumerate the function array for an exact match */
  for (; hp_globals.monitored_function_names[i] != NULL; i++) {
    char *name = hp_globals.monitored_function_names[i];
    if (!strcmp(curr_func, name))
      break;
  }

  if (hp_globals.monitored_function_names[i] == NULL)
    return -1; /* False match, we have nothing */

  hp_globals.monitored_function_tsc_start[i] = cycle_timer();

  if (i != POS_ENTRY_PDO_EXECUTE)
    hp_globals.current_monitored_function = i;

  if (i == POS_ENTRY_TOHTML) {
    qm_before_tohtml(i, execute_data TSRMLS_CC);
    return i;
  }
  if (i == POS_ENTRY_EV_CACHE_FLUSH)
    return qm_record_cache_flush_event(i, execute_data TSRMLS_CC);
  if (i == POS_ENTRY_EV_CLEAN_TYPE)
    return qm_record_cache_clean_event(i, execute_data TSRMLS_CC);
  if (i == POS_ENTRY_EV_MAGE_CLEAN)
    return qm_record_cache_system_flush_event(i, execute_data TSRMLS_CC);
  if (i == POS_ENTRY_EV_BEFORE_SAVE)
    return qm_record_reindex_event(i, execute_data TSRMLS_CC);

  return i; /* No, bailout */
}
