#include "quanta_mon.h"

static void init_profiled_application_counters(profiled_application_t *app) {
  size_t i;

  for (i = 0; i < app->nb_functions; i++) {
    memset(&app->functions[i].tsc, 0, sizeof(app->functions[i].tsc));
  }
}

void init_profiled_application(profiled_application_t *app) {
  app->sql_queries.first = NULL;
  app->sql_queries.last = NULL;
  init_profiled_application_counters(app);
  if (app->create_context)
    app->context = app->create_context(app);
  else
    app->context = NULL;
}

void clean_profiled_application(profiled_application_t *app) {
  sql_query_record_t *query;
  sql_query_record_t *next_query;
  size_t sql_count;

  if (!app)
    return;
  sql_count = 0;
  for (query = app->sql_queries.first; query; query = next_query) {
    sql_count++;
    next_query = query->next;
    // PRINTF_QUANTA("SQL QUERY: %s\n", query->query);
    efree(query->query);
    efree(query);
  }
  PRINTF_QUANTA("TOTAL SQL QUERIES: %zu\n", sql_count);
  if (app->cleanup_context)
    app->cleanup_context(app);
}
