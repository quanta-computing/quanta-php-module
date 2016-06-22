#include "quanta_mon.h"
#include <inttypes.h>

#define MAX_METRIC_NAME_LENGTH 1024


/* The metric request_uri should be duplicated for each Monikor Handler which needs to check against
** the original step URL.
*/
static void fetch_request_uri(struct timeval *clock, monikor_metric_list_t *metrics) {
  monikor_metric_t *metric;
  char metric_name[MAX_METRIC_NAME_LENGTH];

  if (!hp_globals.request_uri)
    return;
  if (hp_globals.profiler_level == QUANTA_MON_MODE_MAGENTO_PROFILING)
    sprintf(metric_name, "magento.%zu.request_uri", hp_globals.quanta_step_id);
  else
    sprintf(metric_name, "qtracer.%zu.request_uri", hp_globals.quanta_step_id);
  metric = monikor_metric_string(metric_name, clock, hp_globals.request_uri);
  if (metric)
    monikor_metric_list_push(metrics, metric);
}

static void fetch_xhprof_metrics(struct timeval *clock, monikor_metric_list_t *metrics TSRMLS_DC) {
  char metric_name[MAX_METRIC_NAME_LENGTH];
  zval encoded;

  if (hp_globals.profiler_level > QUANTA_MON_MODE_SAMPLED)
    return;
  ZVAL_NULL(&encoded);
  if (safe_call_function("json_encode", &encoded, IS_STRING, 1, &hp_globals.stats_count TSRMLS_CC)) {
    PRINTF_QUANTA("Error: cannot json encode xhprof output\n");
  } else {
    sprintf(metric_name, "qtracer.%zu.json", hp_globals.quanta_step_id);
    monikor_metric_t *metric = monikor_metric_string(metric_name, clock, Z_STRVAL(encoded));
    if (metric)
      monikor_metric_list_push(metrics, metric);
  }
  zval_dtor(&encoded);
}

static void push_magento_version_metric(struct timeval *clock, monikor_metric_list_t *metrics,
char *version, char *edition) {
  char value[512];

  if (snprintf(value, 512, "%s %s", version, edition) < 512) {
    monikor_metric_t *metric = monikor_metric_string("magento.version.magento", clock, value);
    if (metric) {
      monikor_metric_list_push(metrics, metric);
      PRINTF_QUANTA("Magento version %s\n", value);
    }
  } else {
    PRINTF_QUANTA("Cannot format magento version\n");
  }
}

static void fetch_magento2_version(struct timeval *clock, monikor_metric_list_t *metrics TSRMLS_DC) {
  zval *objectManager;
  zval *productMetaData;
  zval version;
  zval edition;
  zval *params[1];
  int ret;

  MAKE_STD_ZVAL(objectManager);
  MAKE_STD_ZVAL(productMetaData);
  MAKE_STD_ZVAL(params[0]);
  ZVAL_NULL(&version);
  ZVAL_NULL(&edition);
  ret = safe_call_function("Magento\\Framework\\App\\ObjectManager::getInstance",
    objectManager, IS_OBJECT, 0, NULL TSRMLS_CC);
  if (ret) {
    PRINTF_QUANTA("Cannot get Magento\\Framework\\App\\ObjectManager\n");
    goto end;
  }
  ZVAL_STRING(params[0], "Magento\\Framework\\App\\ProductMetadataInterface", 1);
  if (safe_call_method(objectManager, "get", productMetaData, IS_OBJECT, 1, params TSRMLS_CC)) {
    PRINTF_QUANTA("Cannot get Magento\\Framework\\App\\ProductMetaData\n");
    goto end;
  }
  if (safe_call_method(productMetaData, "getVersion", &version, IS_STRING, 0, NULL TSRMLS_CC)
  || safe_call_method(productMetaData, "getEdition", &edition, IS_STRING, 0, NULL TSRMLS_CC)) {
    PRINTF_QUANTA("Cannot get magento2 version\n");
    goto end;
  }
  push_magento_version_metric(clock, metrics, Z_STRVAL(version), Z_STRVAL(edition));

end:
  FREE_ZVAL(objectManager);
  FREE_ZVAL(productMetaData);
  FREE_ZVAL(params[0]);
  zval_dtor(&version);
  zval_dtor(&edition);
}

