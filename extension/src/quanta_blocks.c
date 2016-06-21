#include "quanta_mon.h"

static zval *get_this(zend_execute_data *execute_data TSRMLS_DC) {
  zval *this;

  if (!execute_data || !execute_data->prev_execute_data
  || !(this = execute_data->prev_execute_data->current_this)
  || Z_TYPE_P(this) != IS_OBJECT) {
    PRINTF_QUANTA ("Cannot get this\n");
    return NULL;
  }
  return this;
}

static char *get_block_attr(const char *key, size_t key_len, zval *this TSRMLS_DC) {
  zval **data;
  HashTable *block;

  block = Z_OBJPROP_P(this);
  if (zend_hash_find(block, key, key_len, (void **)&data) == FAILURE
  || Z_TYPE_PP(data) != IS_STRING) {
    // PRINTF_QUANTA("Cannot extract attr %s from block\n", key + 3);
    return NULL;
  } else {
    return estrdup(Z_STRVAL_PP(data));
  }
}

int qm_before_tohtml(int profile_curr, zend_execute_data *execute_data TSRMLS_DC) {
  zval *this;
  const char *class_name;
  zend_uint class_name_len;
  magento_block_t *block;

  if (!(this = get_this(execute_data TSRMLS_CC)))
    return -1;
  if (!Z_OBJ_HANDLER_P(this, get_class_name)
  || Z_OBJ_HANDLER_P(this, get_class_name)(this, &class_name, &class_name_len, 0 TSRMLS_CC) != SUCCESS) {
    PRINTF_QUANTA ("qm_extract_this_before_tohtml: 'this' is an object of unknown class\n");
    return -1;
  }
  if (!(block = ecalloc(1, sizeof(magento_block_t))))
    return -1;
  block->class = estrndup(class_name, class_name_len);

  block->name = get_block_attr("\0*\0_nameInLayout", 17, this TSRMLS_CC);
  block->template = get_block_attr("\0*\0_template", 13, this TSRMLS_CC);
  if (!block->name) {
    // PRINTF_QUANTA("Block does not have a nameInLayout (template: %s / class: %s)\n",
    //   block->template, block->class);
    efree(block->name);
    efree(block->class);
    efree(block->template);
    efree(block);
    return -1;
  }
  PRINTF_QUANTA("BLOCK FOUND %s, %s, %s\n", block->name, block->template, block->class);
  block_stack_push(block);
  if (hp_globals.magento_blocks_first == NULL)
    hp_globals.magento_blocks_first = block;
  else
    hp_globals.magento_blocks_last->next = block;
  hp_globals.magento_blocks_last = block;
  block->tsc_renderize_first_start = cycle_timer();
  return profile_curr;
}

int qm_after_tohtml(zend_execute_data *execute_data TSRMLS_DC) {
  zval *this;
  char *name;
  magento_block_t *block;
  magento_block_t *parent;

  if (!(this = get_this(execute_data TSRMLS_CC))
  || !(name = get_block_attr("\0*\0_nameInLayout", 17, this TSRMLS_CC))) {
    // PRINTF_QUANTA("nameInLayout not found in afterToHtml\n");
    return -1;
  }
  if (!(block = block_stack_pop())) {
    PRINTF_QUANTA("Block %s not found\n", name);
    efree(name);
    return -1;
  }
  block->tsc_renderize_last_stop = cycle_timer();
  if ((parent = block_stack_top())) {
    parent->renderize_children_cycles += block->tsc_renderize_last_stop -
      block->tsc_renderize_first_start;
  }
  efree(name);
  return 0;
}
