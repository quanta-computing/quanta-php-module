#include "quanta_mon.h"

const char *get_obj_class_name(zval *obj TSRMLS_DC) {
#if PHP_MAJOR_VERSION < 7
  const char *class_name;
  zend_uint class_name_len;

  if (!Z_OBJ_HANDLER_P(obj, get_class_name)
  || Z_OBJ_HANDLER_P(obj, get_class_name)(obj, &class_name, &class_name_len, 0 TSRMLS_CC) != SUCCESS)
    return NULL;
  return class_name;
#else
  zend_string *class_name;

  if (!(class_name = Z_OBJ_HANDLER_P(obj, get_class_name)(Z_OBJ_P(obj))))
    return NULL;
  return ZSTR_VAL(class_name);
#endif
}

zval *get_this(zend_execute_data *execute_data TSRMLS_DC) {
  zval *this = NULL;
  const char *class_name;

#if PHP_MAJOR_VERSION < 7
  if (!execute_data
  || !(this = execute_data->current_this)
  || Z_TYPE_P(this) != IS_OBJECT) {
    PRINTF_QUANTA ("Cannot get this (pun intended) execute_data %p this %p\n", execute_data, this);
    return NULL;
  }
#else
  if (!execute_data || !Z_OBJ(execute_data->This)) {
    PRINTF_QUANTA ("Cannot get this (pun intended) execute_data %p this %p\n", execute_data, this);
    return NULL;
  }
  this = &execute_data->This;
#endif
  if (!(class_name = get_obj_class_name(this TSRMLS_CC))) {
    PRINTF_QUANTA ("GET THIS: 'this' is an object of unknown class\n");
    return NULL;
  }
  PRINTF_QUANTA("This is an object of class %s\n", class_name);
  return this;
}

zval *get_prev_this(zend_execute_data *execute_data TSRMLS_DC) {
  if (!execute_data) {
    PRINTF_QUANTA("Cannot get this: no execute data\n");
    return NULL;
  }
  return get_this(execute_data->prev_execute_data TSRMLS_CC);
}

#if PHP_MAJOR_VERSION < 7
static int _safe_call_function(char *function_name, zval *object, zval *ret_val, int ret_type,
size_t call_params_count, zval **call_params TSRMLS_DC) {
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
  if (object) {
    array_init(&params[0]);
    add_next_index_zval(&params[0], object);
    add_next_index_string(&params[0], function_name);
  } else {
    ZVAL_STRING_COMPAT(&params[0], function_name);
  }
  ret = call_user_function(CG(function_table), NULL, &func, &is_callable, 1, params TSRMLS_CC);
  if (ret != SUCCESS || Z_TYPE(is_callable) != IS_TRUE) {
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
  return ret != SUCCESS || Z_TYPE_P(ret_val) != ret_type ? -1 : 0;
}
#endif


int safe_call_function(char *function, zval *ret, int ret_type,
size_t params_count, zval params[] TSRMLS_DC) {
#if PHP_MAJOR_VERSION < 7
  zval *php5_params[params_count];
  size_t i;

  for (i = 0; i < params_count; i++)
    php5_params[i] = &params[i];
  return _safe_call_function(function, NULL, ret, ret_type, params_count, php5_params TSRMLS_CC);
#else
  return _safe_call_function(function, NULL, ret, ret_type, params_count, params TSRMLS_CC);
#endif
}

int safe_call_method(zval *object, char *method, zval *ret, int ret_type,
size_t params_count, zval params[] TSRMLS_DC) {
#if PHP_MAJOR_VERSION < 7
  zval *php5_params[params_count];
  size_t i;

  for (i = 0; i < params_count; i++)
    php5_params[i] = &params[i];
  return _safe_call_function(method, object, ret, ret_type, params_count, php5_params TSRMLS_CC);
#else
  return _safe_call_function(method, object, ret, ret_type, params_count, params TSRMLS_CC);
#endif
}

zend_class_entry *_zend_fetch_class_compat(char *class, int flags) {
#if PHP_MAJOR_VERSION < 7
  return zend_fetch_class(class, strlen(class), flags);
#else
  zend_class_entry *ce;
  zend_string *class_name = zend_string_init(class, strlen(class), 0);

  if (!class_name)
    return NULL;
  ce = zend_fetch_class(class_name, flags);
  zend_string_release(class_name);
  return ce;
#endif
}

int safe_new(char *class, zval *object, int params_count, zval params[] TSRMLS_DC) {
  zend_class_entry *ce;
  zval dummy;

  ZVAL_NULL(&dummy);
  if (!(ce = _zend_fetch_class_compat(class, ZEND_FETCH_CLASS_SILENT))
  || object_init_ex(object, ce)
  || safe_call_method(object, "__construct", &dummy, IS_NULL, params_count, params TSRMLS_CC)) {
    zval_dtor(&dummy);
    return -1;
  }
  zval_dtor(&dummy);
  return 0;
}

int safe_get_class_constant(char *class, char *name, zval *constant, int type TSRMLS_DC) {
  zval reflection;
  zval reflection_params[1];
  zval get_constant_params[1];
  int ret = 0;

  ZVAL_NULL(constant);
  ZVAL_NULL(&reflection);
  ZVAL_STRING_COMPAT(&reflection_params[0], class);
  ZVAL_STRING_COMPAT(&get_constant_params[0], name);
  if (safe_new("ReflectionClass", &reflection, 1, reflection_params TSRMLS_CC)
  || safe_call_method(&reflection, "getConstant", constant, type, 1, get_constant_params TSRMLS_CC)) {
    PRINTF_QUANTA("Cannot get constant %s::%s\n", class, name);
    ret = -1;
  }
  zval_dtor(&reflection_params[0]);
  zval_dtor(&get_constant_params[0]);
  zval_dtor(&reflection);
  return ret;
}

zval *safe_get_constant(const char *name, int type TSRMLS_DC) {
  zval *constant;

#if PHP_MAJOR_VERSION < 7
  MAKE_STD_ZVAL(constant);
  if (!zend_get_constant(name, strlen(name), constant) || Z_TYPE_P(constant) != type) {
    FREE_ZVAL(constant);
    return NULL;
  }
#else
  if (!(constant = zend_get_constant_str(name, strlen(name))) || Z_TYPE_P(constant) != type) {
    return NULL;
  }
#endif

  return constant;
}

zval *safe_get_argument(zend_execute_data *ex, size_t num, int type) {
  zval *ret;

#if PHP_MAJOR_VERSION < 7
  if (!ex || (int)ex->function_state.arguments[0] < num)
    return NULL;
  ret = (zval *)ex->function_state.arguments[
    -((size_t)(zend_uintptr_t)ex->function_state.arguments[0] - num)];
#else
  if (ZEND_CALL_NUM_ARGS(ex) < num)
    return NULL;
  ret = ZEND_CALL_ARG(ex, num);
#endif
  if (!ret || Z_TYPE_P(ret) != type)
    return NULL;
  return ret;
}