static void fetch_magento_version(struct timeval *clock, monikor_metric_list_t *metrics TSRMLS_DC) {
  zval version;
  zval edition;

  ZVAL_NULL(&version);
  ZVAL_NULL(&edition);
  if (safe_call_function("Mage::getVersion", &version, IS_STRING, 0, NULL TSRMLS_CC)
  || safe_call_function("Mage::getEdition", &edition, IS_STRING, 0, NULL)) {
    PRINTF_QUANTA("Could not get magento version\n");
    goto end;
  }
  push_magento_version_metric(clock, metrics, Z_STRVAL(version), Z_STRVAL(edition));

end:
  zval_dtor(&version);
  zval_dtor(&edition);
}

static void fetch_php_version(struct timeval *clock, monikor_metric_list_t *metrics) {
  monikor_metric_t *metric = monikor_metric_string("magento.version.php", clock, PHP_VERSION);
  if (metric)
    monikor_metric_list_push(metrics, metric);
}

static void fetch_module_version(struct timeval *clock, monikor_metric_list_t *metrics) {
  monikor_metric_t *metric = monikor_metric_string("magento.version.module", clock, QUANTA_MON_VERSION);
  if (metric)
    monikor_metric_list_push(metrics, metric);
}

static void fetch_all_versions(struct timeval *clock, monikor_metric_list_t *metrics TSRMLS_DC) {
  fetch_magento_version(clock, metrics TSRMLS_CC);
  fetch_magento2_version(clock, metrics TSRMLS_CC);
  fetch_php_version(clock, metrics);
  fetch_module_version(clock, metrics);
}

static inline const char *event_class_str(magento_event_t *event) {
  if (event->class == MAGENTO_EVENT_CACHE_CLEAR)
    return "cache_clear";
  else if (event->class == MAGENTO_EVENT_REINDEX)
    return "reindex";
  else
    return "unknown";
}

static void fetch_magento_events(struct timeval *clock, monikor_metric_list_t *metrics) {
  char metric_name[MAX_METRIC_NAME_LENGTH];
  monikor_metric_t *metric;
  magento_event_t *event;
  magento_event_t *prev;

  event = hp_globals.magento_events;
  while (event) {
    prev = event->prev;
    sprintf(metric_name, "magento.events.%s.%s", event_class_str(event), event->type);
    if ((metric = monikor_metric_string(metric_name, clock, event->subtype)))
      monikor_metric_list_push(metrics, metric);
    efree(event->type);
    efree(event->subtype);
    efree(event);
    event = prev;
  }
}

#define PROF_STARTS(i) i, -1
#define PROF_STOPS(i) -1, i

static const struct {
  char *name;
  int8_t starts_a;
  int8_t stops_a;
  int8_t starts_b;
  int8_t stops_b;
} magento_metrics[] = {
  {"loading", PROF_STARTS(0), PROF_STARTS(8)},
  {"before_init_config", PROF_STARTS(0), PROF_STARTS(1)},
  {"init_config", PROF_STARTS(1), PROF_STOPS(1)},
  {"init_cache", PROF_STARTS(2), PROF_STOPS(2)},
  {"load_modules", PROF_STARTS(3), PROF_STOPS(4)},
  {"db_updates", PROF_STARTS(5), PROF_STOPS(5)},
  {"load_db", PROF_STARTS(6), PROF_STOPS(6)},
  {"init_stores", PROF_STARTS(7), PROF_STOPS(7)},
  {"routing", PROF_STOPS(7), PROF_STARTS(8)},
  {"controller", PROF_STARTS(8), PROF_STOPS(11)},
  {"before_layout_loading", PROF_STARTS(8), PROF_STARTS(9)},
  {"layout_loading", PROF_STARTS(9), PROF_STOPS(9)},
  {"between_layout_loading_and_rendering", PROF_STOPS(9), PROF_STARTS(10)},
  {"layout_rendering", PROF_STARTS(10), PROF_STOPS(10)},
  {"after_layout_rendering", PROF_STOPS(10), PROF_STOPS(11)},
  {"before_sending_response", PROF_STOPS(11), PROF_STARTS(12)},
  {"total", PROF_STARTS(0), PROF_STOPS(0)},
  {"before_magento", PROF_STARTS(POS_ENTRY_PHP_TOTAL), PROF_STARTS(0)},
  {"after_magento", PROF_STOPS(0), PROF_STOPS(POS_ENTRY_PHP_TOTAL)},
  {"php_total", PROF_STARTS(POS_ENTRY_PHP_TOTAL), PROF_STOPS(POS_ENTRY_PHP_TOTAL)},
  {0}
};

