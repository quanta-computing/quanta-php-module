#include "quanta_mon.h"

static void init_profiled_application_counters(profiled_application_t *app) {
  size_t i;

  for (i = 0; i < app->nb_functions; i++) {
    memset(&app->functions[i].tsc, 0, sizeof(app->functions[i].tsc));
    memset(&app->functions[i].sql_counters, 0, sizeof(app->functions[i].sql_counters));
  }
}

void init_profiled_application(profiled_application_t *app TSRMLS_DC) {
  app->current_function = NULL;
  app->last_function = NULL;
  init_profiled_application_counters(app);
  if (app->create_context)
    app->context = app->create_context(app TSRMLS_CC);
  else
    app->context = NULL;
}

void clean_profiled_application(profiled_application_t *app TSRMLS_DC) {
  if (!app)
    return;
  if (app->cleanup_context)
    app->cleanup_context(app TSRMLS_CC);
}
