#include "quanta_mon.h"

int qm_record_event(uint8_t class, char *type, char *subtype) {
  magento_event_t *event;

  if (!(event = ecalloc(1, sizeof(*event))))
    return -1;
  PRINTF_QUANTA("EVENT[%hhd]: %s %s\n", class, type, subtype);
  event->class = class;
  if (!(event->type = estrdup(type))
  || !(event->subtype = estrdup(subtype))) {
    free(event->type);
    free(event);
    return -1;
  }
  event->prev = hp_globals.magento_events;
  hp_globals.magento_events = event;
  return 0;
}
