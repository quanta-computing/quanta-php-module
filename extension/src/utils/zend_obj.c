#include "quanta_mon.h"

const char *get_obj_class_name(zval *obj) {
#if PHP_MAJOR_VERSION < 7
  const char *class_name;
  zend_uint class_name_len;

  if (!Z_OBJ_HANDLER_P(obj, get_class_name)
  || Z_OBJ_HANDLER_P(obj, get_class_name)(obj, &class_name, &class_name_len, 0) != SUCCESS)
    return NULL;
  return class_name;
#else
  zend_string *class_name;

  if (!(class_name = Z_OBJ_HANDLER_P(obj, get_class_name)(Z_OBJ_P(obj))))
    return NULL;
  return ZSTR_VAL(class_name);
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

int safe_new(char *class, zval *object, int params_count, zval params[]) {
  zend_class_entry *ce;
  zval dummy;

  ZVAL_NULL(&dummy);
  if (!(ce = _zend_fetch_class_compat(class, ZEND_FETCH_CLASS_SILENT))
  || object_init_ex(object, ce)
  || safe_call_method(object, "__construct", &dummy, IS_NULL, params_count, params)) {
    zval_dtor(&dummy);
    return -1;
  }
  zval_dtor(&dummy);
  return 0;
}

int safe_get_class_constant(char *class, char *name, zval *retval, int type) {
  zend_class_entry *ce;
  zval *constant;

  if (!(ce = _zend_fetch_class_compat(class, ZEND_FETCH_CLASS_SILENT))) {
    PRINTF_QUANTA("Cannot find zend_class_entry for class %s\n", class);
    return -1;
  }
  if (!(constant = zend_hash_find_compat(&ce->constants_table, name, strlen(name)))) {
    PRINTF_QUANTA("Cannot find constant %s::%s\n", class, name);
    return -1;
  }
  if (Z_TYPE_P(constant) != type) {
    PRINTF_QUANTA("Constant %s::%s is not of type %d (it's a %d)\n",
      class, name, type, Z_TYPE_P(constant));
    return -1;
  }
#if PHP_MAJOR_VERSION < 7
  MAKE_COPY_ZVAL(&constant, retval)
#else
  ZVAL_DUP(retval, constant);
#endif
  return 0;
}
