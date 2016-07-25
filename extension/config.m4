PHP_ARG_ENABLE(quanta_mon, whether to enable quanta monitoring,
[ --enable-quanta      Enable quanta monitoring])

if test "$PHP_QUANTAMON" != "no"; then
  LIBMONIKOR=monikor
  LIBMONIKOR_SYM=monikor_metric_list_push

  PHP_CHECK_LIBRARY($LIBMONIKOR,$LIBMONIKOR_SYM,
  [
    PHP_ADD_LIBRARY($LIBMONIKOR,,QUANTA_MON_SHARED_LIBADD)
    PHP_ADD_LIBRARY($LIBMONIKOR)
  ],[
    AC_MSG_ERROR([wrong $LIBMONIKOR lib version or lib not found])
  ],[
    -lmonikor
  ])

  PHP_ADD_INCLUDE(include/)

  PHP_SUBST(QUANTA_MON_SHARED_LIBADD)
  PHP_NEW_EXTENSION(quanta_mon,
    src/block_stack.c \
    src/call_function_helpers.c \
    src/computed_functions_filter.c \
    src/constants.c \
    src/cpu.c \
    src/functions_filter.c \
    src/hp_begin.c \
    src/hp_end.c \
    src/hp_list.c \
    src/mage.c \
    src/module_info.c \
    src/module_init.c \
    src/module_shutdown.c \
    src/profiler_callbacks.c \
    src/profiler.c \
    src/quanta_blocks.c \
    src/quanta_events.c \
    src/quanta_mon.c \
    src/quanta.c \
    src/request_init.c \
    src/request_shutdown.c \
    src/send_metrics.c \
    src/utils.c \
    src/zend_hash.c \
    src/zend_zval.c \
    src/zend.c, $ext_shared,,-W -Wall -Wextra -Wno-unused-parameter -D_GNU_SOURCE)
fi
