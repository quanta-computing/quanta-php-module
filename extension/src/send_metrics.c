#include "quanta_mon.h"

#define MAX_METRIC_NAME_LENGTH 1024

#define PROF_STARTS(i) i, -1
#define PROF_STOPS(i) -1, i

static const struct {
  char *name;
  int8_t starts_a;
  int8_t stops_a;
  int8_t starts_b;
  int8_t stops_b;
} magento_metrics[] = {
  {"loading_time", PROF_STARTS(0), PROF_STARTS(1)},
  {"before_layout_loading_time", PROF_STARTS(1), PROF_STARTS(2)},
  {"layout_loading_time", PROF_STARTS(2), PROF_STOPS(2)},
  {"between_layout_loading_and_rendering_time", PROF_STOPS(2), PROF_STARTS(3)},
  {"layout_rendering_time", PROF_STARTS(3), PROF_STOPS(3)},
  {"after_layout_rendering_time", PROF_STOPS(3), PROF_STOPS(4)},
  {"before_sending_response_time", PROF_STOPS(4), PROF_STOPS(5)},
  {0}
};

static void fetch_profiler_metrics(struct timeval *clock, monikor_metric_list_t *metrics, float cpufreq) {
  uint64_t *starts = hp_globals.monitored_function_tsc_start;
  uint64_t *stops = hp_globals.monitored_function_tsc_stop;
  char metric_name[MAX_METRIC_NAME_LENGTH];
  char *metric_base_end;
  monikor_metric_t *metric;
  size_t i;

  sprintf(metric_name, "magento.%zu.", hp_globals.quanta_step_id);
  metric_base_end = metric_name + strlen(metric_name);
  for (i = 0; magento_metrics[i].name; i++) {
    float a = (magento_metrics[i].starts_a != -1) ?
      starts[magento_metrics[i].starts_a] : stops[magento_metrics[i].stops_a];
    float b = (magento_metrics[i].starts_b != -1) ?
      starts[magento_metrics[i].starts_b] : stops[magento_metrics[i].stops_b];
    strcpy(metric_base_end, magento_metrics[i].name);
    if ((metric = monikor_metric_float(metric_name, clock, cpu_cycles_to_ms(cpufreq, a, b), 0)))
      monikor_metric_list_push(metrics, metric);
  }
}

static void fetch_block_metrics(struct timeval *clock, monikor_metric_list_t *metrics, float cpufreq,
generate_renderize_block_details *block) {
  monikor_metric_t *metric;
  char metric_name[MAX_METRIC_NAME_LENGTH];
  char *metric_base_end;

  sprintf(metric_name, "magento.blocks.%zu.%.255s.", hp_globals.quanta_step_id, block->name);
  metric_base_end = metric_name + strlen(metric_name);
  strcpy(metric_base_end, "type");
  if (block->type && (metric = monikor_metric_string(metric_name, clock, block->type)))
    monikor_metric_list_push(metrics, metric);
  strcpy(metric_base_end, "class");
  if (block->class && (metric = monikor_metric_string(metric_name, clock, block->class)))
    monikor_metric_list_push(metrics, metric);
  strcpy(metric_base_end, "generation_time");
  metric = monikor_metric_float(metric_name, clock, cpu_cycles_to_ms(cpufreq,
    block->tsc_generate_start, block->tsc_generate_stop), 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
  strcpy(metric_base_end, "rendering_time");
  metric = monikor_metric_float(metric_name, clock, cpu_cycles_to_ms(cpufreq,
    block->tsc_renderize_first_start, block->tsc_renderize_last_stop), 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
  efree(block->name);
  efree(block->type);
  efree(block->template);
}

static void fetch_blocks_metrics(struct timeval *clock, monikor_metric_list_t *metrics, float cpufreq) {
  generate_renderize_block_details *current_block;
  generate_renderize_block_details *next_block;

  current_block = hp_globals.monitored_function_generate_renderize_block_first_linked_list;
  while (current_block) {
    next_block = current_block->next_generate_renderize_block_detail;
    fetch_block_metrics(clock, metrics, cpufreq, current_block);
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

static void fetch_xhprof_metrics(struct timeval *now, monikor_metric_list_t *metrics TSRMLS_DC) {
  zval func;
  zval retval;
  int ret;

  INIT_ZVAL(func);
  ZVAL_NULL(&retval);
  ZVAL_STRING(&func, "json_encode", 1);
  ret = call_user_function(CG(function_table), NULL, &func, &retval, 1, &hp_globals.stats_count TSRMLS_CC);
  if (ret != SUCCESS || Z_TYPE(retval) != IS_STRING) {
    PRINTF_QUANTA("Error: cannot json encode this shit\n :(");
  } else {
    monikor_metric_t *metric = monikor_metric_string("qtracer.json", clock, Z_STRVAL(retval));
    if (metric)
      monikor_metric_list_push(metrics, metric);
  }
  zval_dtor(&retval);
  zval_dtor(&func);
}

void send_metrics(TSRMLS_D) {
  monikor_metric_list_t *metrics;
  struct timeval now;

  if (!hp_globals.cpu_frequencies
  || !(metrics = monikor_metric_list_new())) {
    PRINTF_QUANTA("Cannot initialize profiling output\n");
    return;
  }
  gettimeofday(&now, NULL);
  fetch_profiler_metrics(&now, metrics, hp_globals.cpu_frequencies[hp_globals.cur_cpu_id]);
  fetch_blocks_metrics(&now, metrics, hp_globals.cpu_frequencies[hp_globals.cur_cpu_id]);
  if (hp_globals.profiler_level <= QUANTA_MON_MODE_SAMPLED)
    fetch_xhprof_metrics(&now, metrics TSRMLS_CC);
  send_data_to_monikor(metrics);
  monikor_metric_list_free(metrics);
}
