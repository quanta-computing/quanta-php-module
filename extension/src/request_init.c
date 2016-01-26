#include "quanta_mon.h"


/**
 * Request init callback.
 */
PHP_RINIT_FUNCTION(quanta_mon) {
// xxx VERIFIER qu'on puisse bien faire un hp_begin avec des param differents
// a chaque appel
// puis remplir hp_mode_evnets_only_beginfn_cb
  int mode;
  long  quanta_mon_flags = 0;                                    /* QuantaMon flags */
  zval *optional_array = NULL;         /* optional array arg: for future use */
  char *cookie_data = SG(request_info).cookie_data;

  if (cookie_data && strstr(cookie_data, hp_globals.full_profiling_cookie_trigger))
    mode = QUANTA_MON_MODE_HIERARCHICAL;
  else if (cookie_data && strstr(cookie_data, hp_globals.magento_profiling_cookie_trigger))
    mode = QUANTA_MON_MODE_MAGENTO_PROFILING;
  else
    mode = QUANTA_MON_MODE_EVENTS_ONLY;

  hp_get_ignored_functions_from_arg(optional_array);
  hp_get_monitored_functions_fill();

  hp_begin(mode, quanta_mon_flags TSRMLS_CC);

  return SUCCESS;
}
