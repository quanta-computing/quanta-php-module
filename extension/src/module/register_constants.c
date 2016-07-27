#include "quanta_mon.h"

void hp_register_constants(INIT_FUNC_ARGS) {
  (void)type;
  (void)module_number;
  REGISTER_LONG_CONSTANT("QUANTA_MON_FLAGS_NO_BUILTINS",
                         QUANTA_MON_FLAGS_NO_BUILTINS,
                         CONST_CS | CONST_PERSISTENT);

  REGISTER_LONG_CONSTANT("QUANTA_MON_FLAGS_CPU",
                         QUANTA_MON_FLAGS_CPU,
                         CONST_CS | CONST_PERSISTENT);

  REGISTER_LONG_CONSTANT("QUANTA_MON_FLAGS_MEMORY",
                         QUANTA_MON_FLAGS_MEMORY,
                         CONST_CS | CONST_PERSISTENT);
}
