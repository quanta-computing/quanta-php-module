#include "quanta_mon.h"

int magento_record_cache_flush_event(profiled_application_t *app,
zend_execute_data *execute_data TSRMLS_DC) {
  (void)app;
  (void)execute_data;
  if (!qm_record_event(APP_EV_CACHE_CLEAR, "flush", "all"))
    return 0;
  else
    return -1;
}

int magento_record_cache_system_flush_event(profiled_application_t *app, zend_execute_data *execute_data TSRMLS_DC) {
  (void)execute_data;
  if (!qm_record_event(APP_EV_CACHE_CLEAR, "flush", "system"))
    return 0;
  else
    return -1;
}
