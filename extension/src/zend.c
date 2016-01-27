#include "quanta_mon.h"

#if PHP_VERSION_ID < 50500
/* Pointer to the original execute function */
static ZEND_DLEXPORT void (*_zend_execute) (zend_op_array *ops TSRMLS_DC);

/* Pointer to the origianl execute_internal function */
static ZEND_DLEXPORT void (*_zend_execute_internal) (zend_execute_data *data,
                           int ret TSRMLS_DC);
#else
/* Pointer to the original execute function */
static void (*_zend_execute_ex) (zend_execute_data *execute_data TSRMLS_DC);

/* Pointer to the origianl execute_internal function */
static void (*_zend_execute_internal) (zend_execute_data *data,
                      struct _zend_fcall_info *fci, int ret TSRMLS_DC);
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

void hp_hijack_zend_execute(uint32_t flags) {
  /* Replace zend_compile with our proxy */
  _zend_compile_file = zend_compile_file;
  zend_compile_file  = hp_compile_file;

  /* Replace zend_compile_string with our proxy */
  _zend_compile_string = zend_compile_string;
  zend_compile_string = hp_compile_string;

  /* Replace zend_execute with our proxy */
#if PHP_VERSION_ID < 50500
  _zend_execute = zend_execute;
  zend_execute  = hp_execute;
#else
  _zend_execute_ex = zend_execute_ex;
  zend_execute_ex  = hp_execute_ex;
#endif

  /* Replace zend_execute_internal with our proxy */
  _zend_execute_internal = zend_execute_internal;
  if (!(flags & QUANTA_MON_FLAGS_NO_BUILTINS)) {
    /* if NO_BUILTINS is not set (i.e. user wants to profile builtins),
     * then we intercept internal (builtin) function calls.
     */
    zend_execute_internal = hp_execute_internal;
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
  zend_op_array *ops = execute_data->op_array;
#endif
  char          *func = NULL;
  char          *pathname;
  int hp_profile_flag = 1;

  func = hp_get_function_name(ops, &pathname TSRMLS_CC);
  if (!func) {
#if PHP_VERSION_ID < 50500
    _zend_execute(ops TSRMLS_CC);
#else
    _zend_execute_ex(execute_data TSRMLS_CC);
#endif
    return;
  }

  hp_profile_flag = hp_begin_profiling(&hp_globals.entries, func, pathname, execute_data);
#if PHP_VERSION_ID < 50500
  _zend_execute(ops TSRMLS_CC);
#else
  _zend_execute_ex(execute_data TSRMLS_CC);
#endif
  if (hp_globals.entries) {
    hp_end_profiling(&hp_globals.entries, hp_profile_flag, execute_data);
  }
  efree(func);
}

#undef EX
#define EX(element) ((execute_data)->element)

/**
 * Very similar to hp_execute. Proxy for zend_execute_internal().
 * Applies to zend builtin functions.
 *
 * @author hzhao, kannan
 */

#if PHP_VERSION_ID < 50500
#define EX_T(offset) (*(temp_variable *)((char *) EX(Ts) + offset))

ZEND_DLEXPORT void hp_execute_internal(zend_execute_data *execute_data, int ret TSRMLS_DC) {
#else
#define EX_T(offset) (*EX_TMP_VAR(execute_data, offset))

ZEND_DLEXPORT void hp_execute_internal(zend_execute_data *execute_data,
struct _zend_fcall_info *fci, int ret TSRMLS_DC) {
#endif
  zend_execute_data *current_data;
  char             *func = NULL;
  char             *pathname;
  int    hp_profile_flag = 1;

  current_data = EG(current_execute_data);
  func = hp_get_function_name(current_data->op_array, &pathname TSRMLS_CC);
  if (func) {
    hp_profile_flag = hp_begin_profiling(&hp_globals.entries, func, pathname, execute_data);
  }

  if (!_zend_execute_internal) {
    /* no old override to begin with. so invoke the builtin's implementation  */

#if ZEND_EXTENSION_API_NO >= 220121212
    /* PHP 5.5. This is just inlining a copy of execute_internal(). */

    if (fci != NULL) {
      ((zend_internal_function *) execute_data->function_state.function)->handler(
        fci->param_count,
        *fci->retval_ptr_ptr,
        fci->retval_ptr_ptr,
        fci->object_ptr,
        1 TSRMLS_CC);
    } else {
      zval **return_value_ptr = &EX_TMP_VAR(execute_data, execute_data->opline->result.var)->var.ptr;
      ((zend_internal_function *) execute_data->function_state.function)->handler(
        execute_data->opline->extended_value,
        *return_value_ptr,
        (execute_data->function_state.function->common.fn_flags & ZEND_ACC_RETURN_REFERENCE)
          ? return_value_ptr
          : NULL,
        execute_data->object,
        ret TSRMLS_CC);
    }
#elif ZEND_EXTENSION_API_NO >= 220100525
    zend_op *opline = EX(opline);
    temp_variable *retvar = &EX_T(opline->result.var);
    ((zend_internal_function *) EX(function_state).function)->handler(
                       opline->extended_value,
                       retvar->var.ptr,
                       (EX(function_state).function->common.fn_flags & ZEND_ACC_RETURN_REFERENCE) ?
                       &retvar->var.ptr:NULL,
                       EX(object), ret TSRMLS_CC);
#else
    zend_op *opline = EX(opline);
    ((zend_internal_function *) EX(function_state).function)->handler(
                       opline->extended_value,
                       EX_T(opline->result.u.var).var.ptr,
                       EX(function_state).function->common.return_reference ?
                       &EX_T(opline->result.u.var).var.ptr:NULL,
                       EX(object), ret TSRMLS_CC);
#endif
  } else {
    /* call the old override */
#if PHP_VERSION_ID < 50500
    _zend_execute_internal(execute_data, ret TSRMLS_CC);
#else
    _zend_execute_internal(execute_data, fci, ret TSRMLS_CC);
#endif
  }

  if (func) {
    if (hp_globals.entries) {
      hp_end_profiling(&hp_globals.entries, hp_profile_flag, execute_data);
    }
    efree(func);
  }

}

/**
 * Proxy for zend_compile_file(). Used to profile PHP compilation time.
 *
 * @author kannan, hzhao
 */
ZEND_DLEXPORT zend_op_array* hp_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC) {

  char           *filename;
  char           *func;
  int             len;
  zend_op_array  *ret;
  int             hp_profile_flag = 1;

  filename = strdup(hp_get_base_filename(file_handle->filename));
  len      = strlen("load") + strlen(filename) + 3;
  func      = (char *)emalloc(len);
  snprintf(func, len, "load::%s", filename);

  hp_profile_flag = hp_begin_profiling(&hp_globals.entries, func, filename, NULL);
  ret = _zend_compile_file(file_handle, type TSRMLS_CC);
  if (hp_globals.entries) {
    hp_end_profiling(&hp_globals.entries, hp_profile_flag, NULL);
  }

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

    hp_profile_flag = hp_begin_profiling(&hp_globals.entries, func, filename, NULL);
    ret = _zend_compile_string(source_string, filename TSRMLS_CC);
    if (hp_globals.entries) {
        hp_end_profiling(&hp_globals.entries, hp_profile_flag, NULL);
    }

    efree(func);
    return ret;
}
