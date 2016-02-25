#include "quanta_mon.h"

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
  zval func;
  zval retval;
  int ret;

  if (hp_globals.profiler_level > QUANTA_MON_MODE_SAMPLED)
    return;
  INIT_ZVAL(func);
  ZVAL_NULL(&retval);
  ZVAL_STRING(&func, "json_encode", 1);
  ret = call_user_function(CG(function_table), NULL, &func, &retval, 1, &hp_globals.stats_count TSRMLS_CC);
  if (ret != SUCCESS || Z_TYPE(retval) != IS_STRING) {
    PRINTF_QUANTA("Error: cannot json encode xhprof output\n :(");
  } else {
    sprintf(metric_name, "qtracer.%zu.json", hp_globals.quanta_step_id);
    monikor_metric_t *metric = monikor_metric_string(metric_name, clock, Z_STRVAL(retval));
    if (metric)
      monikor_metric_list_push(metrics, metric);
  }
  zval_dtor(&retval);
  zval_dtor(&func);
}

static void fetch_magento_version(struct timeval *clock, monikor_metric_list_t *metrics TSRMLS_DC) {
  char value[512];
  zval func;
  zval version;
  zval edition;
  int ret;

  INIT_ZVAL(func);
  ZVAL_NULL(&version);
  ZVAL_NULL(&edition);
  ZVAL_STRING(&func, "Mage::getVersion", 1);
  ret = call_user_function(CG(function_table), NULL, &func, &version, 0, NULL TSRMLS_CC);
  if (ret != SUCCESS || Z_TYPE(version) != IS_STRING) {
    PRINTF_QUANTA("Cannot get magento version\n");
    goto end;
  }
  ZVAL_STRING(&func, "Mage::getEdition", 1);
  ret = call_user_function(CG(function_table), NULL, &func, &edition, 0, NULL TSRMLS_CC);
  if (ret != SUCCESS || Z_TYPE(edition) != IS_STRING) {
    PRINTF_QUANTA("Cannot get magento edition\n");
    goto end;
  }
  if (snprintf(value, 512, "%s %s", Z_STRVAL(version), Z_STRVAL(edition)) < 512) {
    monikor_metric_t *metric = monikor_metric_string("magento.version.magento", clock, value);
    if (metric)
      monikor_metric_list_push(metrics, metric);
  } else {
    PRINTF_QUANTA("Cannot format magento version\n");
  }

end:
  zval_dtor(&version);
  zval_dtor(&edition);
  zval_dtor(&func);
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

static void fetch_all_versions(struct timeval *clock, monikor_metric_list_t *metrics) {
  fetch_magento_version(clock, metrics);
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

//TODO! remove between on magento loading time
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
  {"between_init_config_init_cache", PROF_STOPS(1), PROF_STARTS(2)},
  {"init_cache", PROF_STARTS(2), PROF_STOPS(2)},
  {"between_init_cache_load_modules", PROF_STOPS(2), PROF_STARTS(3)},
  {"load_modules", PROF_STARTS(3), PROF_STOPS(4)},
  {"between_load_modules_db_updates", PROF_STOPS(4), PROF_STARTS(5)},
  {"db_updates", PROF_STARTS(5), PROF_STOPS(5)},
  {"between_db_updates_load_db", PROF_STOPS(5), PROF_STARTS(6)},
  {"load_db", PROF_STARTS(6), PROF_STOPS(6)},
  {"between_load_db_init_stores", PROF_STOPS(6), PROF_STARTS(7)},
  {"init_stores", PROF_STARTS(7), PROF_STOPS(7)},
  {"routing", PROF_STOPS(7), PROF_STARTS(8)},
  {"before_layout_loading", PROF_STARTS(8), PROF_STARTS(9)},
  {"layout_loading", PROF_STARTS(9), PROF_STOPS(9)},
  {"between_layout_loading_and_rendering", PROF_STOPS(9), PROF_STARTS(10)},
  {"layout_rendering", PROF_STARTS(10), PROF_STOPS(10)},
  {"after_layout_rendering", PROF_STOPS(10), PROF_STOPS(11)},
  {"before_sending_response", PROF_STOPS(11), PROF_STOPS(12)},
  {"total", PROF_STARTS(0), PROF_STOPS(0)},
  {0}
};

static void fetch_profiler_sql_metrics(struct timeval *clock, monikor_metric_list_t *metrics,
float cpufreq, size_t metric_idx, size_t func_idx) {
  char metric_name[MAX_METRIC_NAME_LENGTH];
  char *metric_base_end;
  monikor_metric_t *metric;

  sprintf(metric_name, "magento.%zu.profiling.%s.sql.", hp_globals.quanta_step_id,
    magento_metrics[metric_idx].name);
  metric_base_end = metric_name + strlen(metric_name);
  strcpy(metric_base_end, "count");
  metric = monikor_metric_integer(metric_name, clock,
    hp_globals.monitored_function_sql_queries_count[func_idx], 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
  strcpy(metric_base_end, "time");
  metric = monikor_metric_float(metric_name, clock, cpu_cycles_to_ms(cpufreq,
    hp_globals.monitored_function_sql_cpu_cycles[func_idx]), 0);
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
    float a = (magento_metrics[i].starts_a != -1) ?
      starts[magento_metrics[i].starts_a] : stops[magento_metrics[i].stops_a];
    float b = (magento_metrics[i].starts_b != -1) ?
      starts[magento_metrics[i].starts_b] : stops[magento_metrics[i].stops_b];
    strcpy(metric_base_end, magento_metrics[i].name);
    strcpy(metric_base_end + strlen(magento_metrics[i].name), ".time");
    metric = monikor_metric_float(metric_name, clock, cpu_cycles_range_to_ms(cpufreq, a, b), 0);
    if (metric)
      monikor_metric_list_push(metrics, metric);

    /* We can add SQL timers only if we are profiling only one function in this timer */
    if (magento_metrics[i].starts_a != -1
    && magento_metrics[i].starts_a == magento_metrics[i].stops_b)
      fetch_profiler_sql_metrics(clock, metrics, cpufreq, i, magento_metrics[i].starts_a);
  }
}

static void fetch_block_class_metrics(struct timeval *clock, monikor_metric_list_t *metrics,
magento_block_t *block) {
  char metric_name[MAX_METRIC_NAME_LENGTH];
  char *metric_base_end;
  monikor_metric_t *metric;
  zval func;
  zval retval;
  zval class_name;
  zval *params;
  int ret;

  if (!block->class)
    return;
  sprintf(metric_name, "magento.%zu.blocks.%.255s.", hp_globals.quanta_step_id, block->name);
  metric_base_end = metric_name + strlen(metric_name);
  strcpy(metric_base_end, "class");
  if ((metric = monikor_metric_string(metric_name, clock, block->class)))
    monikor_metric_list_push(metrics, metric);
  INIT_ZVAL(func);
  INIT_ZVAL(class_name);
  ZVAL_NULL(&retval);
  ZVAL_STRING(&func, "mageFindClassFile", 1);
  ZVAL_STRING(&class_name, block->class, 1);
  params = &class_name;
  ret = call_user_function(CG(function_table), NULL, &func, &retval, 1, &params TSRMLS_CC);
  if (ret != SUCCESS || Z_TYPE(retval) != IS_STRING) {
    PRINTF_QUANTA("Error: cannot get class_file for block %s\n", block->name);
  } else {
    strcpy(metric_base_end, "class_file");
    if ((metric = monikor_metric_string(metric_name, clock, Z_STRVAL(retval))))
      monikor_metric_list_push(metrics, metric);
  }
  zval_dtor(&class_name);
  zval_dtor(&retval);
  zval_dtor(&func);
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
magento_block_t *block) {
  monikor_metric_t *metric;
  char metric_name[MAX_METRIC_NAME_LENGTH];
  char *metric_base_end;

  sprintf(metric_name, "magento.%zu.blocks.%.255s.", hp_globals.quanta_step_id, block->name);
  metric_base_end = metric_name + strlen(metric_name);
  fetch_block_class_metrics(clock, metrics, block);
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

static void fetch_blocks_metrics(struct timeval *clock, monikor_metric_list_t *metrics, float cpufreq) {
  magento_block_t *current_block;
  magento_block_t *next_block;

  current_block = hp_globals.magento_blocks_first;
  while (current_block) {
    next_block = current_block->next;
    if (hp_globals.profiler_level == QUANTA_MON_MODE_MAGENTO_PROFILING)
      fetch_block_metrics(clock, metrics, cpufreq, current_block);
    efree(current_block->class);
    efree(current_block->name);
    efree(current_block->template);
    efree(current_block);
    current_block = next_block;
  }
}

static void send_data_to_monikor(monikor_metric_list_t *metrics) {
  void *data = NULL;
  int sock;
  size_t size;
  struct sockaddr_un addr;

  if (!hp_globals.path_quanta_agent_socket) {
    PRINTF_QUANTA("Cannot send data to monikor: socket not configured\n");
    return;
  }

  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, hp_globals.path_quanta_agent_socket);
  if (monikor_metric_list_serialize(metrics, &data, &size)) {
    PRINTF_QUANTA("Cannot send data to monikor: cannot serialize metrics\n");
    free(data);
    return;
  }
  if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1
  || connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1
  || write(sock, data, size) == -1) {
    PRINTF_QUANTA("Cannot send data to monikor: %s\n", strerror(errno));
  }
  free(data);
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
      fetch_blocks_metrics(&now, metrics, cpufreq);
      fetch_xhprof_metrics(&now, metrics TSRMLS_CC);
    }
  }
  /* We only want to provide context information such as versions when we already have some metrics
  */
  if (metrics->size) {
    fetch_all_versions(&now, metrics);
    send_data_to_monikor(metrics);
  }
  monikor_metric_list_free(metrics);
}
