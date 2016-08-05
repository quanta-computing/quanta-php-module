#include "quanta_mon.h"

int hp_begin_profiling(hp_entry_t **entries, const char *symbol, zend_execute_data *data TSRMLS_DC) {
  int profile_curr;

  profile_curr = qm_begin_profiling(symbol, data TSRMLS_CC);
  if (hp_globals.profiler_level == QUANTA_MON_MODE_HIERARCHICAL) {
    hp_entry_t *cur_entry = hp_fast_alloc_hprof_entry();
    if (!cur_entry)
      return profile_curr;
    cur_entry->hash_code = hp_inline_hash(symbol);
    cur_entry->name_hprof = symbol;
    cur_entry->prev_hprof = *entries;
    hp_hier_begin_profiling(entries, cur_entry TSRMLS_CC);
    /* Update entries linked list */
    *entries = cur_entry;
  }
  return profile_curr;
}
