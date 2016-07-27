#include "quanta_mon.h"

#if PHP_MAJOR_VERSION < 7
static int _safe_call_function(char *function_name, zval *object, zval *ret_val, int ret_type,
size_t call_params_count, zval *call_params[] TSRMLS_DC) {
  int ret;
  zval *params[1];
  zval is_callable;
  zval func;

  MAKE_STD_ZVAL(params[0]);
  ZVAL_STRING_COMPAT(&func, "is_callable");
  if (object) {
    array_init(params[0]);
    add_next_index_zval(params[0], object);
    add_next_index_string(params[0], function_name, 1);
  } else {
    ZVAL_STRING_COMPAT(params[0], function_name);
  }
  ret = call_user_function(CG(function_table), NULL, &func, &is_callable, 1, params TSRMLS_CC);
  if (ret != SUCCESS || Z_TYPE(is_callable) != IS_BOOL || !Z_BVAL(is_callable)) {
    ret = FAILURE;
    goto end;
  }
  ZVAL_STRING_COMPAT(&func, function_name);
  ret = call_user_function(object ? NULL : CG(function_table), object ? &object : NULL,
    &func, ret_val, call_params_count, call_params TSRMLS_CC);

end:
  FREE_ZVAL(params[0]);
  zval_dtor(&is_callable);
  zval_dtor(&func);
  return ret != SUCCESS || Z_TYPE_P(ret_val) != ret_type ? -1 : 0;
}
#else
static int _safe_call_function(char *function_name, zval *object, zval *ret_val, int ret_type,
size_t call_params_count, zval call_params[] TSRMLS_DC) {
  int ret;
  zval params[1];
  zval is_callable;
  zval func;

  ZVAL_STRING_COMPAT(&func, "is_callable");
  ZVAL_NULL(&is_callable);
  if (object) {
    array_init(&params[0]);
    add_next_index_zval(&params[0], object);
    add_next_index_string(&params[0], function_name);
  } else {
    ZVAL_STRING_COMPAT(&params[0], function_name);
  }
  ret = call_user_function(CG(function_table), NULL, &func, &is_callable, 1, params TSRMLS_CC);
  if (ret != SUCCESS || Z_TYPE(is_callable) != IS_TRUE) {
    PRINTF_QUANTA("%s::%s is not callable (%d == %d), ztype %d\n",
      object ? get_obj_class_name(object TSRMLS_CC) : NULL,
      function_name, ret, SUCCESS,
      Z_TYPE(is_callable));
    ret = FAILURE;
    goto end;
  }
  ZVAL_STRING_COMPAT(&func, function_name);
  ret = call_user_function(object ? NULL : CG(function_table), object,
    &func, ret_val, call_params_count, call_params TSRMLS_CC);

end:
  zval_dtor(&params[0]);
  zval_dtor(&is_callable);
  zval_dtor(&func);
  if (ret == SUCCESS && Z_TYPE_P(ret_val) != ret_type)
    PRINTF_QUANTA("_safe_call_function: ret is of type %d (expected %d)\n",
      Z_TYPE_P(ret_val), ret_type);
  return ret != SUCCESS || Z_TYPE_P(ret_val) != ret_type ? -1 : 0;
}
#endif


int safe_call_function(char *function, zval *ret, int ret_type,
size_t params_count, zval params[] TSRMLS_DC) {
#if PHP_MAJOR_VERSION < 7
  zval *php5_params[params_count];
  zval *tmp;
  size_t i;
  int retcode;

  for (i = 0; i < params_count; i++) {
    MAKE_STD_ZVAL(php5_params[i]);
    tmp = &params[i];
    MAKE_COPY_ZVAL(&tmp, php5_params[i]);
  }
  retcode = _safe_call_function(function, NULL, ret, ret_type, params_count, php5_params TSRMLS_CC);
  for (i = 0; i < params_count; i++)
    FREE_ZVAL(php5_params[i]);
  return retcode;
#else
  return _safe_call_function(function, NULL, ret, ret_type, params_count, params TSRMLS_CC);
#endif
}

int safe_call_method(zval *object, char *method, zval *ret, int ret_type,
size_t params_count, zval params[] TSRMLS_DC) {
#if PHP_MAJOR_VERSION < 7
  zval *php5_params[params_count];
  zval *tmp;
  size_t i;
  int retcode;

  for (i = 0; i < params_count; i++) {
    MAKE_STD_ZVAL(php5_params[i]);
    tmp = &params[i];
    MAKE_COPY_ZVAL(&tmp, php5_params[i]);
  }
  retcode = _safe_call_function(method, object, ret, ret_type, params_count, php5_params TSRMLS_CC);
  for (i = 0; i < params_count; i++)
    FREE_ZVAL(php5_params[i]);
  return retcode;
#else
  return _safe_call_function(method, object, ret, ret_type, params_count, params TSRMLS_CC);
#endif
}
