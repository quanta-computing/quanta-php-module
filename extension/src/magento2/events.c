#include "quanta_mon.h"

int magento2_record_reindex_event(profiled_application_t *app, profiled_function_t *function,
zend_execute_data *execute_data TSRMLS_DC) {
  zval *this;
  zval *title;
  zval *title_text;

  (void)app;
  (void)function;
  if (!(this = get_this(execute_data TSRMLS_CC))
  || !(title = get_mage_model_zdata(Z_OBJPROP_P(this), "title", IS_OBJECT TSRMLS_CC))
  || !(title_text = zend_read_property_compat(Z_OBJCE_P(title), title, "text"))) {
    PRINTF_QUANTA("Cannot get reindex type\n");
    return -1;
  }
  if (qm_record_event(APP_EV_REINDEX, "Reindex", Z_STRVAL_P(title_text))) {
    PRINTF_QUANTA("Cannot push reindex event\n");
    return -1;
  }
  return 0;
}

int magento2_record_cache_clean_event(profiled_application_t *app, profiled_function_t *function,
zend_execute_data *execute_data TSRMLS_DC) {
  zval *subtype;

  (void)app;
  (void)function;
  if (!(subtype = safe_get_argument(execute_data, 1, IS_STRING))
  || qm_record_event(APP_EV_CACHE_CLEAR, "clean", Z_STRVAL_P(subtype)))
    return -1;
  else
    return 0;
}
