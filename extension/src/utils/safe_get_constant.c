#include "quanta_mon.h"

zval *safe_get_constant(const char *name, int type) {
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
