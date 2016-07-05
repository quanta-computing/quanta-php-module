#include "quanta_mon.h"

zval *get_this(zend_execute_data *execute_data TSRMLS_DC) {
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

static int _safe_call_function(char *function_name, zval *object, zval *ret_val, int ret_type,
size_t call_params_count, zval **call_params TSRMLS_DC) {
  int ret;
  zval *params[1];
  zval is_callable;
  zval func;

  MAKE_STD_ZVAL(params[0]);
  ZVAL_STRING(&func, "is_callable", 1);
  if (object) {
    array_init(params[0]);
    add_next_index_zval(params[0], object);
    add_next_index_string(params[0], function_name, 1);
  } else {
    ZVAL_STRING(params[0], function_name, 1);
  }
  ret = call_user_function(CG(function_table), NULL, &func, &is_callable, 1, params TSRMLS_CC);
  if (ret != SUCCESS || Z_TYPE(is_callable) != IS_BOOL || !Z_BVAL(is_callable)) {
    ret = FAILURE;
    goto end;
  }
  ZVAL_STRING(&func, function_name, 1);
  ret = call_user_function(object ? NULL : CG(function_table), object ? &object : NULL,
    &func, ret_val, call_params_count, call_params TSRMLS_CC);

end:
  FREE_ZVAL(params[0]);
  zval_dtor(&is_callable);
  zval_dtor(&func);
  return ret != SUCCESS || Z_TYPE_P(ret_val) != ret_type ? -1 : 0;
}

int safe_call_function(char *function, zval *ret, int ret_type,
size_t params_count, zval **params TSRMLS_DC) {
  return _safe_call_function(function, NULL, ret, ret_type, params_count, params TSRMLS_CC);
}

int safe_call_method(zval *object, char *method, zval *ret, int ret_type,
size_t params_count, zval **params TSRMLS_DC) {
  return _safe_call_function(method, object, ret, ret_type, params_count, params TSRMLS_CC);
}

zval *safe_new(char *class, int params_count, zval **params TSRMLS_DC) {
  zend_class_entry *ce;
  zval *object;
  zval dummy;

  ZVAL_NULL(&dummy);
  MAKE_STD_ZVAL(object);
  if (!(ce = zend_fetch_class(class, strlen(class), ZEND_FETCH_CLASS_SILENT))
  || object_init_ex(object, ce)
  || safe_call_method(object, "__construct", &dummy, IS_NULL, params_count, params TSRMLS_CC)) {
    zval_dtor(&dummy);
    FREE_ZVAL(object);
    return NULL;
  }
  zval_dtor(&dummy);
  return object;
}

zval *safe_get_class_constant(char *class, char *name, int type TSRMLS_DC) {
  zval *reflection;
  zval *reflection_params[1];
  zval *get_constant_params[1];
  zval *constant;

  MAKE_STD_ZVAL(constant);
  MAKE_STD_ZVAL(reflection_params[0]);
  MAKE_STD_ZVAL(get_constant_params[0]);
  ZVAL_STRING(reflection_params[0], class, 1);
  ZVAL_STRING(get_constant_params[0], name, 1);
  if (!(reflection = safe_new("ReflectionClass", 1, reflection_params TSRMLS_CC))
  || safe_call_method(reflection, "getConstant", constant, type, 1, get_constant_params TSRMLS_CC)) {
    PRINTF_QUANTA("Cannot get constant %s::%s\n", class, name);
    FREE_ZVAL(constant);
    constant = NULL;
    goto end;
  }

end:
  FREE_ZVAL(reflection_params[0]);
  FREE_ZVAL(get_constant_params[0]);
  if (reflection)
    FREE_ZVAL(reflection);
  return constant;
}
