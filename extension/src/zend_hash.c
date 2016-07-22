#include "quanta_mon.h"

zval *zend_hash_find_compat(HashTable *ht, const char *key, size_t key_len) {
#if PHP_MAJOR_VERSION < 7
  zval **data;

  if (zend_hash_find(ht, key, key_len, (void **)&data) == FAILURE) {
    return NULL;
  }
  return *data;
#else
  return zend_hash_str_find(ht, key, key_len);
#endif
}

zend_bool zend_hash_exists_compat(HashTable *ht, const char *key, size_t key_len) {
#if PHP_MAJOR_VERSION < 7
  return zend_hash_exists(ht, key, key_len);
#else
  return zend_hash_str_exists(ht, key, key_len);
#endif
}
