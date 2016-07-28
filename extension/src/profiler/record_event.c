#include "quanta_mon.h"

int qm_record_event(applicative_event_class_t class, char *type, char *subtype) {
  applicative_event_t *event;

  if (!(event = ecalloc(1, sizeof(*event))))
    return -1;
  PRINTF_QUANTA("EVENT[%d]: %s %s\n", class, type, subtype);
  event->class = class;
  if (!(event->type = estrdup(type))
  || !(event->subtype = estrdup(subtype))) {
    free(event->type);
    free(event);
    return -1;
  }
  event->prev = hp_globals.app_events;
  hp_globals.app_events = event;
  return 0;
}
