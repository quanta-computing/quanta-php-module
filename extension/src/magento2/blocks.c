#include "quanta_mon.h"

static zval *get_block_name(zend_execute_data *execute_data) {
  zval *ret;

  if (!execute_data || !execute_data->prev_execute_data
  || !(ret = safe_get_argument(execute_data->prev_execute_data, 1, IS_STRING))) {
    PRINTF_QUANTA("Cannot get block name parameter\n");
    return NULL;
  }
  return ret;
}

static zval *get_block_object(zval *this, zval *block_name) {
  zval *blocks = NULL;
  zval *block = NULL;

  blocks = zend_hash_find_compat(Z_OBJPROP_P(this), "\0*\0_blocks", sizeof("\0*\0_blocks") - 1);
  if (!blocks) {
    PRINTF_QUANTA("_blocks not found\n");
    return NULL;
  }
#if PHP_MAJOR_VERSION >= 7
  if (Z_TYPE_P(blocks) == IS_INDIRECT)
    blocks = Z_INDIRECT_P(blocks);
#endif
  if (Z_TYPE_P(blocks) != IS_ARRAY) {
    PRINTF_QUANTA("Cannot get block object, _blocks is a %d (expected %d)\n",
      blocks ? Z_TYPE_P(blocks): 0, IS_ARRAY);
    return NULL;
  }
  block = zend_hash_find_compat(Z_ARRVAL_P(blocks), Z_STRVAL_P(block_name), Z_STRLEN_P(block_name));
  if (!block || Z_TYPE_P(block) != IS_OBJECT) {
    PRINTF_QUANTA("Cannot get block %s (it's a %d)\n", Z_STRVAL_P(block_name),
      block ? Z_TYPE_P(block) : 0);
    return NULL;
  }
  return block;
}

int magento2_block_before_render(profiled_application_t *app, profiled_function_t *function,
zend_execute_data *execute_data) {
  zval *this;
  zval *block_name;
  zval *zblock;
  magento_block_t *block = NULL;
  magento_context_t *context = (magento_context_t *)app->context;

  (void)function;
  if (!(this = get_prev_this(execute_data))
  || !(block_name = get_block_name(execute_data))
  || !(zblock = get_block_object(this, block_name))
  || !(block = ecalloc(1, sizeof(magento_block_t)))
  || !(block->name = estrdup(Z_STRVAL_P(block_name)))) {
    PRINTF_QUANTA("Cannot get block info\n");
    efree(block);
    return -1;
  }
  block->class = magento_get_block_class_name(zblock);
  if (block->class)
    block->class_file = magento_get_block_class_file(zblock);
  block->template = magento_get_block_attr("\0*\0_template",
    sizeof("\0*\0_template") - 1, zblock);
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
