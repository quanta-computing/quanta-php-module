#include "quanta_mon.h"

int magento2_record_reindex_event(int profile_curr, zend_execute_data *execute_data TSRMLS_DC) {
  zval *this;
  zval *title;
  zval *title_text;

  if (!(this = get_this(execute_data TSRMLS_CC))
  || !(title = get_mage_model_zdata(Z_OBJPROP_P(this), "title", IS_OBJECT TSRMLS_CC))
  || !(title_text = zend_read_property_compat(Z_OBJCE_P(title), title, "text"))) {
    PRINTF_QUANTA("Cannot get reindex type\n");
    return -1;
  }
  if (qm_record_event(MAGENTO_EVENT_REINDEX, "Reindex", Z_STRVAL_P(title_text))) {
    PRINTF_QUANTA("Cannot push reindex event\n");
    return -1;
  }
  return profile_curr;
}

int magento2_record_cache_clean_event(int profile_curr, zend_execute_data *execute_data TSRMLS_DC) {
  zval *subtype;

  if (!(subtype = safe_get_argument(execute_data, 1, IS_STRING))
  || qm_record_event(MAGENTO_EVENT_CACHE_CLEAR, "clean", Z_STRVAL_P(subtype)))
    return -1;
  else
    return profile_curr;
}
