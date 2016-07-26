#include "quanta_mon.h"

static int push_magento_event(uint8_t class, char *type, char *subtype) {
  magento_event_t *event;

  if (!(event = ecalloc(1, sizeof(*event))))
    return -1;
  PRINTF_QUANTA("EVENT[%hhd]: %s %s\n", class, type, subtype);
  event->class = class;
  if (!(event->type = estrdup(type))
  || !(event->subtype = estrdup(subtype))) {
    free(event->type);
    free(event);
    return -1;
  }
  event->prev = hp_globals.magento_events;
  hp_globals.magento_events = event;
  return 0;
}

int qm_record_cache_clean_event(int profile_curr, zend_execute_data *execute_data TSRMLS_DC) {
  zval *subtype;

  if (!(subtype = safe_get_argument(execute_data, 1, IS_STRING))
  || push_magento_event(MAGENTO_EVENT_CACHE_CLEAR, "clean", Z_STRVAL_P(subtype)))
    return -1;
  else
    return profile_curr;
}

int qm_record_cache_flush_event(int profile_curr, zend_execute_data *execute_data TSRMLS_DC) {
  (void)execute_data;
  if (!push_magento_event(MAGENTO_EVENT_CACHE_CLEAR, "flush", "all"))
    return profile_curr;
  else
    return -1;
}

int qm_record_cache_system_flush_event(int profile_curr, zend_execute_data *execute_data TSRMLS_DC) {
  (void)execute_data;
  if (!push_magento_event(MAGENTO_EVENT_CACHE_CLEAR, "flush", "system"))
    return profile_curr;
  else
    return -1;
}


int qm_record_reindex_event(int profile_curr, zend_execute_data *execute_data TSRMLS_DC) {
  zval *this;
  zval *title;
  zval *title_text;

  if (!(this = get_this(execute_data TSRMLS_CC))
  || !(title = get_mage_model_zdata(Z_OBJPROP_P(this), "title", IS_OBJECT TSRMLS_CC))
  || !(title_text = zend_read_property_compat(Z_OBJCE_P(title), title, "text"))) {
    PRINTF_QUANTA("Cannot get reindex type\n");
    return -1;
  }
  if (push_magento_event(MAGENTO_EVENT_REINDEX, "Reindex", Z_STRVAL_P(title_text))) {
    PRINTF_QUANTA("Cannot push reindex event\n");
    return -1;
  }
  return profile_curr;
}

int qm_record_reindex_event1(int profile_curr, zend_execute_data *execute_data TSRMLS_DC) {
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
  if (!push_magento_event(MAGENTO_EVENT_REINDEX, type, entity))
    return profile_curr;
  else
    return -1;
}
