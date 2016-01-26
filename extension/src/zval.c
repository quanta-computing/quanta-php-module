#include "quanta_mon.h"

/** Look in the PHP assoc array to find a key and return the zval associated
 *  with it.
 *
 *  @author mpal
 **/
zval *hp_zval_at_key(char  *key, zval *values) {
  zval *result = NULL;

  if (values->type == IS_ARRAY) {
    HashTable *ht;
    zval     **value;
    uint       len = strlen(key) + 1;

    ht = Z_ARRVAL_P(values);
    if (zend_hash_find(ht, key, len, (void**)&value) == SUCCESS) {
      result = *value;
    }
  } else {
    result = NULL;
  }

  return result;
}

/** Convert the PHP array of strings to an emalloced array of strings. Note,
 *  this method duplicates the string data in the PHP array.
 *
 *  @author mpal
 **/
char **hp_strings_in_zval(zval  *values) {
  char   **result;
  size_t   count;
  size_t   ix = 0;

  if (!values) {
    return NULL;
  }

  if (values->type == IS_ARRAY) {
    HashTable *ht;

    ht    = Z_ARRVAL_P(values);
    count = zend_hash_num_elements(ht);

    if((result =
         (char**)emalloc(sizeof(char*) * (count + 1))) == NULL) {
      return result;
    }

    for (zend_hash_internal_pointer_reset(ht);
         zend_hash_has_more_elements(ht) == SUCCESS;
         zend_hash_move_forward(ht)) {
      char  *str;
      uint   len;
      ulong  idx;
      int    type;
      zval **data;

      type = zend_hash_get_current_key_ex(ht, &str, &len, &idx, 0, NULL);
      /* Get the names stored in a standard array */
      if(type == HASH_KEY_IS_LONG) {
        if ((zend_hash_get_current_data(ht, (void**)&data) == SUCCESS) &&
            Z_TYPE_PP(data) == IS_STRING &&
            strcmp(Z_STRVAL_PP(data), ROOT_SYMBOL)) { /* do not ignore "main" */
          result[ix] = estrdup(Z_STRVAL_PP(data));
          ix++;
        }
      }
    }
  } else if(values->type == IS_STRING) {
    if((result = (char**)emalloc(sizeof(char*) * 2)) == NULL) {
      return result;
    }
    result[0] = estrdup(Z_STRVAL_P(values));
    ix = 1;
  } else {
    result = NULL;
  }

  /* NULL terminate the array */
  if (result != NULL) {
    result[ix] = NULL;
  }

  return result;
}
