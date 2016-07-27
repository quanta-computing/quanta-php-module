#include "quanta_mon.h"

zval *safe_get_argument(zend_execute_data *ex, size_t num, int type) {
  zval *ret;

  if (!ex)
    return NULL;
#if PHP_MAJOR_VERSION < 7
# if PHP_VERSION_ID >= 50500
  if (!ex->function_state.arguments)
    ex = ex->prev_execute_data;
# endif
  if (!ex || (size_t)(zend_uintptr_t)ex->function_state.arguments[0] < num)
    return NULL;
  ret = (zval *)ex->function_state.arguments[
    -((size_t)(zend_uintptr_t)ex->function_state.arguments[0] - (num - 1))];
#else
  if (ZEND_CALL_NUM_ARGS(ex) < num) {
    PRINTF_QUANTA("Method does not take so much arguments (it takes %d)\n", ZEND_CALL_NUM_ARGS(ex));
    return NULL;
  }
  ret = ZEND_CALL_ARG(ex, num);
#endif
  PRINTF_QUANTA("arg %zu is a %d (expected %d)\n", num, ret ? Z_TYPE_P(ret) : 0, type);
  if (!ret || Z_TYPE_P(ret) != type)
    return NULL;
  return ret;
}
