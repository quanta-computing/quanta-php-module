#include "quanta_mon.h"


static zval *get_this(zend_execute_data *execute_data TSRMLS_DC) {
  zval *this;
  const char *class_name;
  zend_uint class_name_len;

  if (!execute_data || !execute_data->prev_execute_data
  || !(this = execute_data->prev_execute_data->current_this)
  || Z_TYPE_P(this) != IS_OBJECT) {
    PRINTF_QUANTA ("Cannot get this (pun intended)\n");
    return NULL;
  }
  if (!Z_OBJ_HANDLER_P(this, get_class_name)
  || Z_OBJ_HANDLER_P(this, get_class_name)(this, &class_name, &class_name_len, 0 TSRMLS_CC) != SUCCESS) {
    PRINTF_QUANTA ("GET THIS: 'this' is an object of unknown class\n");
    return NULL;
  }
  return this;
}

static zval *get_block_name(zend_execute_data *execute_data TSRMLS_DC) {
  if (!execute_data || !execute_data->prev_execute_data
  || execute_data->prev_execute_data->function_state.arguments[0] != (void *)(1)
  || Z_TYPE_P((zval *)(execute_data->prev_execute_data->function_state.arguments[-1])) != IS_STRING) {
    PRINTF_QUANTA("Cannot get block name parameter\n");
    return NULL;
  }
  return (zval *)(execute_data->prev_execute_data->function_state.arguments[-1]);
}

static char *get_block_class_name(zval *block TSRMLS_DC) {
  const char *class_name;
  zend_uint class_name_len;

  if (Z_TYPE_P(block) != IS_OBJECT
  || !Z_OBJ_HANDLER_P(block, get_class_name)
  || Z_OBJ_HANDLER_P(block, get_class_name)(block, &class_name, &class_name_len, 0 TSRMLS_CC) != SUCCESS) {
    return NULL;
  }
  return estrdup(class_name);
}

static char *get_block_class_file(const char *class_name TSRMLS_DC) {
  zval *reflection;
  zval *params[1];
  zval zclass_file;
  char *class_file = NULL;

  ZVAL_NULL(&zclass_file);
  MAKE_STD_ZVAL(params[0]);
  ZVAL_STRING(params[0], class_name, 1);
  if (!(reflection = safe_new("ReflectionClass", 1, params TSRMLS_CC))
  || safe_call_method(reflection, "getFileName", &zclass_file, IS_STRING, 0, NULL TSRMLS_CC)
  || !(class_file = estrdup(Z_STRVAL(zclass_file)))) {
    PRINTF_QUANTA("Cannot get file for class %s\n", class_name);
    goto end;
  }

end:
  zval_dtor(&zclass_file);
  FREE_ZVAL(params[0]);
  if (reflection)
    FREE_ZVAL(reflection);
  return class_file;
}

static zval *get_block_object(zval *this, zval *block_name TSRMLS_DC) {
  zval **blocks = NULL;
  zval **data = NULL;
  int ret;

  ret = zend_hash_find(Z_OBJPROP_P(this),
    "\0*\0_blocks", sizeof("\0*\0_blocks"), (void **)&blocks);
  if (ret != SUCCESS || Z_TYPE_PP(blocks) != IS_ARRAY) {
    return NULL;
  }
  ret = zend_hash_find(Z_ARRVAL_PP(blocks), Z_STRVAL_P(block_name),
    Z_STRLEN_P(block_name) + 1, (void **)&data);
  if (ret != SUCCESS || Z_TYPE_PP(data) != IS_OBJECT) {
    if (ret == SUCCESS)
      PRINTF_QUANTA("Block %s is a %d\n", Z_STRVAL_P(block_name), Z_TYPE_PP(data));
    else
      PRINTF_QUANTA("block %s not in blocks\n", Z_STRVAL_P(block_name));
    return NULL;
  }
  return *data;
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
  zval *block_name;
  zval *block_object;
  magento_block_t *block;

  if (!(this = get_this(execute_data TSRMLS_CC)))
    return -1;
  if (!(block_name = get_block_name(execute_data TSRMLS_CC))) {
    PRINTF_QUANTA("Cannot get block name\n");
    return -1;
  }
  if (!(block_object = get_block_object(this, block_name TSRMLS_CC))) {
    PRINTF_QUANTA("Cannot get block object\n");
    return -1;
  }
  if (!(block = ecalloc(1, sizeof(magento_block_t)))) {
    PRINTF_QUANTA("Cannot allocate memory\n");
    return -1;
  }
  //TODO! check for null pointers
  block->name = estrdup(Z_STRVAL_P(block_name));
  block->class = get_block_class_name(block_object TSRMLS_CC);
  block->class_file = get_block_class_file(block->class TSRMLS_CC);
  block->template = get_block_attr("\0*\0_template", sizeof("\0*\0_template"), block_object TSRMLS_CC);
  gc_collect_cycles();
  // block->class = estrndup(class_name, class_name_len);
  // block->name = get_block_attr("\0*\0_nameInLayout", 17, this TSRMLS_CC);
  // block->template = get_block_attr("\0*\0_template", 13, this TSRMLS_CC);
  // if (!block->name) {
  //   efree(block->name);
  //   efree(block->class);
  //   efree(block->template);
  //   efree(block);
  //   return -1;
  // }
  PRINTF_QUANTA("BEGIN BLOCK %s\n", block->name);
  // PRINTF_QUANTA("BLOCK FOUND %s, %s, %s\n", block->name, block->template, block->class);
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
  zval *block_name;
  zval *block_object;
  // char *name = NULL;
  magento_block_t *block;
  magento_block_t *parent;


  if (!(this = get_this(execute_data TSRMLS_CC))
  || (!(block_name = get_block_name(execute_data TSRMLS_CC)))
  || (!(block_object = get_block_object(this, block_name TSRMLS_CC)))) {
    // || !(name = get_block_attr("\0*\0_nameInLayout", 17, this TSRMLS_CC))) {
    // PRINTF_QUANTA("nameInLayout not found in afterToHtml\n");
    return -1;
  }
  if (!(block = block_stack_pop())) {
    PRINTF_QUANTA("Block %s not found\n", Z_STRVAL_P(block_name));
    // efree(name);
    return -1;
  }
  PRINTF_QUANTA("END BLOCK %s\n", Z_STRVAL_P(block_name));
  block->tsc_renderize_last_stop = cycle_timer();
  if ((parent = block_stack_top())) {
    parent->renderize_children_cycles += block->tsc_renderize_last_stop -
      block->tsc_renderize_first_start;
  }
  // efree(name);
  return 0;
}
