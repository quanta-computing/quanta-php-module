#include "quanta_mon.h"

char *magento_get_block_class_name(zval *block) {
  const char *class_name;

  if (!(class_name = get_obj_class_name(block)))
    return NULL;
  return estrdup(class_name);
}

char *magento_get_block_class_file(zval *block) {
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
  return estrdup(ZSTR_VAL(ce->info.user.filename));
#endif
}

char *magento_get_block_attr(const char *key, size_t key_len, zval *this) {
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

int magento_block_after_render(profiled_application_t *app, profiled_function_t *function,
zend_execute_data *execute_data) {
  magento_block_t *block;
  magento_block_t *parent;
  magento_context_t *context = (magento_context_t *)app->context;

  (void)function;
  if (!(block = block_stack_pop(context))) {
    PRINTF_QUANTA("Block not found on stack\n");
    return -1;
  }
  block->tsc_renderize_last_stop = cycle_timer();
  if ((parent = block_stack_top(context))) {
    parent->renderize_children_cycles += block->tsc_renderize_last_stop -
      block->tsc_renderize_first_start;
  }
  PRINTF_QUANTA("END BLOCK %s\n", block->name);
  return 0;
}
