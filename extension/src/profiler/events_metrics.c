#include "quanta_mon.h"

static inline const char *event_class_str(applicative_event_t *event) {
  if (event->class == APP_EV_CACHE_CLEAR)
    return "cache_clear";
  else if (event->class == APP_EV_REINDEX)
    return "reindex";
  else
    return "unknown";
}

void qm_send_events_metrics(struct timeval *clock, monikor_metric_list_t *metrics) {
  char metric_name[MAX_METRIC_NAME_LENGTH];
  monikor_metric_t *metric;
  applicative_event_t *event;
  applicative_event_t *prev;

  event = hp_globals.app_events;
  while (event) {
    prev = event->prev;
    sprintf(metric_name, "%s.events.%s.%s", hp_globals.profiled_application->name,
      event_class_str(event), event->type);
    if ((metric = monikor_metric_string(metric_name, clock, event->subtype)))
      monikor_metric_list_push(metrics, metric);
    efree(event->type);
    efree(event->subtype);
    efree(event);
    event = prev;
  }
}
