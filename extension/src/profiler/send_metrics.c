#include "quanta_mon.h"

/* The metric request_uri should be duplicated for each Monikor Handler which needs to check against
** the original step URL.
*/
static void fetch_request_uri(struct timeval *clock, monikor_metric_list_t *metrics) {
  profiled_application_t *app = hp_globals.profiled_application;
  monikor_metric_t *metric;
  char metric_name[MAX_METRIC_NAME_LENGTH];

  if (!hp_globals.request_uri)
    return;
  if (hp_globals.profiler_level == QUANTA_MON_MODE_APP_PROFILING) {
    sprintf(metric_name, "%s.%zu.request_uri",
      app ? app->name : "php", hp_globals.quanta_step_id);
  } else if (hp_globals.profiler_level == QUANTA_MON_MODE_HIERARCHICAL) {
    sprintf(metric_name, "qtracer.%zu.request_uri", hp_globals.quanta_step_id);
  } else {
    PRINTF_QUANTA("Cannot fetch request_uri\n");
    return;
  }
  metric = monikor_metric_string(metric_name, clock, hp_globals.request_uri);
  if (metric)
    monikor_metric_list_push(metrics, metric);
}

static void fetch_xhprof_metrics(struct timeval *clock, monikor_metric_list_t *metrics TSRMLS_DC) {
  char metric_name[MAX_METRIC_NAME_LENGTH];
  zval encoded;

  if (hp_globals.profiler_level != QUANTA_MON_MODE_HIERARCHICAL)
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

static void fetch_php_version(struct timeval *clock, monikor_metric_list_t *metrics) {
  monikor_metric_t *metric;

  metric = monikor_metric_string("app.version.php", clock, PHP_VERSION);
  if (metric)
    monikor_metric_list_push(metrics, metric);
}

static void fetch_module_version(struct timeval *clock, monikor_metric_list_t *metrics) {
  monikor_metric_t *metric;

  metric = monikor_metric_string("app.version.module", clock, QUANTA_MON_VERSION);
  if (metric)
    monikor_metric_list_push(metrics, metric);
}

#ifdef DEBUG_QUANTA
static void dump_metric(monikor_metric_t *metric) {
  switch (metric->type) {
  case MONIKOR_INTEGER:
    PRINTF_QUANTA("METRIC INT %s@%ld: %"PRIu64"\n",
      metric->name, metric->clock.tv_sec, metric->value._int);
    break;
  case MONIKOR_FLOAT:
    PRINTF_QUANTA("METRIC FLOAT %s@%ld: %f\n",
      metric->name, metric->clock.tv_sec, metric->value._float);
    break;
  case MONIKOR_STRING:
    PRINTF_QUANTA("METRIC STRING %s@%ld: %s\n",
      metric->name, metric->clock.tv_sec, metric->value._string);
    break;
  default:
    PRINTF_QUANTA("UNKNOWN METRIC %s@%ld\n",
      metric->name, metric->clock.tv_sec);
    break;
  }
}
#endif

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

  #ifdef DEBUG_QUANTA
    monikor_metric_list_apply(metrics, dump_metric);
  #endif
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
  qm_send_events_metrics(&now, metrics);
  if (hp_globals.profiler_level <= QUANTA_MON_MODE_APP_PROFILING) {
  // && hp_globals.cpu_frequencies) {
    // float cpufreq = hp_globals.cpu_frequencies[hp_globals.cur_cpu_id];
    float cpufreq = 0.0; //TODO!
    fetch_request_uri(&now, metrics);
    fetch_xhprof_metrics(&now, metrics TSRMLS_CC);
    qm_send_profiler_metrics(&now, metrics, cpufreq TSRMLS_CC);
    qm_send_selfprofiling_metrics(&now, metrics, cpufreq TSRMLS_CC);
    if (hp_globals.profiled_application && hp_globals.profiled_application->send_metrics) {
      hp_globals.profiled_application->send_metrics(hp_globals.profiled_application, metrics,
        cpufreq, &now TSRMLS_CC);
    }
  }
  /* We only want to provide context information such as versions when we actually have some metrics
  */
  if (metrics->size) {
    fetch_php_version(&now, metrics);
    fetch_module_version(&now, metrics);
    send_data_to_monikor(metrics);
  }
  monikor_metric_list_free(metrics);
}
