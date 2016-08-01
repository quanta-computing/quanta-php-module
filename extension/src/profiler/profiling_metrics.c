#include "quanta_mon.h"

static void qm_send_profiled_function_sql_metrics(char *metric_name, char *metric_base_end,
const profiler_timer_t *timer, struct timeval *clock, monikor_metric_list_t *metrics, float cpufreq) {
  profiled_application_t *app = hp_globals.profiled_application;
  monikor_metric_t *metric;
  uint64_t cycles;
  uint64_t count;
  size_t i;

  if (timer->options.ignore_sql)
    return;
  if (timer->start.counter == PROF_FIRST_STOP
  || timer->start.counter == PROF_LAST_STOP) {
    cycles += timer->start.function->sql_counters.cycles_after;
    count += timer->start.function->sql_counters.count_after;
    i = timer->start.function->index + 1;
  } else {
    cycles = 0;
    count = 0;
    i = timer->start.function->index;
  }
  for (; i < timer->end.function->index; i++) {
    cycles += app->functions[i].sql_counters.cycles + app->functions[i].sql_counters.cycles_after;
    count += app->functions[i].sql_counters.count + app->functions[i].sql_counters.count_after;
  }
  if (timer->end.counter == PROF_FIRST_STOP || PROF_LAST_STOP) {
    cycles += timer->end.function->sql_counters.cycles;
    count += timer->end.function->sql_counters.count;
  }
  strcpy(metric_base_end, "sql.time");
  metric = monikor_metric_float(metric_name, clock, cpu_cycles_to_ms(cpufreq, cycles), 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
  PRINTF_QUANTA("METRIC %s: %f\n", metric_name, cpu_cycles_to_ms(cpufreq, cycles));
  strcpy(metric_base_end, "sql.count");
  metric = monikor_metric_integer(metric_name, clock, count, 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
  PRINTF_QUANTA("METRIC %s: %"PRIu64"\n", metric_name, count);
}

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

static void qm_send_profiled_function_time_metrics(char *metric_name, char *metric_base_end,
const profiler_timer_t *timer, struct timeval *clock, monikor_metric_list_t *metrics, float cpufreq) {
  monikor_metric_t *metric;
  uint64_t start;
  uint64_t end;

  start = fetch_timed_function_counter_value(&timer->start);
  end = fetch_timed_function_counter_value(&timer->end);
  strcpy(metric_base_end, "time");
  metric = monikor_metric_float(metric_name, clock, cpu_cycles_range_to_ms(cpufreq, start, end), 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
  PRINTF_QUANTA("METRIC %s: %f\n", metric_name, cpu_cycles_range_to_ms(cpufreq, start, end));
}

static void qm_send_profiled_functions_metrics(char *metric_name, char *metric_base_end,
struct timeval *clock, monikor_metric_list_t *metrics, float cpufreq) {
  profiled_application_t *app = hp_globals.profiled_application;
  size_t i;

  if (hp_globals.profiler_level != QUANTA_MON_MODE_MAGENTO_PROFILING)
    return;
  for (i = 0; i < app->nb_timers; i++) {
    char *metric_timer_end = metric_base_end + strlen(app->timers[i].name) + 1;
    sprintf(metric_base_end, "%s.", app->timers[i].name);
    qm_send_profiled_function_time_metrics(metric_name, metric_timer_end, &app->timers[i],
      clock, metrics, cpufreq);
    qm_send_profiled_function_sql_metrics(metric_name, metric_timer_end, &app->timers[i],
      clock, metrics, cpufreq);
  }
}

static void qm_send_total_time_metrics(char *metric_name, char *metric_base_end,
struct timeval *clock, monikor_metric_list_t *metrics, float cpufreq) {
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
  qm_send_total_time_metrics(metric_name, metric_base_end, clock, metrics, cpufreq);
}
