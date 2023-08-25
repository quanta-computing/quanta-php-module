#include "quanta_mon.h"

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

static void qm_send_profiled_function_sql_metrics(char *metric_name, char *metric_base_end,
const profiler_timer_t *timer, uint64_t tsc_start, uint64_t tsc_end, struct timeval *clock,
monikor_metric_list_t *metrics, float cpufreq) {
  profiled_application_t *app = hp_globals.profiled_application;
  uint64_t cycles = 0;
  uint64_t count = 0;
  monikor_metric_t *metric;
  sql_query_record_t *query;

  if (timer->options.ignore_sql)
    return;
  if (tsc_start && tsc_end) {
    // Could be optimized with a binary search, a nice implementation might use a skip-list
    for (query = app->sql_queries.first; query; query = query->next) {
      if (query->tsc_start > tsc_start && query->tsc_start < tsc_end) {
        count++;
        cycles += query->tsc_stop - query->tsc_start;
      }
    }
  } else {
    PRINTF_QUANTA("NO SQL FOR TIMER %s\n", timer->name);
  }
  strcpy(metric_base_end, "sql.time");
  metric = monikor_metric_float(metric_name, clock, cpu_cycles_to_ms(cpufreq, cycles), 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
  strcpy(metric_base_end, "sql.count");
  metric = monikor_metric_integer(metric_name, clock, count, 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
}

static int qm_send_profiled_function_time_metrics(char *metric_name, char *metric_base_end,
const profiler_timer_t *timer, uint64_t tsc_start, uint64_t tsc_end, struct timeval *clock,
monikor_metric_list_t *metrics, float cpufreq) {
  monikor_metric_t *metric;

  strcpy(metric_base_end, "time");
  metric = monikor_metric_float(metric_name, clock,
    cpu_cycles_range_to_ms(cpufreq, tsc_start, tsc_end), 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
  if (!tsc_end || !tsc_start || tsc_start > tsc_end)
    return -1;
  return 0;
}

static void qm_send_profiled_functions_metric(char *metric_name, char *metric_base_end,
const profiler_timer_t *timer, struct timeval *clock,
monikor_metric_list_t *metrics, float cpufreq) {
  uint64_t start = 0;
  uint64_t end = 0;
  size_t i;
  int ret;

  for (i = 0; !start && i < timer->nb_start; i++)
    start = fetch_timed_function_counter_value(&timer->start[i]);
  for (i = 0; !end && i < timer->nb_end; i++)
    end = fetch_timed_function_counter_value(&timer->end[i]);
  ret = qm_send_profiled_function_time_metrics(metric_name, metric_base_end,
    timer, start, end, clock, metrics, cpufreq);
  if (ret)
    return;
  qm_send_profiled_function_sql_metrics(metric_name, metric_base_end, timer,
    start, end, clock, metrics, cpufreq);
}

static void qm_send_profiled_functions_metrics(char *metric_name, char *metric_base_end,
struct timeval *clock, monikor_metric_list_t *metrics, float cpufreq) {
  profiled_application_t *app = hp_globals.profiled_application;
  size_t i;

  if (hp_globals.profiler_level != QUANTA_MON_MODE_APP_PROFILING || !app)
    return;
  for (i = 0; i < app->nb_timers; i++) {
    sprintf(metric_base_end, "%s.", app->timers[i].name);
    qm_send_profiled_functions_metric(metric_name,
      metric_base_end + strlen(app->timers[i].name) + 1,
      &app->timers[i], clock, metrics, cpufreq
    );
  }
}

static void qm_send_total_time_metrics(char *metric_name, char *metric_base_end,
struct timeval *clock, monikor_metric_list_t *metrics, float cpufreq) {
  monikor_metric_t *metric;

  strcpy(metric_base_end, "php_total.time");
  metric = monikor_metric_float(metric_name, clock, cpu_cycles_range_to_ms(cpufreq,
    hp_globals.global_tsc.start, hp_globals.global_tsc.stop
  ), 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
}

static void qm_send_before_after_app_metrics(char *metric_name, char *metric_base_end,
struct timeval *clock, monikor_metric_list_t *metrics, float cpufreq) {
  profiled_application_t *app = hp_globals.profiled_application;
  monikor_metric_t *metric;

  if (!app)
    return;
  strcpy(metric_base_end, "before_app.time");
  metric = monikor_metric_float(metric_name, clock, cpu_cycles_range_to_ms(cpufreq,
    hp_globals.global_tsc.start, app->first_app_function->tsc.first_start
  ), 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
  strcpy(metric_base_end, "after_app.time");
  metric = monikor_metric_float(metric_name, clock, cpu_cycles_range_to_ms(cpufreq,
    app->last_app_function->tsc.last_stop, hp_globals.global_tsc.stop
  ), 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
}

void qm_send_profiler_metrics(struct timeval *clock, monikor_metric_list_t *metrics,
float cpufreq) {
  profiled_application_t *app = hp_globals.profiled_application;
  char metric_name[MAX_METRIC_NAME_LENGTH];
  char *metric_base_end;

  if (hp_globals.profiler_level != QUANTA_MON_MODE_APP_PROFILING)
    return;
  sprintf(metric_name, "%s.%zu.profiling.", app ? app->name : "php", hp_globals.quanta_step_id);
  metric_base_end = metric_name + strlen(metric_name);
  qm_send_profiled_functions_metrics(metric_name, metric_base_end, clock, metrics, cpufreq);
  qm_send_total_time_metrics(metric_name, metric_base_end, clock, metrics, cpufreq);
  qm_send_before_after_app_metrics(metric_name, metric_base_end, clock, metrics, cpufreq);
}
