#include "quanta_mon.h"

char *hp_get_function_name_fast(zend_execute_data *execute_data TSRMLS_DC) {
#if PHP_MAJOR_VERSION < 7
  return execute_data ? execute_data->function_state.function->common.function_name : NULL;
#else
  return (execute_data && execute_data->func && execute_data->func->common.function_name) ?
    ZSTR_VAL(execute_data->func->common.function_name)
    : NULL;
#endif
}

static zend_function *hp_get_current_function(zend_execute_data *execute_data) {
#if PHP_MAJOR_VERSION < 7
  return execute_data ? execute_data->function_state.function : NULL;
#else
  return execute_data ? execute_data->func : NULL;
#endif
}

const char *hp_get_class_name(zend_execute_data *data TSRMLS_DC) {
  const char *cls = NULL;
  zend_function      *curr_func;

  if (!(curr_func = hp_get_current_function(data)))
    return NULL;
#if PHP_MAJOR_VERSION < 7
  if (curr_func->common.scope) {
    cls = curr_func->common.scope->name;
  } else if (data->object) {
    cls = Z_OBJCE_P(data->object)->name;
  }
#else
  if (curr_func->common.scope && curr_func->common.scope->name) {
    cls = ZSTR_VAL(curr_func->common.scope->name);
  } else if (Z_OBJ(data->This) && Z_OBJCE(data->This)->name) {
    cls = ZSTR_VAL(Z_OBJCE(data->This)->name);
  }
#endif
  return cls;
}


/**
 * Get the name of the current function. The name is qualified with
 * the class name if the function is in a class.
 *
 * @author kannan, hzhao
 */
char *hp_get_function_name(zend_execute_data *data TSRMLS_DC) {
  const char        *func = NULL;
  const char        *cls = NULL;
  char              *ret = NULL;
  zend_function      *curr_func;

  if (!data || !(curr_func = hp_get_current_function(data)))
    return NULL;
  func = hp_get_function_name_fast(data TSRMLS_CC);
  if (func) {
    cls = hp_get_class_name(data TSRMLS_CC);
    if (cls) {
      int   len;
      len = strlen(cls) + strlen(func) + 8;
      ret = (char*)emalloc(len);
      snprintf(ret, len, "%s::%s", cls, func);
    } else {
      ret = estrdup(func);
    }
  } else {
    long     curr_op;
    int      add_filename = 1;

    /* we are dealing with a special directive/function like
     * include, eval, etc.
     */
#if ZEND_EXTENSION_API_NO >= 220121212
    if (data->prev_execute_data) {
      curr_op = data->prev_execute_data->opline->extended_value;
    } else {
      curr_op = data->opline->extended_value;
    }
#elif ZEND_EXTENSION_API_NO >= 220100525
    curr_op = data->opline->extended_value;
#else
    curr_op = data->opline->op2.u.constant.value.lval;
#endif

    switch (curr_op) {
      case ZEND_EVAL:
        func = "eval";
        break;
      case ZEND_INCLUDE:
        func = "include";
        add_filename = 1;
        break;
      case ZEND_REQUIRE:
        func = "require";
        add_filename = 1;
        break;
      case ZEND_INCLUDE_ONCE:
        func = "include_once";
        add_filename = 1;
        break;
      case ZEND_REQUIRE_ONCE:
        func = "require_once";
        add_filename = 1;
        break;
      default:
        func = "???_op";
        break;
    }

    /* For some operations, we'll add the filename as part of the function
     * name to make the reports more useful. So rather than just "include"
     * you'll see something like "run_init::foo.php" in your reports.
     */
    if (add_filename){
      const char *filename = NULL;
      int   len;

      if (curr_func->op_array.filename) {
#if PHP_MAJOR_VERSION < 7
        filename = hp_get_base_filename(curr_func->op_array.filename);
#else
        filename = hp_get_base_filename(ZSTR_VAL(curr_func->op_array.filename));
#endif
      }
      if (!filename)
        filename = "???";
      len      = strlen("run_init::") + strlen(filename) + 1;
      ret      = (char *)emalloc(len);
      snprintf(ret, len, "run_init::%s", filename);
    } else {
      ret = estrdup(func);
    }
  }
  return ret;
}
