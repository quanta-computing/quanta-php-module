#include "quanta_mon.h"

/**
 * Request shutdown callback. Stop profiling and return.
 */
PHP_RSHUTDOWN_FUNCTION(quanta_mon) {
  PRINTF_QUANTA("REQUEST SHUTDOWN();\n");
  hp_end(TSRMLS_C);
  return SUCCESS;
}
