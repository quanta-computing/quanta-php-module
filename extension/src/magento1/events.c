#include "quanta_mon.h"

int magento1_record_reindex_event(profiled_application_t *app, profiled_function_t *function,
zend_execute_data *execute_data TSRMLS_DC) {
  zval *this;
  char *entity;

  if (!(this = get_this(execute_data TSRMLS_CC))
  || !(entity = get_mage_model_data(Z_OBJPROP_P(this), "indexer_code" TSRMLS_CC))
  || qm_record_event(APP_EV_REINDEX, "reindex", entity)) {
    PRINTF_QUANTA("Cannot record reindex event\n");
    return -1;
  }
  return 0;
}

int magento1_record_process_index_event(profiled_application_t *app, profiled_function_t *function,
zend_execute_data *execute_data TSRMLS_DC) {
  zval *event;
  char *type;
  char *entity;

  if (!(event = safe_get_argument(execute_data, 1, IS_OBJECT))
  || !(type = get_mage_model_data(Z_OBJPROP_P(event), "entity" TSRMLS_CC))
  || !(entity = get_mage_model_data(Z_OBJPROP_P(event), "type" TSRMLS_CC))
  || qm_record_event(APP_EV_REINDEX, type, entity)) {
    PRINTF_QUANTA("Cannot record process index event\n");
    return -1;
  }
  return 0;
}

int magento1_record_cache_clean_event(profiled_application_t *app, profiled_function_t *function,
zend_execute_data *execute_data TSRMLS_DC) {
  zval *subtype;

  (void)app;
  (void)function;
  if (!(subtype = safe_get_argument(execute_data, 1, IS_STRING))
  || qm_record_event(APP_EV_CACHE_CLEAR, "clean", Z_STRVAL_P(subtype))) {
    PRINTF_QUANTA("Cannot record cache_clean event\n");
    return -1;
  }
  return 0;
}
