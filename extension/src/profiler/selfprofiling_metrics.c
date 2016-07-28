#include "quanta_mon.h"

void qm_send_selfprofiling_metrics(struct timeval *clock, monikor_metric_list_t *metrics,
float cpufreq TSRMLS_DC) {
  char metric_name[MAX_METRIC_NAME_LENGTH];
  char *metric_base_end;
  monikor_metric_t *metric;

  sprintf(metric_name, "magento2.%zu.selfprofiling.", hp_globals.quanta_step_id);
  metric_base_end = metric_name + strlen(metric_name);
  strcpy(metric_base_end, "init_time");
  metric = monikor_metric_float(metric_name, clock,
    cpu_cycles_to_ms(cpufreq, hp_globals.internal_match_counters.init_cycles), 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
  strcpy(metric_base_end, "profiling_time");
  metric = monikor_metric_float(metric_name, clock,
    cpu_cycles_to_ms(cpufreq, hp_globals.internal_match_counters.profiling_cycles), 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
  hp_globals.internal_match_counters.shutdown_cycles = cycle_timer()
    - hp_globals.internal_match_counters.shutdown_cycles;
  strcpy(metric_base_end, "shutdown_time");
  metric = monikor_metric_float(metric_name, clock,
    cpu_cycles_to_ms(cpufreq, hp_globals.internal_match_counters.shutdown_cycles), 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
  strcpy(metric_base_end, "match_time");
  metric = monikor_metric_float(metric_name, clock,
    cpu_cycles_to_ms(cpufreq, hp_globals.internal_match_counters.cycles), 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
  strcpy(metric_base_end, "match_total_count");
  metric = monikor_metric_integer(metric_name, clock,
    hp_globals.internal_match_counters.total, 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
  strcpy(metric_base_end, "match_function_count");
  metric = monikor_metric_integer(metric_name, clock,
    hp_globals.internal_match_counters.function, 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
  strcpy(metric_base_end, "match_full_count");
  metric = monikor_metric_float(metric_name, clock, hp_globals.internal_match_counters.function
    - hp_globals.internal_match_counters.class_unmatched, 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
}
