#include "quanta_mon.h"

int magento1_record_reindex_event(int profile_curr, zend_execute_data *execute_data TSRMLS_DC) {
  HashTable *attributes;
  zval *this;
  char *entity;
  char *type;

  if (!(this = get_prev_this(execute_data TSRMLS_CC)))
    return -1;
  attributes = Z_OBJPROP_P(this);
  /* If we want to get a count : array_count = zend_hash_num_elements(arr_hash); */
  entity = get_mage_model_data(attributes, "entity" TSRMLS_CC);
  type = get_mage_model_data(attributes, "type" TSRMLS_CC);
  if (!entity || !type) {
    PRINTF_QUANTA("reindex: Entity or type is NULL\n");
    return -1;
  }
  if (!qm_record_event(MAGENTO_EVENT_REINDEX, type, entity))
    return profile_curr;
  else
    return -1;
}
