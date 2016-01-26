#include "quanta_mon.h"


/**
 * Module shutdown callback.
 */
PHP_MSHUTDOWN_FUNCTION(quanta_mon) {
  /* Make sure cpu_frequencies is free'ed. */
  clear_frequencies();

  /* free any remaining items in the free list */
  hp_free_the_free_list();

  UNREGISTER_INI_ENTRIES();

  return SUCCESS;
}
