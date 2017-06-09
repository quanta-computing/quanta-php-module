#include "quanta_mon.h"

static oro_context_t oro_context = {
  NULL, NULL,
  {NULL, NULL},
  NULL
};

void *oro_init_context(profiled_application_t *app TSRMLS_DC) {
  oro_context.version = NULL;
  oro_context.edition = NULL;
  oro_context.blocks.first = NULL;
  oro_context.blocks.last = NULL;
  oro_context.block_stack = NULL;
  return (void *)&oro_context;
}

void oro_cleanup_context(profiled_application_t *app TSRMLS_DC) {
  oro_context_t *context = (oro_context_t *)app->context;

  efree(context->version);
  efree(context->edition);
  while (oro_block_stack_pop(context));
  oro_init_context(app TSRMLS_CC);
}
