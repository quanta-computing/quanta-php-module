#include "quanta_mon.h"

static void init_profiled_application_counters(profiled_application_t *app) {
  size_t i;

  for (i = 0; i < app->nb_functions; i++) {
    memset(&app->functions[i].tsc, 0, sizeof(app->functions[i].tsc));
    memset(&app->functions[i].sql_queries, 0, sizeof(app->functions[i].sql_queries));
  }
}

void init_profiled_application(profiled_application_t *app TSRMLS_DC) {
  app->function_stack = NULL;
  app->sql_queries.first = NULL;
  app->sql_queries.last = NULL;
  init_profiled_application_counters(app);
  if (app->create_context)
    app->context = app->create_context(app TSRMLS_CC);
  else
    app->context = NULL;
}

void clean_profiled_application(profiled_application_t *app TSRMLS_DC) {
  profiled_function_stack_t *stack_entry;
  profiled_function_stack_t *prev_stack_entry;
  sql_query_record_t *query;
  sql_query_record_t *next_query;
  size_t i;

  if (!app)
    return;
  for (stack_entry = app->function_stack; stack_entry; stack_entry = prev_stack_entry) {
    prev_stack_entry = stack_entry->prev;
    efree(stack_entry);
  }
  i = 0;
  for (query = app->sql_queries.first; query; query = next_query) {
    i++;
    next_query = query->next;
    PRINTF_QUANTA("SQL QUERY: %s\n", query->query);
    efree(query->query);
    efree(query);
  }
  PRINTF_QUANTA("TOTAL SQL QUERIES: %zu\n", i);
  if (app->cleanup_context)
    app->cleanup_context(app TSRMLS_CC);
}
