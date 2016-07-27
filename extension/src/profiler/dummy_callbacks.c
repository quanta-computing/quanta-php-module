#include "quanta_mon.h"

void hp_mode_dummy_init_cb(TSRMLS_D) { }

void hp_mode_dummy_exit_cb(TSRMLS_D) { }

void hp_mode_dummy_beginfn_cb(hp_entry_t **entries, hp_entry_t *current  TSRMLS_DC) {
  (void)entries;
  (void)current;
}

void hp_mode_dummy_endfn_cb(hp_entry_t **entries   TSRMLS_DC) {
  (void)entries;
}