static void fetch_profiler_sql_metrics(struct timeval *clock, monikor_metric_list_t *metrics,
float cpufreq, size_t metric_idx) {
  char metric_name[MAX_METRIC_NAME_LENGTH];
  char *metric_base_end;
  monikor_metric_t *metric;
  uint64_t sql_cycles;
  uint64_t sql_count;
  size_t start;
  size_t stop;
  size_t i;

  // Ignore if we have no Magento time
  if (hp_globals.monitored_function_tsc_start[0] <= 0)
    return;
  start = magento_metrics[metric_idx].starts_a == -1 ?
    magento_metrics[metric_idx].stops_a + 1: magento_metrics[metric_idx].starts_a;
  stop = magento_metrics[metric_idx].starts_b == -1 ?
    magento_metrics[metric_idx].stops_b + 1: magento_metrics[metric_idx].starts_b;
  /* Mage::run has all queries, so ignore it unless we are counting queries for the total */
  if (!start && strcmp(magento_metrics[metric_idx].name, "total"))
    start = 1;
  if (magento_metrics[metric_idx].starts_a == -1) {
    sql_cycles = hp_globals.monitored_function_sql_cpu_cycles_after[magento_metrics[metric_idx].stops_a];
    sql_count = hp_globals.monitored_function_sql_queries_count_after[magento_metrics[metric_idx].stops_a];
  } else {
    sql_cycles = 0;
    sql_count = 0;
  }
  for (i = start; i < stop; i++) {
    sql_cycles += hp_globals.monitored_function_sql_cpu_cycles[i];
    sql_count += hp_globals.monitored_function_sql_queries_count[i];
    if (i != stop -1 || magento_metrics[metric_idx].starts_b != -1) {
      sql_cycles += hp_globals.monitored_function_sql_cpu_cycles_after[i];
      sql_count += hp_globals.monitored_function_sql_queries_count_after[i];
    }
  }
  sprintf(metric_name, "magento.%zu.profiling.%s.sql.", hp_globals.quanta_step_id,
    magento_metrics[metric_idx].name);
  metric_base_end = metric_name + strlen(metric_name);
  strcpy(metric_base_end, "count");
  metric = monikor_metric_integer(metric_name, clock, sql_count, 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
  strcpy(metric_base_end, "time");
  metric = monikor_metric_float(metric_name, clock, cpu_cycles_to_ms(cpufreq, sql_cycles), 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
}

static void fetch_profiler_metrics(struct timeval *clock, monikor_metric_list_t *metrics, float cpufreq) {
  uint64_t *starts = hp_globals.monitored_function_tsc_start;
  uint64_t *stops = hp_globals.monitored_function_tsc_stop;
  char metric_name[MAX_METRIC_NAME_LENGTH];
  char *metric_base_end;
  monikor_metric_t *metric;
  size_t i;

  if (hp_globals.profiler_level != QUANTA_MON_MODE_MAGENTO_PROFILING)
    return;
  sprintf(metric_name, "magento.%zu.profiling.", hp_globals.quanta_step_id);
  metric_base_end = metric_name + strlen(metric_name);
  for (i = 0; magento_metrics[i].name; i++) {
    uint64_t a = (magento_metrics[i].starts_a != -1) ?
      starts[magento_metrics[i].starts_a] : stops[magento_metrics[i].stops_a];
    uint64_t b = (magento_metrics[i].starts_b != -1) ?
      starts[magento_metrics[i].starts_b] : stops[magento_metrics[i].stops_b];
    strcpy(metric_base_end, magento_metrics[i].name);
    strcpy(metric_base_end + strlen(magento_metrics[i].name), ".time");
    PRINTF_QUANTA("METRIC %s: a: %"PRIu64" b: %"PRIu64" = %f ms\n", metric_name, a, b,
      cpu_cycles_range_to_ms(cpufreq, a, b));
    metric = monikor_metric_float(metric_name, clock, cpu_cycles_range_to_ms(cpufreq, a, b), 0);
    // Ignore if this is a Magento metric and we have no magento total time
    if (metric && (hp_globals.monitored_function_tsc_start[0] > 0
    || !strcmp("php_total", magento_metrics[i].name)))
      monikor_metric_list_push(metrics, metric);
    fetch_profiler_sql_metrics(clock, metrics, cpufreq, i);
  }
}

// TODO! Check if it still works with Magento 1
static void fetch_block_class_file_metric(struct timeval *clock, monikor_metric_list_t *metrics,
magento_block_t *block) {
  monikor_metric_t *metric;
  zval *reflection;
  zval *params[1];
  zval class_file;
  char metric_name[MAX_METRIC_NAME_LENGTH];

  ZVAL_NULL(&class_file);
  MAKE_STD_ZVAL(params[0]);
  ZVAL_STRING(params[0], block->class, 1);
  if (!(reflection = safe_new("ReflectionClass", 1, params TSRMLS_CC))
  || safe_call_method(reflection, "getFileName", &class_file, IS_STRING, 0, NULL TSRMLS_CC)) {
    PRINTF_QUANTA("Cannot get file for class %s\n", block->class);
    goto end;
  }
  PRINTF_QUANTA("Block %s class file: %s\n", block->name, Z_STRVAL(class_file));
  sprintf(metric_name, "magento.%zu.blocks.%.255s.class_file",
    hp_globals.quanta_step_id, block->name);
  if ((metric = monikor_metric_string(metric_name, clock, Z_STRVAL(class_file))))
    monikor_metric_list_push(metrics, metric);

end:
  zval_dtor(&class_file);
  FREE_ZVAL(params[0]);
  if (reflection)
    FREE_ZVAL(reflection);
}

static void fetch_block_class_metric(struct timeval *clock, monikor_metric_list_t *metrics,
magento_block_t *block TSRMLS_DC) {
  char metric_name[MAX_METRIC_NAME_LENGTH];
  monikor_metric_t *metric;
  zval *params[1];

  if (!block->class)
    return;
  sprintf(metric_name, "magento.%zu.blocks.%.255s.class", hp_globals.quanta_step_id, block->name);
  if ((metric = monikor_metric_string(metric_name, clock, block->class)))
    monikor_metric_list_push(metrics, metric);
  MAKE_STD_ZVAL(params[0]);
  ZVAL_STRING(params[0], block->class, 1);
  FREE_ZVAL(params[0]);
}

static void fetch_block_template_metrics(struct timeval *clock, monikor_metric_list_t *metrics,
magento_block_t *block) {
  char metric_name[MAX_METRIC_NAME_LENGTH];
  monikor_metric_t *metric;
  char *metric_base_end;

  if (!block->template)
    return;
  sprintf(metric_name, "magento.%zu.blocks.%.255s.", hp_globals.quanta_step_id, block->name);
  metric_base_end = metric_name + strlen(metric_name);
  strcpy(metric_base_end, "template");
  if ((metric = monikor_metric_string(metric_name, clock, block->template)))
    monikor_metric_list_push(metrics, metric);
}

static void fetch_block_sql_metrics(struct timeval *clock, monikor_metric_list_t *metrics,
float cpufreq, magento_block_t *block) {
  char metric_name[MAX_METRIC_NAME_LENGTH];
  monikor_metric_t *metric;
  char *metric_base_end;

  sprintf(metric_name, "magento.%zu.blocks.%.255s.", hp_globals.quanta_step_id, block->name);
  metric_base_end = metric_name + strlen(metric_name);
  strcpy(metric_base_end, "sql_count");
  if ((metric = monikor_metric_integer(metric_name, clock, block->sql_queries_count, 0)))
    monikor_metric_list_push(metrics, metric);
  strcpy(metric_base_end, "sql_time");
  metric = monikor_metric_float(metric_name, clock, cpu_cycles_to_ms(
    cpufreq, block->sql_cpu_cycles), 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
}

static void fetch_block_metrics(struct timeval *clock, monikor_metric_list_t *metrics, float cpufreq,
magento_block_t *block TSRMLS_DC) {
  monikor_metric_t *metric;
  char metric_name[MAX_METRIC_NAME_LENGTH];
  char *metric_base_end;

  sprintf(metric_name, "magento.%zu.blocks.%.255s.", hp_globals.quanta_step_id, block->name);
  metric_base_end = metric_name + strlen(metric_name);
  fetch_block_class_metric(clock, metrics, block TSRMLS_CC);
  fetch_block_class_file_metric(clock, metrics, block TSRMLS_CC);
  fetch_block_template_metrics(clock, metrics, block);
  fetch_block_sql_metrics(clock, metrics, cpufreq, block);
  strcpy(metric_base_end, "rendering_time");
  uint64_t rendering_time = cpu_cycles_range_to_ms(cpufreq,
    block->renderize_children_cycles + block->tsc_renderize_first_start,
    block->tsc_renderize_last_stop
  );
  metric = monikor_metric_float(metric_name, clock, rendering_time, 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
}

static void fetch_blocks_metrics(struct timeval *clock, monikor_metric_list_t *metrics, float cpufreq TSRMLS_DC) {
  magento_block_t *current_block;
  magento_block_t *next_block;

  current_block = hp_globals.magento_blocks_first;
  while (current_block) {
    next_block = current_block->next;
    if (hp_globals.profiler_level == QUANTA_MON_MODE_MAGENTO_PROFILING)
      fetch_block_metrics(clock, metrics, cpufreq, current_block TSRMLS_CC);
    efree(current_block->class);
    efree(current_block->name);
    efree(current_block->template);
    efree(current_block);
    current_block = next_block;
  }
}

static void send_data_to_monikor(monikor_metric_list_t *metrics) {
  void *data = NULL;
  int sock = -1;
  int flags;
  int result;
  socklen_t result_len = sizeof(result);
  size_t size;
  struct sockaddr_un addr;
  fd_set wrfds;
  struct timeval timeout;

  if (!hp_globals.path_quanta_agent_socket) {
    PRINTF_QUANTA("Cannot send data to monikor: socket not configured\n");
    return;
  }
  PRINTF_QUANTA("Sending data to monikor\n");
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, hp_globals.path_quanta_agent_socket);
  if (monikor_metric_list_serialize(metrics, &data, &size)) {
    PRINTF_QUANTA("Cannot send data to monikor: cannot serialize metrics\n");
    goto end;
  }
  if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1
  || (flags = fcntl(sock, F_GETFL, 0)) == -1 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1
  || (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1 && errno != EINPROGRESS)
  || fcntl(sock, F_SETFL, flags & ~O_NONBLOCK) == -1) {
    PRINTF_QUANTA("Cannot send data to monikor (socket creation error): %s\n", strerror(errno));
    goto end;
  }
  FD_ZERO(&wrfds);
  FD_SET(sock, &wrfds);
  timeout.tv_sec = 0;
  timeout.tv_usec = SEND_METRICS_TIMEOUT_US;
  if (select(sock + 1, NULL, &wrfds, NULL, &timeout) <= 0
  || getsockopt(sock, SOL_SOCKET, SO_ERROR, &result, &result_len) == -1 || result != 0
  || write(sock, data, size) == -1) {
    PRINTF_QUANTA("Cannot send data to monikor: %s\n", strerror(errno));
    goto end;
  }
  PRINTF_QUANTA("Data sent to monikor\n");

end:
  free(data);
  if (sock != -1)
    close(sock);
}

void send_metrics(TSRMLS_D) {
  monikor_metric_list_t *metrics;
  struct timeval now;

  if (!(metrics = monikor_metric_list_new())) {
    PRINTF_QUANTA("Cannot initialize profiling output\n");
    return;
  }
  /* We get the clock from gettimeofday and override only the seconds if quanta_clock is set. this
  ** might be useful later to identify the different requests */
  gettimeofday(&now, NULL);
  if (hp_globals.quanta_clock)
    now.tv_sec = hp_globals.quanta_clock;
  fetch_magento_events(&now, metrics);
  if (hp_globals.profiler_level <= QUANTA_MON_MODE_MAGENTO_PROFILING) {
    if (!hp_globals.cpu_frequencies) {
      PRINTF_QUANTA("CPU frequencies not initialized, cannot profile\n");
    } else {
      float cpufreq = hp_globals.cpu_frequencies[hp_globals.cur_cpu_id];

      fetch_request_uri(&now, metrics);
      fetch_profiler_metrics(&now, metrics, cpufreq);
      fetch_blocks_metrics(&now, metrics, cpufreq TSRMLS_CC);
      fetch_xhprof_metrics(&now, metrics TSRMLS_CC);
    }
  }
  /* We only want to provide context information such as versions when we actually have some metrics
  */
  if (metrics->size) {
    fetch_all_versions(&now, metrics TSRMLS_CC);
    send_data_to_monikor(metrics);
  }
  monikor_metric_list_free(metrics);
}
