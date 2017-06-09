#include "quanta_mon.h"
//
// static int depth = 0;
//
// static void block_var_dump(zval *val TSRMLS_DC, int num_args, va_list args, zend_hash_key *key) {
//   PRINTF_QUANTA("BLOCK VAR !!\n");
//   if (Z_TYPE_P(val) == IS_INDIRECT)
//     val = Z_INDIRECT_P(val);
//   if (Z_TYPE_P(val) == IS_REFERENCE)
//     val = Z_REFVAL_P(val);
//   if (Z_TYPE_P(val) == IS_STRING)
//     PRINTF_QUANTA("BLOCK VAR [%zu] %*s (%zu) = %s\n", key->h, (int)ZSTR_LEN(key->key), ZSTR_VAL(key->key), ZSTR_LEN(key->key), Z_STRVAL_P(val));
//   else
//     PRINTF_QUANTA("BLOCK VAR [%zu] %*s (%zu) = %d\n", key->h, (int)ZSTR_LEN(key->key), ZSTR_VAL(key->key), ZSTR_LEN(key->key), Z_TYPE_P(val));
// }
//
// static void block_vars_dump(zval *block TSRMLS_DC) {
//   zval *vars;
//   vars = zend_hash_find_compat(Z_OBJPROP_P(block), "vars", sizeof("vars") - 1);
//   if (Z_TYPE_P(vars) == IS_INDIRECT)
//     vars = Z_INDIRECT_P(vars);
//   if (Z_TYPE_P(vars) == IS_REFERENCE)
//     vars = Z_REFVAL_P(vars);
//   if (Z_TYPE_P(vars) != IS_ARRAY)
//     PRINTF_QUANTA("vars is not an array\n");
//   else
//     zend_hash_apply_with_arguments(Z_ARRVAL_P(vars), block_var_dump, 0, NULL TSRMLS_CC);
// }

static zval *get_block_var(zval *block, const char * key, int type TSRMLS_DC) {
  zval *data;
  zval *ret;

  if (!(data = zend_hash_find_compat(Z_OBJPROP_P(block), "vars", sizeof("vars") - 1))) {
    PRINTF_QUANTA("Cannot fetch block vars\n");
    return NULL;
  }
#if PHP_MAJOR_VERSION >= 7
  if (Z_TYPE_P(data) == IS_INDIRECT)
    data = Z_INDIRECT_P(data);
#endif
  if (Z_TYPE_P(data) == IS_REFERENCE)
    data = Z_REFVAL_P(data);
  if (Z_TYPE_P(data) != IS_ARRAY) {
    PRINTF_QUANTA("vars is not an array, it's a %d\n", Z_TYPE_P(data));
    return NULL;
  }
  if (!(ret = zend_hash_find_compat(Z_ARRVAL_P(data), key, strlen(key)))) {
    PRINTF_QUANTA("Cannot fetch %s in vars\n", key);
    return NULL;
  }
  if (Z_TYPE_P(ret) != type) {
    PRINTF_QUANTA("%s is not a %d, it's a %d :(\n", key, type, Z_TYPE_P(ret));
    return NULL;
  }
  return ret;
}

int oro_before_render_block(profiled_application_t *app, profiled_function_t *function,
zend_execute_data *execute_data TSRMLS_DC) {
  oro_context_t *context = (oro_context_t *)app->context;
  oro_block_t *current_block;
  oro_block_t *block;
  zval *zblock;
  zval *resource;
  zval *id;
  zval twig;
  zval template;

  zblock = safe_get_argument(execute_data, 1, IS_OBJECT);
  resource = safe_get_argument(execute_data, 2, IS_ARRAY);
  if (!zblock || !resource) {
    PRINTF_QUANTA("Cannot parse arguments\n");
    return -1;
  }
  if (!(id = get_block_var(zblock, "id", IS_STRING TSRMLS_CC))) {
    PRINTF_QUANTA("Cannot get block id\n");
    return -1;
  }
  if ((current_block = oro_block_stack_top(context))
  && !strcmp(Z_STRVAL_P(id), current_block->name)) {
    PRINTF_QUANTA("Same block id (%s), skipping this block\n", Z_STRVAL_P(id));
    return -1;
  }
  if (!(block = ecalloc(1, sizeof(*block)))
  || !(block->name = estrdup(Z_STRVAL_P(id))))
    return -1;
  if (!safe_call_function("current", &twig, IS_OBJECT, 1, resource TSRMLS_CC)
  && !safe_call_method(&twig, "getTemplateName", &template, IS_STRING, 0, NULL TSRMLS_CC)) {
    block->template = estrdup(Z_STRVAL(template));
  } else {
    PRINTF_QUANTA("Cannot get template\n");
  }
  oro_block_stack_push(context, block);
  if (context->blocks.first == NULL)
    context->blocks.first = block;
  else
    context->blocks.last->next = block;
  context->blocks.last = block;
  block->tsc_renderize_first_start = cycle_timer();
  // PRINTF_QUANTA("START BLOCK %s (%s)\n", block->name, block->template);
  return 0;
}

int oro_after_render_block(profiled_application_t *app, profiled_function_t *function,
zend_execute_data *execute_data TSRMLS_DC) {
  oro_context_t *context = (oro_context_t *)app->context;
  oro_block_t *block;
  oro_block_t *parent;

  if (!(block = oro_block_stack_pop(context))) {
    PRINTF_QUANTA("Block not found on stack\n");
    return -1;
  }
  block->tsc_renderize_last_stop = cycle_timer();
  if ((parent = oro_block_stack_top(context))) {
    parent->renderize_children_cycles += block->tsc_renderize_last_stop -
      block->tsc_renderize_first_start;
  }
  // PRINTF_QUANTA("END BLOCK %s (%s)\n", block->name, block->template);
  return 0;
}

int oro_before_process_block(profiled_application_t *app, profiled_function_t *function,
zend_execute_data *execute_data TSRMLS_DC) {
  // PRINTF_QUANTA("BEFORE PROCESS BLOCK\n");
  return 0;
}
