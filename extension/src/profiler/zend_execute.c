#include "quanta_mon.h"

#if PHP_VERSION_ID < 50500
/* Pointer to the original execute function */
static ZEND_DLEXPORT void (*_zend_execute) (zend_op_array *ops TSRMLS_DC);

/* Pointer to the origianl execute_internal function */
static ZEND_DLEXPORT void (*_zend_execute_internal) (zend_execute_data *data,
                           int ret TSRMLS_DC);
#elif PHP_MAJOR_VERSION < 7
/* Pointer to the original execute function */
static void (*_zend_execute_ex) (zend_execute_data *execute_data TSRMLS_DC);

/* Pointer to the origianl execute_internal function */
static void (*_zend_execute_internal) (zend_execute_data *data,
                      struct _zend_fcall_info *fci, int ret TSRMLS_DC);
#else
static void (*_zend_execute_ex) (zend_execute_data *execute_data);
static void (*_zend_execute_internal) (zend_execute_data *execute_data, zval *return_value);
#endif

/* Pointer to the original compile function */
static zend_op_array * (*_zend_compile_file) (zend_file_handle *file_handle, int type TSRMLS_DC);

/* Pointer to the original compile string function (used by eval) */
static zend_op_array * (*_zend_compile_string) (zval *source_string, char *filename TSRMLS_DC);


void hp_restore_original_zend_execute(void) {
  /* Remove proxies, restore the originals */
  #if PHP_VERSION_ID < 50500
    zend_execute          = _zend_execute;
  #else
    zend_execute_ex       = _zend_execute_ex;
  #endif
    zend_execute_internal = _zend_execute_internal;
    zend_compile_file     = _zend_compile_file;
    zend_compile_string   = _zend_compile_string;
}

void hp_hijack_zend_execute(long level) {
#if PHP_VERSION_ID < 50500
  _zend_execute = zend_execute;
  zend_execute  = hp_execute;
#else
  _zend_execute_ex = zend_execute_ex;
  zend_execute_ex  = hp_execute_ex;
#endif

  _zend_execute_internal = zend_execute_internal;
  zend_execute_internal = hp_execute_internal;

  _zend_compile_file = zend_compile_file;
  _zend_compile_string = zend_compile_string;
  if (level == QUANTA_MON_MODE_HIERARCHICAL) {
    zend_compile_string = hp_compile_string;
    zend_compile_file  = hp_compile_file;
  }
}

/**
 * ***************************
 * PHP EXECUTE/COMPILE PROXIES
 * ***************************
 */


/**
 * QuantaMon enable replaced the zend_execute function with this
 * new execute function. We can do whatever profiling we need to
 * before and after calling the actual zend_execute().
 *
 * @author hzhao, kannan
 */
