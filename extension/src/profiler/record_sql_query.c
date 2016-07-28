#include "quanta_mon.h"

int qm_record_sql_query(profiled_application_t *app, zend_execute_data *data TSRMLS_DC) {
  (void)app;
  (void)data;
  return 0;
  // TODO! Oh hi...
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

}
