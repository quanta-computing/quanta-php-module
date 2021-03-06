#include "quanta_mon.h"

zval *zend_read_property_compat(zend_class_entry *ce, zval *obj, const char *name) {
#if PHP_MAJOR_VERSION < 7
  return zend_read_property(ce, obj, name, strlen(name), 1);
#else
  return zend_read_property(ce, obj, name, strlen(name), 1, NULL);
#endif
}
