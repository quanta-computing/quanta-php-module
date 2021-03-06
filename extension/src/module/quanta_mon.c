#include "quanta_mon.h"

/**
 * *********************
 * PHP EXTENSION GLOBALS
 * *********************
 */

/* Callback functions for the quanta_mon extension */
zend_module_entry quanta_mon_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
  STANDARD_MODULE_HEADER,
#endif
  "quanta_mon",                        /* Name of the extension */
  NULL,                                /* List of functions exposed */
  PHP_MINIT(quanta_mon),               /* Module init callback */
  PHP_MSHUTDOWN(quanta_mon),           /* Module shutdown callback */
  PHP_RINIT(quanta_mon),               /* Request init callback */
  PHP_RSHUTDOWN(quanta_mon),           /* Request shutdown callback */
  PHP_MINFO(quanta_mon),               /* Module info callback */
#if ZEND_MODULE_API_NO >= 20010901
  QUANTA_MON_VERSION,
#endif
  STANDARD_MODULE_PROPERTIES
};


/* Init module */
ZEND_GET_MODULE(quanta_mon)


/**
 * ***********************
 * GLOBAL STATIC VARIABLES
 * ***********************
 */
/* QuantaMon global state */
hp_global_t       hp_globals;
