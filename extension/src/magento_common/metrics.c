#include "quanta_mon.h"

static void fetch_block_class_metric(char *metric_name, char *metric_base_end,
struct timeval *clock, monikor_metric_list_t *metrics, magento_block_t *block) {
  monikor_metric_t *metric;

  if (!block->class)
    return;
  strcpy(metric_base_end, "class");
  if ((metric = monikor_metric_string(metric_name, clock, block->class)))
    monikor_metric_list_push(metrics, metric);
  if (block->class_file) {
    strcpy(metric_base_end, "class_file");
    if ((metric = monikor_metric_string(metric_name, clock, block->class_file)))
      monikor_metric_list_push(metrics, metric);
  }
}

static void fetch_block_template_metrics(char *metric_name, char *metric_base_end,
struct timeval *clock, monikor_metric_list_t *metrics, magento_block_t *block) {
  monikor_metric_t *metric;

  if (!block->template)
    return;
  strcpy(metric_base_end, "template");
  if ((metric = monikor_metric_string(metric_name, clock, block->template)))
    monikor_metric_list_push(metrics, metric);
}

static void fetch_block_sql_metrics(char *metric_name, char *metric_base_end,
struct timeval *clock, monikor_metric_list_t *metrics,
float cpufreq, magento_block_t *block) {
  monikor_metric_t *metric;

  strcpy(metric_base_end, "sql_count");
  if ((metric = monikor_metric_integer(metric_name, clock, block->sql_queries_count, 0)))
    monikor_metric_list_push(metrics, metric);
  strcpy(metric_base_end, "sql_time");
  metric = monikor_metric_float(metric_name, clock, cpu_cycles_to_ms(
    cpufreq, block->sql_cpu_cycles), 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
}

static void fetch_block_rendering_metrics(char *metric_name, char *metric_base_end,
struct timeval *clock, monikor_metric_list_t *metrics, float cpufreq, magento_block_t *block) {
  monikor_metric_t *metric;
  float rendering_time;

  strcpy(metric_base_end, "rendering_time");
  rendering_time = cpu_cycles_range_to_ms(cpufreq,
    block->renderize_children_cycles + block->tsc_renderize_first_start,
    block->tsc_renderize_last_stop
  );
  PRINTF_QUANTA("BLOCK %s rendered in %f\n  - class: %s (%s)\n  - template: %s\n  - SQL: %zu (%fms)\n",
    block->name, rendering_time,
    block->class, block->class_file,
    block->template,
    block->sql_queries_count, cpu_cycles_to_ms(cpufreq, block->sql_cpu_cycles)
  );
  metric = monikor_metric_float(metric_name, clock, rendering_time, 0);
  if (metric)
    monikor_metric_list_push(metrics, metric);
}

static void fetch_block_metrics(profiled_application_t *app, struct timeval *clock,
monikor_metric_list_t *metrics, float cpufreq, magento_block_t *block) {
  char metric_name[MAX_METRIC_NAME_LENGTH];
  char *metric_base_end;

  sprintf(metric_name, "%s.%zu.blocks.%.255s.", app->name, hp_globals.quanta_step_id, block->name);
  metric_base_end = metric_name + strlen(metric_name);
  fetch_block_class_metric(metric_name, metric_base_end, clock, metrics, block);
  fetch_block_template_metrics(metric_name, metric_base_end, clock, metrics, block);
  fetch_block_sql_metrics(metric_name, metric_base_end, clock, metrics, cpufreq, block);
  fetch_block_rendering_metrics(metric_name, metric_base_end, clock, metrics, cpufreq, block);
}

static void push_blocks_metrics(profiled_application_t *app, struct timeval *clock,
monikor_metric_list_t *metrics, float cpufreq TSRMLS_DC) {
  magento_context_t *context = (magento_context_t *)app->context;
  magento_block_t *current_block;
  magento_block_t *next_block;

  current_block = context->blocks.first;
  while (current_block) {
    next_block = current_block->next;
    fetch_block_metrics(app, clock, metrics, cpufreq, current_block);
    efree(current_block->class);
    efree(current_block->class_file);
    efree(current_block->name);
    efree(current_block->template);
    efree(current_block);
    current_block = next_block;
  }
}

static void push_magento_version_metric(profiled_application_t *app, struct timeval *clock,
monikor_metric_list_t *metrics) {
  magento_context_t *context = (magento_context_t *)app->context;
  char value[512];
  int ret;

  if (!context->version)
    return;
  ret = snprintf(value, 512, "%s %s", context->version,
    context->edition ? context->edition : "unknown");
  if (ret < 512) {
    monikor_metric_t *metric = monikor_metric_string("app.version.app", clock, value);
    if (metric) {
      monikor_metric_list_push(metrics, metric);
      PRINTF_QUANTA("Magento version %s\n", value);
    }
  } else {
    PRINTF_QUANTA("Cannot format magento version\n");
  }
}

void magento_send_metrics(profiled_application_t *app, monikor_metric_list_t *metrics,
float cpufreq, struct timeval *clock TSRMLS_DC) {
  push_blocks_metrics(app, clock, metrics, cpufreq);
  push_magento_version_metric(app, clock, metrics);
}
