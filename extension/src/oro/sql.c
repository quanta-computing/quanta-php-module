#include "quanta_mon.h"

int oro_record_sql_query(profiled_application_t *app, profiled_function_t *function,
zend_execute_data *data) {
  oro_block_t *block;

  if (qm_record_sql_query(app, function, data))
    return -1;
  if ((block = oro_block_stack_top((oro_context_t *)app->context))) {
    block->sql_cpu_cycles += function->tsc.last_stop - function->tsc.last_start;
    block->sql_queries_count++;
  }
  return 0;
}
