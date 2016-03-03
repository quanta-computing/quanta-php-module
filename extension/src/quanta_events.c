#include "quanta_mon.h"

static int push_magento_event(uint8_t class, char *type, char *subtype) {
  magento_event_t *event;

  if (!(event = ecalloc(1, sizeof(*event))))
    return -1;
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
  zval *param_node;

  if (!execute_data) {
    PRINTF_QUANTA("extract event: execute_data NULL\n");
    return -1;
  }
  if (!execute_data->prev_execute_data) {
    PRINTF_QUANTA("extract event: execute_data->prev_execute_data NULL\n");
    return -1;
  }
  if ((execute_data->prev_execute_data->function_state.arguments)[0] != (void*)1) {
    PRINTF_QUANTA("extract event: execute_data->prev_execute_data->function_state.arguments is not 1 (%p)\n", \
    (execute_data->prev_execute_data->function_state.arguments)[0]);
    return -1;
  }
  param_node = (zval *)(execute_data->prev_execute_data->function_state.arguments)[-1];
  if (!param_node) {
    PRINTF_QUANTA ("extract event: execute_data->prev_execute_data->function_state.arguments[-1] NULL\n");
    return -1;
  }
  if (Z_TYPE_P(param_node) != IS_STRING) {
    PRINTF_QUANTA("extract event: subtype is not a string\n");
    return -1;
  }
  PRINTF_QUANTA("extract event: subtype: %s\n", Z_STRVAL_P(param_node));
  if (!push_magento_event(MAGENTO_EVENT_CACHE_CLEAR, "clean", Z_STRVAL_P(param_node)))
    return profile_curr;
  else
    return -1;
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
  HashTable *attributes;
  zval *this;
  char *entity;
  char *type;

  if (!execute_data) {
    PRINTF_QUANTA("reindex: execute_data NULL\n");
    return -1;
  }
  if (!execute_data->prev_execute_data) {
    PRINTF_QUANTA("reindex: execute_data->prev_execute_data NULL\n");
    return -1;
  }
  this = execute_data->prev_execute_data->current_this;
  if (Z_TYPE_P(this) != IS_OBJECT) {
    PRINTF_QUANTA("reindex: 'this' is not an object\n");
    return -1;
  }
  attributes = Z_OBJPROP_P(this);
  /* If we want to get a count : array_count = zend_hash_num_elements(arr_hash); */
  entity = get_mage_model_data(attributes, "entity" TSRMLS_CC);
  type = get_mage_model_data(attributes, "type" TSRMLS_CC);
  PRINTF_QUANTA("reindex '%s' on entity '%s'\n", type, entity);
  if (!entity || !type) {
    PRINTF_QUANTA("reindex: Entity or type is NULL\n");
    return -1;
  }
  if (!push_magento_event(MAGENTO_EVENT_REINDEX, type, entity))
    return profile_curr;
  else
    return -1;
}
