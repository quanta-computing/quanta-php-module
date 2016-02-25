#include "quanta_mon.h"

/* Free this memory at the end of profiling */
inline void hp_array_del(char **name_array) {
  if (name_array != NULL) {
    int i = 0;
    for(; name_array[i] != NULL && i < QUANTA_MON_MAX_IGNORED_FUNCTIONS; i++) {
      efree(name_array[i]);
    }
    efree(name_array);
  }
}

/**
 * Looksup the hash table for the given symbol
 * Initializes a new array() if symbol is not present
 *
 * @author kannan, veeve
 */
zval * hp_hash_lookup(char *symbol  TSRMLS_DC) {
  HashTable   *ht;
  void        *data;
  zval        *counts = (zval *) 0;

  /* Bail if something is goofy */
  if (!hp_globals.stats_count || !(ht = HASH_OF(hp_globals.stats_count))) {
    return (zval *) 0;
  }

  /* Lookup our hash table */
  if (zend_hash_find(ht, symbol, strlen(symbol) + 1, &data) == SUCCESS) {
    /* Symbol already exists */
    counts = *(zval **) data;
  }
  else {
    /* Add symbol to hash table */
    MAKE_STD_ZVAL(counts);
    array_init(counts);
    add_assoc_zval(hp_globals.stats_count, symbol, counts);
  }

  return counts;
}


/**
 * Takes an input of the form /a/b/c/d/foo.php and returns
 * a pointer to one-level directory and basefile name
 * (d/foo.php) in the same string.
 */
const char *hp_get_base_filename(const char *filename) {
  if (!filename)
    return "";
  return filename;
}

/**
 * Get the name of the current function. The name is qualified with
 * the class name if the function is in a class.
 *
 * @author kannan, hzhao
 */
char *hp_get_function_name(zend_op_array *ops, char **pathname TSRMLS_DC) {
  zend_execute_data *data;
  const char        *func = NULL;
  const char        *cls = NULL;
  char              *ret = NULL;
  zend_function      *curr_func;

  *pathname = "BUG";
  data = EG(current_execute_data);

  if (data) {
    /* shared meta data for function on the call stack */
    curr_func = data->function_state.function;

    /* extract function name from the meta info */
    func = curr_func->common.function_name;

    if (func) {
      /* previously, the order of the tests in the "if" below was
       * flipped, leading to incorrect function names in profiler
       * reports. When a method in a super-type is invoked the
       * profiler should qualify the function name with the super-type
       * class name (not the class name based on the run-time type
       * of the object.
       */
      if (curr_func->common.scope) {
        cls = curr_func->common.scope->name;
      } else if (data->object) {
        cls = Z_OBJCE(*data->object)->name;
      }

      if (cls) {
        const char *filename;
        int   len;
        if (ops)
     filename = hp_get_base_filename(ops->filename);
        else
           filename = "null";
        *pathname = strdup(filename);
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
        const char *filename;
        int   len;
        filename = hp_get_base_filename((curr_func->op_array).filename);
        len      = strlen("run_init") + strlen(filename) + 3;
        ret      = (char *)emalloc(len);
        snprintf(ret, len, "run_init::%s", filename);
        *pathname = strdup(filename);
      } else {
        ret = estrdup(func);
      }
    }
  }
  return ret;
}

/**
 * A hash function to calculate a 8-bit hash code for a function name.
 * This is based on a small modification to 'zend_inline_hash_func' by summing
 * up all bytes of the ulong returned by 'zend_inline_hash_func'.
 *
 * @param str, char *, string to be calculated hash code for.
 *
 * @author cjiang
 */
inline uint8_t hp_inline_hash(char * str) {
  ulong h = 5381;
  uint i = 0;
  uint8_t res = 0;

  while (*str) {
    h += (h << 5);
    h ^= (ulong) *str++;
  }

  for (i = 0; i < sizeof(ulong); i++) {
    res += ((uint8_t *)&h)[i];
  }
  return res;
}


/**
 * Build a caller qualified name for a callee.
 *
 * For example, if A() is caller for B(), then it returns "A==>B".
 * Recursive invokations are denoted with @<n> where n is the recursion
 * depth.
 *
 * For example, "foo==>foo@1", and "foo@2==>foo@3" are examples of direct
 * recursion. And  "bar==>foo@1" is an example of an indirect recursive
 * call to foo (implying the foo() is on the call stack some levels
 * above).
 *
 * @author kannan, veeve
 */
size_t hp_get_function_stack(hp_entry_t *entry, int level, char *result_buf, size_t result_len) {
  size_t         len = 0;

  /* End recursion if we dont need deeper levels or we dont have any deeper
   * levels */
  if (!entry->prev_hprof || (level <= 1)) {
    return hp_get_entry_name(entry, result_buf, result_len);
  }

  /* Take care of all ancestors first */
  len = hp_get_function_stack(entry->prev_hprof,
                              level - 1,
                              result_buf,
                              result_len);

  /* Append the delimiter */
# define    HP_STACK_DELIM        "==>"
# define    HP_STACK_DELIM_LEN    (sizeof(HP_STACK_DELIM) - 1)

  if (result_len < (len + HP_STACK_DELIM_LEN)) {
    /* Insufficient result_buf. Bail out! */
    return len;
  }

  /* Add delimiter only if entry had ancestors */
  if (len) {
    strncat(result_buf + len,
            HP_STACK_DELIM,
            result_len - len);
    len += HP_STACK_DELIM_LEN;
  }

# undef     HP_STACK_DELIM_LEN
# undef     HP_STACK_DELIM

  /* Append the current function name */
  return len + hp_get_entry_name(entry,
                                 result_buf + len,
                                 result_len - len);
}


/**
 * Truncates the given timeval to the nearest slot begin, where
 * the slot size is determined by intr
 *
 * @param  tv       Input timeval to be truncated in place
 * @param  intr     Time interval in microsecs - slot width
 * @return void
 * @author veeve
 */
void hp_trunc_time(struct timeval *tv, uint64_t intr) {
  uint64_t time_in_micro;

  /* Convert to microsecs and trunc that first */
  time_in_micro = (tv->tv_sec * 1000000) + tv->tv_usec;
  time_in_micro /= intr;
  time_in_micro *= intr;

  /* Update tv */
  tv->tv_sec  = (time_in_micro / 1000000);
  tv->tv_usec = (time_in_micro % 1000000);
}
