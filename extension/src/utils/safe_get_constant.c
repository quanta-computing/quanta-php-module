#include "quanta_mon.h"

zval *safe_get_constant(const char *name, int type) {
  zval *constant;

  if (!(constant = zend_get_constant_str(name, strlen(name))) || Z_TYPE_P(constant) != type) {
    return NULL;
  }
  return constant;
}
