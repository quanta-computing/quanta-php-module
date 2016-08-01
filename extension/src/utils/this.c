#include "quanta_mon.h"

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
  return this;
}

zval *get_prev_this(zend_execute_data *execute_data TSRMLS_DC) {
  if (!execute_data) {
    PRINTF_QUANTA("Cannot get this: no execute data\n");
    return NULL;
  }
  return get_this(execute_data->prev_execute_data TSRMLS_CC);
}
