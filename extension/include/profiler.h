#ifndef QUANTA_MON_PROFILER_H_
#define QUANTA_MON_PROFILER_H_

/*
 * Start profiling - called just before calling the actual function
 * NOTE:  PLEASE MAKE SURE TSRMLS_CC IS AVAILABLE IN THE CONTEXT
 *        OF THE FUNCTION WHERE THIS MACRO IS CALLED.
 *        TSRMLS_CC CAN BE MADE AVAILABLE VIA TSRMLS_DC IN THE
 *        CALLING FUNCTION OR BY CALLING TSRMLS_FETCH()
 *        TSRMLS_FETCH() IS RELATIVELY EXPENSIVE.
 *
 * return profile_curr with
 *                          -1 if this function is not specifically profiled
 *                          >=0 this function is specifically profiled, in this case contain the array position
 */
#define BEGIN_PROFILING(entries, symbol, profile_curr, pathname, execute_data)        \
  do {                                                                                \
    /* Use a hash code to filter most of the string comparisons. */                   \
    uint8 hash_code  = hp_inline_hash(symbol);                                        \
    if(hp_ignore_entry(hash_code, symbol))                                            \
	 profile_curr = -1;                                                           \
    else                                                                              \
         profile_curr = qm_record_timers_loading_time(hash_code, symbol, execute_data);    \
    if (hp_globals.profiler_level <= QUANTA_MON_MODE_SAMPLED) {                       \
      hp_entry_t *cur_entry = hp_fast_alloc_hprof_entry();                            \
      (cur_entry)->hash_code = hash_code;                                             \
      (cur_entry)->name_hprof = symbol;                                               \
      (cur_entry)->pathname_hprof = pathname;                                         \
      (cur_entry)->prev_hprof = (*(entries));                                         \
      /* Call the universal callback */                                               \
      hp_mode_common_beginfn((entries), (cur_entry) TSRMLS_CC);                       \
      /* Call the mode's beginfn callback */                                          \
      hp_globals.mode_cb.begin_fn_cb((entries), (cur_entry) TSRMLS_CC);               \
      /* Update entries linked list */                                                \
      (*(entries)) = (cur_entry);                                                     \
    }                                                                                 \
  } while (0)

/*
 * Stop profiling - called just after calling the actual function
 * NOTE:  PLEASE MAKE SURE TSRMLS_CC IS AVAILABLE IN THE CONTEXT
 *        OF THE FUNCTION WHERE THIS MACRO IS CALLED.
 *        TSRMLS_CC CAN BE MADE AVAILABLE VIA TSRMLS_DC IN THE
 *        CALLING FUNCTION OR BY CALLING TSRMLS_FETCH()
 *        TSRMLS_FETCH() IS RELATIVELY EXPENSIVE.
 */
#define END_PROFILING(entries, profile_curr, execute_data)                      \
  do {                                                                          \
    if (profile_curr >= 0) {                                                    \
         hp_globals.monitored_function_tsc_stop[profile_curr] = cycle_timer();  \
         if (profile_curr == POS_ENTRY_GENERATEBLOCK)                           \
             hp_globals.monitored_function_generate_renderize_block_last_linked_list->tsc_generate_stop = hp_globals.monitored_function_tsc_stop[profile_curr]; \
	else if (profile_curr == POS_ENTRY_AFTERTOHTML)                      \
             qm_after_tohmtl(execute_data);                                  \
    }                                                                        \
    if (hp_globals.profiler_level <= QUANTA_MON_MODE_SAMPLED) {              \
      hp_entry_t *cur_entry;                                                 \
      /* Call the mode's endfn callback. */                                  \
      /* NOTE(cjiang): we want to call this 'end_fn_cb' before */            \
      /* 'hp_mode_common_endfn' to avoid including the time in */            \
      /* 'hp_mode_common_endfn' in the profiling results.      */            \
      hp_globals.mode_cb.end_fn_cb((entries) TSRMLS_CC);                     \
      cur_entry = (*(entries));                                              \
      /* Call the universal callback */                                      \
      hp_mode_common_endfn((entries), (cur_entry) TSRMLS_CC);                \
      /* Free top entry and update entries linked list */                    \
      (*(entries)) = (*(entries))->prev_hprof;                               \
      hp_fast_free_hprof_entry(cur_entry);                                   \
    }                                                                        \
  } while (0)

/* Special handling */
#define POS_ENTRY_GENERATEBLOCK 6
#define POS_ENTRY_BEFORETOHTML  7
#define POS_ENTRY_AFTERTOHTML   8
#define POS_ENTRY_EVENTS_ONLY   9 /* Anything below won't be processed unless the special cookie is set */


#endif /* end of include guard: QUANTA_MON_PROFILER_H_ */
