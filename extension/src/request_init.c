#include "quanta_mon.h"


/* We get needed informations in http header QUANTA_HTTP_HEADER:
** Which should contains the step id and the (optional) profiling mode which defaults to magento
** profiling only
** If an error occurs, we assume QUANTA_MON_EVENTS_ONLY profiling mode
**
** @returns: the profiled mode
*/
static int extract_headers_info(TSRMLS_D) {
  HashTable *_SERVER;
  zval **arr;
  zval **data;
  char *quanta_header;
  char *quanta_mode;

  /* _SERVER is lazy-initialized, force population
  */
  if (!zend_hash_exists(&EG(symbol_table), "_SERVER", 8)) {
    zend_auto_global* auto_global;
    if (zend_hash_find(CG(auto_globals), "_SERVER", 8, (void **)&auto_global) != FAILURE) {
      auto_global->armed = auto_global->auto_global_callback(auto_global->name,
        auto_global->name_len TSRMLS_CC);
    }
  }
  if (zend_hash_find(&EG(symbol_table), "_SERVER", 8, (void**)&arr) == FAILURE) {
    PRINTF_QUANTA("NO _SERVER\n");
    return QUANTA_MON_MODE_EVENTS_ONLY;
  }
  _SERVER = Z_ARRVAL_P(*arr);
  if (zend_hash_find(_SERVER, QUANTA_HTTP_HEADER, strlen(QUANTA_HTTP_HEADER) + 1, (void **)&data) == FAILURE
  || Z_TYPE_PP(data) != IS_STRING) {
    PRINTF_QUANTA("NO QUANTA HEADER\n");
    return QUANTA_MON_MODE_EVENTS_ONLY;
  }
  quanta_header = Z_STRVAL_PP(data);
  PRINTF_QUANTA("QUANTA HEADER: %s\n", quanta_header);
  hp_globals.quanta_step_id = strtoull(quanta_header, &quanta_mode, 10);
  if (quanta_mode == quanta_header) {
    PRINTF_QUANTA("WRONG FORMAT %s for step_id\n", quanta_header);
    return QUANTA_MON_MODE_EVENTS_ONLY;
  }
  while (*quanta_mode == ' ')
    quanta_mode++;
  PRINTF_QUANTA("QUANTA STEP_ID %zu, mode %s\n", hp_globals.quanta_step_id, quanta_mode);
  if (!*quanta_mode || !strcmp(quanta_mode, QUANTA_HTTP_HEADER_MODE_MAGE))
    return QUANTA_MON_MODE_MAGENTO_PROFILING;
  else if (!strcmp(quanta_mode, QUANTA_HTTP_HEADER_MODE_FULL))
    return QUANTA_MON_MODE_HIERARCHICAL;
  else
    return QUANTA_MON_MODE_EVENTS_ONLY;
}

/**
 * Request init callback.
 */
PHP_RINIT_FUNCTION(quanta_mon) {
// xxx VERIFIER qu'on puisse bien faire un hp_begin avec des param differents
// a chaque appel
// puis remplir hp_mode_evnets_only_beginfn_cb
  int mode;

  mode = extract_headers_info(TSRMLS_C);
  // hp_get_ignored_functions_from_arg(optional_array);
  hp_get_monitored_functions_fill();

  hp_begin(mode, 0 TSRMLS_CC);

  return SUCCESS;
}
