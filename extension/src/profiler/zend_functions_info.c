#include "quanta_mon.h"

char *hp_get_function_name_fast(zend_execute_data *execute_data) {
  return (execute_data && execute_data->func && execute_data->func->common.function_name) ?
    ZSTR_VAL(execute_data->func->common.function_name)
    : NULL;
}

static zend_function *hp_get_current_function(zend_execute_data *execute_data) {
  return execute_data ? execute_data->func : NULL;
}

const char *hp_get_class_name(zend_execute_data *data) {
  const char *cls = NULL;
  zend_function      *curr_func;

  if (!(curr_func = hp_get_current_function(data)))
    return NULL;
  if (curr_func->common.scope && curr_func->common.scope->name) {
    cls = ZSTR_VAL(curr_func->common.scope->name);
  } else if (Z_OBJ(data->This) && Z_OBJCE(data->This)->name) {
    cls = ZSTR_VAL(Z_OBJCE(data->This)->name);
  }
  return cls;
}


/**
 * Get the name of the current function. The name is qualified with
 * the class name if the function is in a class.
 *
 * @author kannan, hzhao
 */
char *hp_get_function_name(zend_execute_data *data) {
  const char        *func = NULL;
  const char        *cls = NULL;
  char              *ret = NULL;
  zend_function      *curr_func;

  if (!data || !(curr_func = hp_get_current_function(data)))
    return NULL;
  func = hp_get_function_name_fast(data);
  if (func) {
    cls = hp_get_class_name(data);
    if (cls) {
      size_t   len;
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
    if (data->prev_execute_data) {
      curr_op = data->prev_execute_data->opline->extended_value;
    } else {
      curr_op = data->opline->extended_value;
    }

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
      size_t   len;

      if (curr_func->op_array.filename) {
        filename = hp_get_base_filename(ZSTR_VAL(curr_func->op_array.filename));
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
