#include "quanta_mon.h"

int magento_record_cache_flush_event(int profile_curr, zend_execute_data *execute_data TSRMLS_DC) {
  (void)execute_data;
  if (!qm_record_event(MAGENTO_EVENT_CACHE_CLEAR, "flush", "all"))
    return profile_curr;
  else
    return -1;
}

int magento_record_cache_system_flush_event(int profile_curr, zend_execute_data *execute_data TSRMLS_DC) {
  (void)execute_data;
  if (!qm_record_event(MAGENTO_EVENT_CACHE_CLEAR, "flush", "system"))
    return profile_curr;
  else
    return -1;
}
