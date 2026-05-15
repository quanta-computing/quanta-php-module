#include "quanta_mon.h"

zval *get_this(zend_execute_data *execute_data) {
  zval *this = NULL;
  const char *class_name;

  if (!execute_data || !Z_OBJ(execute_data->This)) {
    PRINTF_QUANTA ("Cannot get this (pun intended) execute_data %p this %p\n", execute_data, this);
    return NULL;
  }
  this = &execute_data->This;
  if (!(class_name = get_obj_class_name(this))) {
    PRINTF_QUANTA ("GET THIS: 'this' is an object of unknown class\n");
    return NULL;
  }
  return this;
}

zval *get_prev_this(zend_execute_data *execute_data) {
  if (!execute_data) {
    PRINTF_QUANTA("Cannot get this: no execute data\n");
    return NULL;
  }
  return get_this(execute_data->prev_execute_data);
}
