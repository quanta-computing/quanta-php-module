#include "quanta_mon.h"

/**
 * Increment the count of the given stat with the given count
 * If the stat was not set before, inits the stat to the given count
 *
 * @param  zval *counts   Zend hash table pointer
 * @param  char *name     Name of the stat
 * @param  long  count    Value of the stat to incr by
 * @return void
 * @author kannan
 */
void hp_inc_count(zval *counts, char *name, long count) {
  HashTable *ht;
  zval *data;

  if (!counts) return;
  ht = HASH_OF(counts);
  if (!ht) return;

  if ((data = zend_hash_str_find(ht, name, strlen(name)))) {
    ZVAL_LONG(data, Z_LVAL_P(data) + count);
  } else {
    add_assoc_long(counts, name, count);
  }
}

/**
 * Returns formatted function name
 *
 * @param  entry        hp_entry
 * @param  result_buf   ptr to result buf
 * @param  result_len   max size of result buf
 * @return total size of the function name returned in result_buf
 * @author veeve
 */
size_t hp_get_entry_name(hp_entry_t  *entry, char *result_buf, size_t result_len) {

  /* Validate result_len */
  if (result_len <= 1) {
    /* Insufficient result_bug. Bail! */
    return 0;
  }

  /* Add '@recurse_level' if required */
  /* NOTE:  Dont use snprintf's return val as it is compiler dependent */
  if (entry->rlvl_hprof) {
    snprintf(result_buf, result_len, "%s@%d", entry->name_hprof, entry->rlvl_hprof);
  }
  else {
    snprintf(result_buf, result_len, "%s", entry->name_hprof);
  }

  /* Force null-termination at MAX */
  result_buf[result_len - 1] = 0;

  return strlen(result_buf);
}
