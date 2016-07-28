#include "quanta_mon.h"


int magento1_record_reindex_event(profiled_application_t *app,
zend_execute_data *execute_data TSRMLS_DC) {
  HashTable *attributes;
  zval *this;
  char *entity;
  char *type;

  if (!(this = get_prev_this(execute_data TSRMLS_CC)))
    return -1;
  attributes = Z_OBJPROP_P(this);
  entity = get_mage_model_data(attributes, "entity" TSRMLS_CC);
  type = get_mage_model_data(attributes, "type" TSRMLS_CC);
  if (!entity || !type) {
    PRINTF_QUANTA("reindex: Entity or type is NULL\n");
    return -1;
  }
  if (!qm_record_event(APP_EV_REINDEX, type, entity))
    return 0;
  else
    return -1;
}
