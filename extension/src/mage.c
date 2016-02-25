#include "quanta_mon.h"

char *get_mage_model_data(HashTable *attrs, char *key TSRMLS_DC) {
  HashTable *model_data;
  zval **data;
  zval **ret;

  if (zend_hash_find(attrs, "\0*\0_data", 9, (void **)&data) == FAILURE) {
    PRINTF_QUANTA("Cannot fetch model data\n");
    return NULL;
  }
  model_data = Z_ARRVAL_PP(data);
  if (zend_hash_find(model_data, key, strlen(key) + 1, (void **)&ret) == FAILURE) {
    PRINTF_QUANTA("Cannot fetch %s in model data\n", key);
    return NULL;
  }
  if (Z_TYPE_PP(ret) != IS_STRING) {
    PRINTF_QUANTA("%s is not a string :(\n", key);
    return NULL;
  }
  return Z_STRVAL_PP(ret);
}
