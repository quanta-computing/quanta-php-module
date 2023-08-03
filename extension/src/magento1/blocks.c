#include "quanta_mon.h"

int magento1_block_before_render(profiled_application_t *app, profiled_function_t *function,
zend_execute_data *execute_data) {
  zval *this;
  magento_block_t *block = NULL;
  magento_context_t *context = (magento_context_t *)app->context;

  if (!(this = get_this(execute_data))
  || !(block = ecalloc(1, sizeof(*block))))
    return -1;
  block->name = magento_get_block_attr("\0*\0_nameInLayout",
    sizeof("\0*\0_nameInLayout") - 1, this);
  block->class = magento_get_block_class_name(this);
  if (block->class)
    block->class_file = magento_get_block_class_file(this);
  block->template = magento_get_block_attr("\0*\0_template",
    sizeof("\0*\0_template") - 1, this);
  if (!block->name) {
    PRINTF_QUANTA("Block does not have a nameInLayout (template: %s / class: %s)\n",
      block->template, block->class);
    efree(block->name);
    efree(block->class);
    efree(block->template);
    efree(block);
    return -1;
  }
  block_stack_push(context, block);
  if (context->blocks.first == NULL)
    context->blocks.first = block;
  else
    context->blocks.last->next = block;
  context->blocks.last = block;
  block->tsc_renderize_first_start = cycle_timer();
  PRINTF_QUANTA("BEGIN BLOCK %s\n", block->name);
  return 0;
}
