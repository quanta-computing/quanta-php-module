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
    PRINTF_QUANTA("SQL QUERY IN BLOCK\n");
  }
  if (hp_globals.current_monitored_function > 0
  && hp_globals.current_monitored_function < QUANTA_MON_MAX_MONITORED_FUNCTIONS) {
    hp_globals.monitored_function_sql_cpu_cycles[hp_globals.current_monitored_function] += cycles;
    hp_globals.monitored_function_sql_queries_count[hp_globals.current_monitored_function]++;
    if (!(block = block_stack_top()))
      PRINTF_QUANTA("SQL QUERY IN FUNCTION [%d] %s\n", hp_globals.current_monitored_function,
        hp_globals_monitored_function_names()[hp_globals.current_monitored_function]);
    return 0;
  } else if (hp_globals.last_monitored_function > 0
  && hp_globals.last_monitored_function < QUANTA_MON_MAX_MONITORED_FUNCTIONS) {
    hp_globals.monitored_function_sql_cpu_cycles_after[hp_globals.last_monitored_function] += cycles;
    hp_globals.monitored_function_sql_queries_count_after[hp_globals.last_monitored_function]++;
    if (!(block = block_stack_top()))
      PRINTF_QUANTA("SQL QUERY AFTER FUNCTION [%d] %s\n", hp_globals.last_monitored_function,
        hp_globals_monitored_function_names()[hp_globals.last_monitored_function]);
    return 0;
  } else {
    PRINTF_QUANTA("SQL query outside monitored function\n");
    return -1;
  }
}

static int match_function_and_class(int start, zend_execute_data *data, const char *name TSRMLS_DC) {
  const char *class_name = NULL;
  size_t class_name_len;

  if (!data || !name)
    return QUANTA_MON_MAX_MONITORED_FUNCTIONS -1;
  if (data->function_state.function->common.scope) {
    class_name = data->function_state.function->common.scope->name;
  } else if (data->object) {
    class_name = Z_OBJCE(*data->object)->name;
  }
  if (!class_name)
    return QUANTA_MON_MAX_MONITORED_FUNCTIONS -1;
  class_name_len = strlen(class_name);
  // PRINTF_QUANTA("Function %s::%s matched (maybe)\n", class_name, name);
  for (; hp_globals_monitored_function_names()[start]; start++) {
    // PRINTF_QUANTA("CHECKING %s::%s against %s\n", class_name, name, hp_globals_monitored_function_names()[start]);
    if (!strncmp(class_name, hp_globals_monitored_function_names()[start], class_name_len)
    && !strncmp(hp_globals_monitored_function_names()[start] + class_name_len, "::", 2)
    && !strcmp(name, hp_globals_monitored_function_names()[start] + class_name_len + 2))
      return start;
  }
  return start;
}

/**
 * Check if this entry should be ignored, first with a conservative Bloomish
 * filter then with an exact check against the function names.
 *
 * @author ch
 * return -1 if we don't monitor specifically this function, -2 if we don't monitor at all
 */
int qm_begin_profiling(const char *curr_func, zend_execute_data *execute_data TSRMLS_DC) {
  int i;
  int j;
  uint64_t start;
  uint64_t end;
  uint8_t hash_code;

  // dprintf(hp_globals.internal_match_counters.fd, "%s %s\n",
  //   curr_func, hp_get_class_name(execute_data TSRMLS_CC));

  start = cycle_timer();
  i = hp_match_monitored_function(curr_func, execute_data TSRMLS_CC);
  end = cycle_timer();
  hp_globals.internal_match_counters.cycles += end - start;
  // start = cycle_timer();
  // hash_code = hp_inline_hash(curr_func);
  // if (hp_monitored_functions_filter_collision(hash_code))
  //   j = match_function_and_class(0, execute_data, curr_func TSRMLS_CC);
  // else
  //   j = -1;
  // if (j == QUANTA_MON_MAX_MONITORED_FUNCTIONS - 1)
  //   j = -1;
  // end = cycle_timer();
  // hp_globals.internal_match_counters.hash_cycles += end - start;
  //
  // if (i != j)
  //   printf("FUCKED UP MATCH %d != %d\n", i, j);
  if (i < 0
  || !hp_globals_monitored_function_names()[i] || !*hp_globals_monitored_function_names()[i]
  || (i < POS_ENTRY_EVENTS_ONLY && hp_globals.profiler_level == QUANTA_MON_MODE_EVENTS_ONLY)) {
    return -1; /* False match, we have nothing */
  }
  //TODO! testing
  // return -1;
  if (i == POS_ENTRY_APP_RUN) {
    fetch_magento_version(TSRMLS_C);
  }
  // Init timestamp after version fetch to not interfere with function timers
  hp_globals.monitored_function_tsc_start[i] = cycle_timer();
  if (i != POS_ENTRY_PDO_EXECUTE && i != POS_ENTRY_TOHTML && *hp_globals_monitored_function_names()[i]) {
    PRINTF_QUANTA("BEGIN FUNCTION %d %s\n", i, hp_globals_monitored_function_names()[i]);
    hp_globals.current_monitored_function = i;
    hp_globals.last_monitored_function = -1;
  }
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

  return i; /* No, bailout */
}

int qm_end_profiling(int profile_curr, zend_execute_data *execute_data TSRMLS_DC) {
  hp_globals.monitored_function_tsc_stop[profile_curr] = cycle_timer();
  if (profile_curr != POS_ENTRY_PDO_EXECUTE && profile_curr != POS_ENTRY_TOHTML && *hp_globals_monitored_function_names()[profile_curr]) {
    PRINTF_QUANTA("END FUNCTION %d %s\n", profile_curr, hp_globals_monitored_function_names()[profile_curr]);
    hp_globals.current_monitored_function = -1;
    hp_globals.last_monitored_function = profile_curr;
  }

  if (profile_curr == POS_ENTRY_TOHTML)
    qm_after_tohtml(execute_data TSRMLS_CC);
  if (profile_curr == POS_ENTRY_PDO_EXECUTE)
    qm_record_sql_timers();
  if (profile_curr == POS_ENTRY_EV_BEFORE_SAVE)
    return qm_record_reindex_event(profile_curr, execute_data TSRMLS_CC);

  return profile_curr;
}