#if PHP_VERSION_ID < 50500
ZEND_DLEXPORT void hp_execute (zend_op_array *ops TSRMLS_DC) {
  zend_execute_data *execute_data = EG(current_execute_data);
#else
ZEND_DLEXPORT void hp_execute_ex (zend_execute_data *execute_data TSRMLS_DC) {
#endif
  char *func = NULL;
  int hp_profile_flag = 1;
  uint64_t start1, start2;
  uint64_t end1, end2;
  uint64_t last;

  start1 = cycle_timer();
  if (hp_globals.profiler_level == QUANTA_MON_MODE_HIERARCHICAL) {
    func = hp_get_function_name(execute_data TSRMLS_CC);
  } else {
    func = hp_get_function_name_fast(execute_data TSRMLS_CC);
  }
  if (!func) {
#if PHP_VERSION_ID < 50500
    _zend_execute(ops TSRMLS_CC);
#else
    _zend_execute_ex(execute_data TSRMLS_CC);
#endif
    return;
  }

  hp_profile_flag = hp_begin_profiling(&hp_globals.entries, func, execute_data TSRMLS_CC);
  end1 = cycle_timer();
#if PHP_VERSION_ID < 50500
  _zend_execute(ops TSRMLS_CC);
#else
  _zend_execute_ex(execute_data TSRMLS_CC);
#endif
  start2 = cycle_timer();
  hp_end_profiling(&hp_globals.entries, hp_profile_flag, execute_data TSRMLS_CC);
  if (hp_globals.profiler_level == QUANTA_MON_MODE_HIERARCHICAL)
    efree(func);
  end2 = cycle_timer();
  last = hp_globals.internal_match_counters.profiling_cycles;
  hp_globals.internal_match_counters.profiling_cycles += end1 - start1 + end2 - start2;
  if (hp_globals.internal_match_counters.profiling_cycles < last)
    PRINTF_QUANTA("OVERFLOW :(\n");
}

#undef EX
#define EX(element) ((execute_data)->element)

/**
 * Very similar to hp_execute. Proxy for zend_execute_internal().
 * Applies to zend builtin functions.
 *
 * @author hzhao, kannan
 */
 #if PHP_VERSION_ID >= 70000
 ZEND_DLEXPORT void hp_execute_internal(zend_execute_data *execute_data, zval *return_value) {
 #elif PHP_VERSION_ID < 50500
 #define EX_T(offset) (*(temp_variable *)((char *) EX(Ts) + offset))

 ZEND_DLEXPORT void hp_execute_internal(zend_execute_data *execute_data,
                                        int ret TSRMLS_DC) {
 #else
 #define EX_T(offset) (*EX_TMP_VAR(execute_data, offset))

 ZEND_DLEXPORT void hp_execute_internal(zend_execute_data *execute_data,
                                        struct _zend_fcall_info *fci, int ret TSRMLS_DC) {
 #endif
     char *func = NULL;
     int hp_profile_flag = -1;
     uint64_t start1, start2;
     uint64_t end1, end2;
     uint64_t last;

     start1 = cycle_timer();
     if (hp_globals.profiler_level == QUANTA_MON_MODE_HIERARCHICAL) {
       func = hp_get_function_name(execute_data TSRMLS_CC);
     } else {
       func = hp_get_function_name_fast(execute_data TSRMLS_CC);
     }
     if (func) {
       hp_profile_flag = hp_begin_profiling(&hp_globals.entries, func, execute_data TSRMLS_CC);
     }

     end1 = cycle_timer();
     if (!_zend_execute_internal) {
 #if PHP_VERSION_ID >= 70000
         execute_internal(execute_data, return_value TSRMLS_CC);
 #elif PHP_VERSION_ID < 50500
         execute_internal(execute_data, ret TSRMLS_CC);
 #else
         execute_internal(execute_data, fci, ret TSRMLS_CC);
 #endif
     } else {
         /* call the old override */
 #if PHP_VERSION_ID >= 70000
         _zend_execute_internal(execute_data, return_value TSRMLS_CC);
 #elif PHP_VERSION_ID < 50500
         _zend_execute_internal(execute_data, ret TSRMLS_CC);
 #else
         _zend_execute_internal(execute_data, fci, ret TSRMLS_CC);
 #endif
     }
     start2 = cycle_timer();
     if (func) {
       hp_end_profiling(&hp_globals.entries, hp_profile_flag, execute_data TSRMLS_CC);
     }
     if (hp_globals.profiler_level == QUANTA_MON_MODE_HIERARCHICAL)
       efree(func);
     end2 = cycle_timer();
     last = hp_globals.internal_match_counters.profiling_cycles;
     hp_globals.internal_match_counters.profiling_cycles += end1 - start1 + end2 - start2;
     if (hp_globals.internal_match_counters.profiling_cycles < last)
       PRINTF_QUANTA("OVERFLOW :(\n");
 }

/**
 * Proxy for zend_compile_file(). Used to profile PHP compilation time.
 *
 * @author kannan, hzhao
 */
ZEND_DLEXPORT zend_op_array* hp_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC) {

  const char     *filename;
  char           *func;
  int             len;
  zend_op_array  *ret;
  int             hp_profile_flag = 1;

  filename = hp_get_base_filename(file_handle->filename);
  len = strlen("load") + strlen(filename) + 3;
  func = (char *)emalloc(len);
  snprintf(func, len, "load::%s", filename);

  hp_profile_flag = hp_begin_profiling(&hp_globals.entries, func, NULL TSRMLS_CC);
  ret = _zend_compile_file(file_handle, type TSRMLS_CC);
  hp_end_profiling(&hp_globals.entries, hp_profile_flag, NULL TSRMLS_CC);

  efree(func);
  return ret;
}

/**
 * Proxy for zend_compile_string(). Used to profile PHP eval compilation time.
 */
ZEND_DLEXPORT zend_op_array* hp_compile_string(zval *source_string, char *filename TSRMLS_DC) {

    char          *func;
    int            len;
    zend_op_array *ret;
    int            hp_profile_flag = 1;

    len  = strlen("eval") + strlen(filename) + 3;
    func = (char *)emalloc(len);
    snprintf(func, len, "eval::%s", filename);

    hp_profile_flag = hp_begin_profiling(&hp_globals.entries, func, NULL TSRMLS_CC);
    ret = _zend_compile_string(source_string, filename TSRMLS_CC);
    hp_end_profiling(&hp_globals.entries, hp_profile_flag, NULL TSRMLS_CC);
    efree(func);
    return ret;
}
