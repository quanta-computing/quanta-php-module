#include "quanta_mon.h"

int qm_record_sql_query(profiled_application_t *app, profiled_function_t *function,
zend_execute_data *data TSRMLS_DC) {
  sql_query_record_t *query;
  // #if PHP_VERSION_ID >= 70000
  //     pdo_stmt_t *stmt = (pdo_stmt_t*)((char*)Z_OBJ_P(&data->This) - Z_OBJ_HT_P(&data->This)->offset);
  // #else
  //     pdo_stmt_t *stmt = (pdo_stmt_t*)zend_object_store_get_object_by_handle(
  //       Z_OBJ_HANDLE_P(data->object TSRMLS_CC);
  // #endif

  (void)data;
  if (!(query = ecalloc(1, sizeof(*query))))
    return 0;
  query->tsc_start = function->tsc.last_start;
  query->tsc_stop = function->tsc.last_stop;
  // if (stmt && stmt->query_string)
  //   query->query = estrdup(stmt->query_string);
  if (!app->sql_queries.first)
    app->sql_queries.first = query;
  if (app->sql_queries.last)
    app->sql_queries.last->next = query;
  app->sql_queries.last = query;
  return 0;
}
