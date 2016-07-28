#include "quanta_mon.h"

static zval *get_block_name(zend_execute_data *execute_data TSRMLS_DC) {
  zval *ret;

  if (!execute_data || !execute_data->prev_execute_data
  || !(ret = safe_get_argument(execute_data->prev_execute_data, 1, IS_STRING))) {
    PRINTF_QUANTA("Cannot get block name parameter\n");
    return NULL;
  }
  return ret;
}

static char *get_block_class_name(zval *block TSRMLS_DC) {
  const char *class_name;

  if (!(class_name = get_obj_class_name(block TSRMLS_CC)))
    return NULL;
  return estrdup(class_name);
}

static char *get_block_class_file(zval *block) {
  zend_class_entry *ce;

  if (!(ce = Z_OBJCE_P(block))
  || ce->type != ZEND_USER_CLASS
  || !ce->info.user.filename) {
    PRINTF_QUANTA("Cannot get block class file\n");
    return NULL;
  }
#if PHP_MAJOR_VERSION < 7
  return estrdup(ce->info.user.filename);
#else
  return ZSTR_VAL(ce->info.user.filename) ? estrdup(ZSTR_VAL(ce->info.user.filename)) : NULL;
#endif
}

static zval *get_block_object(zval *this, zval *block_name TSRMLS_DC) {
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

static char *get_block_attr(const char *key, size_t key_len, zval *this TSRMLS_DC) {
  zval *data;
  HashTable *block;

  block = Z_OBJPROP_P(this);
  if (!(data = zend_hash_find_compat(block, key, key_len))) {
    PRINTF_QUANTA("Cannot extract attr %s from block\n", key + 3);
    return NULL;
  }
#if PHP_MAJOR_VERSION >= 7
  if (Z_TYPE_P(data) == IS_INDIRECT)
    data = Z_INDIRECT_P(data);
#endif
  if (Z_TYPE_P(data) != IS_STRING) {
    PRINTF_QUANTA("Block attr %s is not a string (it's a %d)\n", key + 3, Z_TYPE_P(data));
    return NULL;
  }
  return estrdup(Z_STRVAL_P(data));
}

int magento2_block_before_render(profiled_application_t *app,
zend_execute_data *execute_data TSRMLS_DC) {
  zval *this;
  zval *block_name;
  zval *zblock;
  magento_block_t *block = NULL;
  magento_context_t *context = (magento_context_t *)app->context;

  if (!(this = get_prev_this(execute_data TSRMLS_CC))
  || !(block_name = get_block_name(execute_data TSRMLS_CC))
  || !(zblock = get_block_object(this, block_name TSRMLS_CC))
  || !(block = ecalloc(1, sizeof(magento_block_t)))
  || !(block->name = estrdup(Z_STRVAL_P(block_name)))) {
    PRINTF_QUANTA("Cannot get block info\n");
    efree(block);
    return -1;
  }
  block->class = get_block_class_name(zblock TSRMLS_CC);
  if (block->class)
    block->class_file = get_block_class_file(zblock TSRMLS_CC);
  block->template = get_block_attr("\0*\0_template", sizeof("\0*\0_template") - 1, zblock TSRMLS_CC);
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

int magento2_block_after_render(profiled_application_t *app,
zend_execute_data *execute_data TSRMLS_DC) {
  zval *this;
  zval *block_name;
  zval *block_object;
  magento_block_t *block;
  magento_block_t *parent;
  magento_context_t *context = (magento_context_t *)app->context;

  if (!(this = get_this(execute_data TSRMLS_CC))
  || (!(block_name = get_block_name(execute_data TSRMLS_CC)))
  || (!(block_object = get_block_object(this, block_name TSRMLS_CC)))) {
    return -1;
  }
  if (!(block = block_stack_pop(context))) {
    PRINTF_QUANTA("Block %s not found\n", Z_STRVAL_P(block_name));
    return -1;
  }
  block->tsc_renderize_last_stop = cycle_timer();
  if ((parent = block_stack_top(context))) {
    parent->renderize_children_cycles += block->tsc_renderize_last_stop -
      block->tsc_renderize_first_start;
  }
  PRINTF_QUANTA("END BLOCK %s\n", Z_STRVAL_P(block_name));
  return 0;
}
