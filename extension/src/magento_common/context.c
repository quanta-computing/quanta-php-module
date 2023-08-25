#include "quanta_mon.h"

static magento_context_t magento_context = {
  NULL, NULL,
  {NULL, NULL},
  NULL
};

void *magento_init_context(profiled_application_t *app) {
  magento_context.blocks.first = NULL;
  magento_context.blocks.last = NULL;
  magento_context.block_stack = NULL;
  magento_context.version = NULL;
  magento_context.edition = NULL;
  return (void *)&magento_context;
}

void magento_cleanup_context(profiled_application_t *app) {
  magento_context_t *context = (magento_context_t *)app->context;

  efree(context->version);
  efree(context->edition);
  while (block_stack_pop(context));
  magento_init_context(app);
}
