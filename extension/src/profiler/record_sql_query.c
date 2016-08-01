#include "quanta_mon.h"

int qm_record_sql_query(profiled_application_t *app, profiled_function_t *function,
zend_execute_data *data TSRMLS_DC) {
  uint64_t cycles;

  (void)data;
  cycles = function->tsc.last_stop - function->tsc.last_start;
  if (app->current_function) {
    app->current_function->sql_counters.cycles += cycles;
    app->current_function->sql_counters.count++;
  } else if (app->last_function) {
    app->last_function->sql_counters.cycles_after += cycles;
    app->last_function->sql_counters.count_after++;
  }
  return 0;
}
