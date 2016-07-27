#include "quanta_mon.h"

/**
 * Free any items in the free list.
 */
void hp_free_the_free_list() {
  hp_entry_t *p = hp_globals.entry_free_list;
  hp_entry_t *cur;

  while (p) {
    cur = p;
    p = p->prev_hprof;
    free(cur);
  }
}

/**
 * Fast allocate a hp_entry_t structure. Picks one from the
 * free list if available, else does an actual allocate.
 *
 * Doesn't bother initializing allocated memory.
 *
 * @author kannan
 */
hp_entry_t *hp_fast_alloc_hprof_entry() {
  hp_entry_t *p;

  p = hp_globals.entry_free_list;

  if (p) {
    hp_globals.entry_free_list = p->prev_hprof;
    return p;
  } else {
    return (hp_entry_t *)malloc(sizeof(hp_entry_t));
  }
}

/**
 * Fast free a hp_entry_t structure. Simply returns back
 * the hp_entry_t to a free list and doesn't actually
 * perform the free.
 *
 * @author kannan
 */
void hp_fast_free_hprof_entry(hp_entry_t *p) {

  /* we use/overload the prev_hprof field in the structure to link entries in
   * the free list. */
  p->prev_hprof = hp_globals.entry_free_list;
  hp_globals.entry_free_list = p;
}
