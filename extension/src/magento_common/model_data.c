#include "quanta_mon.h"

zval *get_mage_model_zdata(HashTable *attrs, char *key, int type TSRMLS_DC) {
  zval *data;
  zval *ret;

  if (!(data = zend_hash_find_compat(attrs, "\0*\0_data", sizeof("\0*\0_data") - 1))) {
    PRINTF_QUANTA("Cannot fetch model data\n");
    return NULL;
  }
#if PHP_MAJOR_VERSION >= 7
  if (Z_TYPE_P(data) == IS_INDIRECT)
    data = Z_INDIRECT_P(data);
#endif
  if (Z_TYPE_P(data) != IS_ARRAY) {
    PRINTF_QUANTA("_data is not an array\n");
    return NULL;
  }
  // TODO! Segfault
  if (!(ret = zend_hash_find_compat(Z_ARRVAL_P(data), key, strlen(key)))) {
    PRINTF_QUANTA("Cannot fetch %s in model data\n", key);
    return NULL;
  }
  if (Z_TYPE_P(ret) != type) {
    PRINTF_QUANTA("%s is not a %d, it's a %d :(\n", key, type, Z_TYPE_P(ret));
    return NULL;
  }
  return ret;
}

char *get_mage_model_data(HashTable *attrs, char *key TSRMLS_DC) {
  zval *data;

  if (!(data = get_mage_model_zdata(attrs, key, IS_STRING TSRMLS_CC)))
    return NULL;
  return Z_STRVAL_P(data);
}
