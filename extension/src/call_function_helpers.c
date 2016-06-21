#include "quanta_mon.h"


static int _safe_call_function(char *function_name, zval *object, zval *ret_val, int ret_type,
size_t call_params_count, zval **call_params TSRMLS_DC) {
  int ret;
  zval *params[1];
  zval is_callable;
  zval func;

  INIT_ZVAL(func);
  MAKE_STD_ZVAL(params[0]);
  ZVAL_NULL(&is_callable);
  ZVAL_STRING(&func, "is_callable", 1);
  if (object) {
    array_init(params[0]);
    add_next_index_zval(params[0], object);
    add_next_index_string(params[0], function_name, 1);
  } else {
    ZVAL_STRING(params[0], function_name, 1);
  }
  ret = call_user_function(CG(function_table), NULL, &func, &is_callable, 1, params TSRMLS_CC);
  FREE_ZVAL(params[0]);
  if (ret != SUCCESS || Z_TYPE(is_callable) != IS_BOOL || !Z_BVAL(is_callable))
    return -1;
  ZVAL_STRING(&func, function_name, 1);
  ret = call_user_function(object ? NULL : CG(function_table), object ? &object : NULL,
    &func, ret_val, call_params_count, call_params TSRMLS_CC);
  zval_dtor(&func);
  // TODO!
  // return ret != SUCCESS || Z_TYPE_P(ret_val) != ret_type ? -1 : 0;
  return ret != SUCCESS ? -1 : 0;
}

int safe_call_function(char *function, zval *ret, int ret_type,
size_t params_count, zval **params TSRMLS_DC) {
  return _safe_call_function(function, NULL, ret, ret_type, params_count, params TSRMLS_CC);
}

int safe_call_method(zval *object, char *method, zval *ret, int ret_type,
size_t params_count, zval **params TSRMLS_DC) {
  return _safe_call_function(method, object, ret, ret_type, params_count, params TSRMLS_CC);
}
