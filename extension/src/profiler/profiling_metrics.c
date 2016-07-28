#include "quanta_mon.h"

// static void fetch_profiler_sql_metrics(struct timeval *clock, monikor_metric_list_t *metrics,
// float cpufreq, size_t metric_idx) {
//   char metric_name[MAX_METRIC_NAME_LENGTH];
//   char *metric_base_end;
//   monikor_metric_t *metric;
//   uint64_t sql_cycles;
//   uint64_t sql_count;
//   size_t start;
//   size_t stop;
//   size_t i;
//
//   // Ignore if we have no Magento time
//   if (hp_globals.monitored_function_tsc_start[0] <= 0)
//     return;
//   start = magento_metrics[metric_idx].starts_a == -1 ?
//     magento_metrics[metric_idx].stops_a + 1 : magento_metrics[metric_idx].starts_a;
//   stop = magento_metrics[metric_idx].starts_b == -1 ?
//     magento_metrics[metric_idx].stops_b + 1: magento_metrics[metric_idx].starts_b;
//   /* Mage::run has all queries, so ignore it unless we are counting queries for the total */
//   if (!start && strcmp(magento_metrics[metric_idx].name, "total"))
//     start = 1;
//   if (magento_metrics[metric_idx].starts_a == -1) {
//     sql_cycles = hp_globals.monitored_function_sql_cpu_cycles_after[magento_metrics[metric_idx].stops_a];
//     sql_count = hp_globals.monitored_function_sql_queries_count_after[magento_metrics[metric_idx].stops_a];
//   } else {
//     sql_cycles = 0;
//     sql_count = 0;
//   }
//   for (i = start; i < stop; i++) {
//     sql_cycles += hp_globals.monitored_function_sql_cpu_cycles[i];
//     sql_count += hp_globals.monitored_function_sql_queries_count[i];
//     if (i != stop -1 || magento_metrics[metric_idx].starts_b != -1) {
//       sql_cycles += hp_globals.monitored_function_sql_cpu_cycles_after[i];
//       sql_count += hp_globals.monitored_function_sql_queries_count_after[i];
//     }
//   }
//   sprintf(metric_name, "magento2.%zu.profiling.%s.sql.", hp_globals.quanta_step_id,
//     magento_metrics[metric_idx].name);
//   metric_base_end = metric_name + strlen(metric_name);
//   strcpy(metric_base_end, "count");
//   metric = monikor_metric_integer(metric_name, clock, sql_count, 0);
//   if (metric)
//     monikor_metric_list_push(metrics, metric);
//   strcpy(metric_base_end, "time");
//   metric = monikor_metric_float(metric_name, clock, cpu_cycles_to_ms(cpufreq, sql_cycles), 0);
//   if (metric)
//     monikor_metric_list_push(metrics, metric);
// }

static inline uint64_t fetch_timed_function_counter_value(const profiler_timer_function_t *f) {
  switch (f->counter) {
    case PROF_FIRST_START:
      return f->function->tsc.first_start;
    case PROF_LAST_START:
      return f->function->tsc.last_start;
    case PROF_FIRST_STOP:
      return f->function->tsc.first_stop;
    case PROF_LAST_STOP:
      return f->function->tsc.last_stop;
    default:
      return 0;
  }
}

static void qm_send_profiled_functions_metrics(char *metric_name, char *metric_base_end,
struct timeval *clock, monikor_metric_list_t *metrics, float cpufreq) {
  profiled_application_t *app = hp_globals.profiled_application;
  monikor_metric_t *metric;
  size_t i;

  if (hp_globals.profiler_level != QUANTA_MON_MODE_MAGENTO_PROFILING)
    return;
  for (i = 0; i < app->nb_timers; i++) {
    uint64_t a = fetch_timed_function_counter_value(&app->timers[i].start);
    uint64_t b = fetch_timed_function_counter_value(&app->timers[i].end);
    strcpy(metric_base_end, app->timers[i].name);
    strcpy(metric_base_end + strlen(app->timers[i].name), ".time");
    PRINTF_QUANTA("METRIC %s: a: %"PRIu64" b: %"PRIu64" = %f ms\n", metric_name, a, b,
      cpu_cycles_range_to_ms(cpufreq, a, b));
    metric = monikor_metric_float(metric_name, clock, cpu_cycles_range_to_ms(cpufreq, a, b), 0);
    if (metric) {
      monikor_metric_list_push(metrics, metric);
      // fetch_profiler_sql_metrics(clock, metrics, cpufreq, i);
    }
  }
}

static void qm_send_total_metrics(char *metric_name, char *metric_base_end,
struct timeval *clock, monikor_metric_list_t *metrics,
float cpufreq) {
  profiled_application_t *app = hp_globals.profiled_application;
  monikor_metric_t *metric;

  strcpy(metric_base_end, "php_total.time");
  metric = monikor_metric_float(metric_name, clock, cpu_cycles_range_to_ms(cpufreq,
    hp_globals.global_tsc.start, hp_globals.global_tsc.stop
  ), 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
  strcpy(metric_base_end, "before_magento.time");
  metric = monikor_metric_float(metric_name, clock, cpu_cycles_range_to_ms(cpufreq,
    hp_globals.global_tsc.start, app->first_app_function->tsc.first_start
  ), 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
  strcpy(metric_base_end, "after_magento.time");
  metric = monikor_metric_float(metric_name, clock, cpu_cycles_range_to_ms(cpufreq,
    app->last_app_function->tsc.last_stop, hp_globals.global_tsc.stop
  ), 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
}

void qm_send_profiler_metrics(struct timeval *clock, monikor_metric_list_t *metrics,
float cpufreq TSRMLS_DC) {
  profiled_application_t *app = hp_globals.profiled_application;
  char metric_name[MAX_METRIC_NAME_LENGTH];
  char *metric_base_end;

  sprintf(metric_name, "%s.%zu.profiling.", app->name, hp_globals.quanta_step_id);
  metric_base_end = metric_name + strlen(metric_name);
  qm_send_profiled_functions_metrics(metric_name, metric_base_end, clock, metrics, cpufreq);
  qm_send_total_metrics(metric_name, metric_base_end, clock, metrics, cpufreq);
}
