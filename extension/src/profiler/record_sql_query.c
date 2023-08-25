#include "quanta_mon.h"

int qm_record_sql_query(profiled_application_t *app, profiled_function_t *function,
zend_execute_data *data) {
  sql_query_record_t *query;

  (void)data;
  if (!(query = ecalloc(1, sizeof(*query))))
    return 0;
  query->tsc_start = function->tsc.last_start;
  query->tsc_stop = function->tsc.last_stop;
  if (!app->sql_queries.first)
    app->sql_queries.first = query;
  if (app->sql_queries.last)
    app->sql_queries.last->next = query;
  app->sql_queries.last = query;
  return 0;
}
