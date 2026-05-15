#include "quanta_mon.h"

zval *safe_get_argument(zend_execute_data *ex, size_t num, int type) {
  zval *ret;

  if (!ex)
    return NULL;
  if (ZEND_CALL_NUM_ARGS(ex) < num) {
    return NULL;
  }
  ret = ZEND_CALL_ARG(ex, num);
  if (!ret || Z_TYPE_P(ret) != type)
    return NULL;
  return ret;
}
