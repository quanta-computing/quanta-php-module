#include "quanta_mon.h"

/**
 * A hash function to calculate a 8-bit hash code for a function name.
 * This is based on a small modification to 'zend_inline_hash_func' by summing
 * up all bytes of the ulong returned by 'zend_inline_hash_func'.
 *
 * @param str, char *, string to be calculated hash code for.
 *
 * @author cjiang
 */
uint8_t hp_inline_hash(const char *str) {
  ulong h = 5381;
  uint i = 0;
  uint8_t res = 0;

  while (*str)
    h = (h + (h << 5)) ^ (ulong)*str++;

  for (i = 0; i < sizeof(ulong); i++)
    res += ((uint8_t *)&h)[i];
  return res;
}
