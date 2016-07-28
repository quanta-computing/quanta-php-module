#include "quanta_mon.h"

static char *extract_request_uri(HashTable *_SERVER TSRMLS_DC) {
  zval *request_uri;

  request_uri = zend_hash_find_compat(_SERVER, "REQUEST_URI", sizeof("REQUEST_URI") - 1);
  if (!request_uri || Z_TYPE_P(request_uri) != IS_STRING) {
    PRINTF_QUANTA("NO REQUEST URI\n");
    return NULL;
  }
  PRINTF_QUANTA("REQUEST URI: %s\n", Z_STRVAL_P(request_uri));
  return estrdup(Z_STRVAL_P(request_uri));
}

static int extract_step_clock_and_mode(HashTable *_SERVER TSRMLS_DC) {
  zval *data;
  char *quanta_header;
  char *quanta_mode;
  char *quanta_clock;

  data = zend_hash_find_compat(_SERVER, QUANTA_HTTP_HEADER, sizeof(QUANTA_HTTP_HEADER) - 1);
  if (!data || Z_TYPE_P(data) != IS_STRING) {
    PRINTF_QUANTA("NO QUANTA HEADER\n");
    return -1;
  }
  quanta_header = Z_STRVAL_P(data);
  PRINTF_QUANTA("QUANTA HEADER: %s\n", quanta_header);
  hp_globals.quanta_step_id = strtoull(quanta_header, &quanta_clock, 10);
  if (quanta_clock == quanta_header) {
    PRINTF_QUANTA("WRONG FORMAT %s for step_id\n", quanta_header);
    return -1;
  }
  hp_globals.quanta_clock = strtoull(quanta_clock, &quanta_mode, 10);
  if (quanta_mode == quanta_clock) {
    PRINTF_QUANTA("WRONG FORMAT %s for clock\n", quanta_header);
    return -1;
  }
  while (*quanta_mode == ' ')
    quanta_mode++;
  if (!*quanta_mode || !strcmp(quanta_mode, QUANTA_HTTP_HEADER_MODE_MAGE))
    return QUANTA_MON_MODE_MAGENTO_PROFILING;
  else if (!strcmp(quanta_mode, QUANTA_HTTP_HEADER_MODE_FULL))
    return QUANTA_MON_MODE_HIERARCHICAL;
  else
    return -1;
}


static void arm_server_auto_global(TSRMLS_D) {
  zend_auto_global* auto_global;
#if PHP_MAJOR_VERSION >= 7
  zval *auto_global_zval;
#endif

  if (zend_hash_exists_compat(&EG(symbol_table), "_SERVER", sizeof("_SERVER")))
    return;
#if PHP_MAJOR_VERSION < 7
  if (zend_hash_find(CG(auto_globals), "_SERVER", sizeof("_SERVER"), (void **)&auto_global) == FAILURE)
    return;
  auto_global->armed = auto_global->auto_global_callback(auto_global->name,
    auto_global->name_len TSRMLS_CC);
#else
  auto_global_zval = zend_hash_find_compat(CG(auto_globals), "_SERVER", sizeof("_SERVER") - 1);
  if (!auto_global_zval) {
    return;
  }
  auto_global = (zend_auto_global *)Z_PTR_P(auto_global_zval);
  if (auto_global)
    auto_global->armed = auto_global->auto_global_callback(auto_global->name);
#endif
}

/* We get needed informations in http header QUANTA_HTTP_HEADER:
** It should contains the step id and the (optional) profiling mode which defaults to magento
** profiling only
**
** @returns: the profiled mode
*/
static int extract_headers_info(TSRMLS_D) {
  HashTable *_SERVER;
  zval *zserver;
  int mode;

  hp_globals.quanta_clock = 0;
  arm_server_auto_global(TSRMLS_C);
  zserver = zend_hash_find_compat(&EG(symbol_table), "_SERVER", sizeof("_SERVER") - 1);
  if (!zserver || Z_TYPE_P(zserver) != IS_ARRAY) {
    PRINTF_QUANTA("NO _SERVER\n");
    return -1;
  }
  _SERVER = Z_ARRVAL_P(zserver);
  hp_globals.request_uri = extract_request_uri(_SERVER TSRMLS_CC);
  mode = extract_step_clock_and_mode(_SERVER TSRMLS_CC);
  if (mode == -1) {
    if (!hp_globals.request_uri || strstr(hp_globals.request_uri, hp_globals.admin_url)) {
      return QUANTA_MON_MODE_EVENTS_ONLY;
    } else {
      efree(hp_globals.request_uri);
      hp_globals.request_uri = NULL;
      return -1;
    }
  } else {
    return mode;
  }
}

/**
 * Request init callback.
 */
PHP_RINIT_FUNCTION(quanta_mon) {
  int mode;
  long flags;
  uint64_t start;
  uint64_t end;

  start = cycle_timer();
  mode = extract_headers_info(TSRMLS_C);
  if (mode == -1) {
    PRINTF_QUANTA("PROFILER NOT ENABLED\n");
    return SUCCESS;
  }
  PRINTF_QUANTA("PROFILER ENABLED WITH MODE %d\n", mode);
  if (mode <= QUANTA_MON_MODE_SAMPLED)
    flags = QUANTA_MON_FLAGS_CPU | QUANTA_MON_FLAGS_MEMORY;
  else
    flags = 0;
  hp_globals.global_tsc.start = start;
  bzero(&hp_globals.internal_match_counters, sizeof(hp_globals.internal_match_counters));
  hp_begin(mode, flags TSRMLS_CC);
  end = cycle_timer();
  hp_globals.internal_match_counters.init_cycles += end - start;
  return SUCCESS;
}
