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
    src/magento_common/block_stack.c \
    src/magento_common/events.c \
    src/magento_common/model_data.c \
    src/magento1/events.c \
    src/magento1/version.c \
    src/magento2/blocks.c \
    src/magento2/events.c \
    src/magento2/version.c \
    src/module/module_info.c \
    src/module/module_init.c \
    src/module/module_shutdown.c \
    src/module/quanta_mon.c \
    src/module/register_constants.c \
    src/module/request_init.c \
    src/module/request_shutdown.c \
    src/profiler/application.c \
    src/profiler/begin_profiling.c \
    src/profiler/common_callbacks.c \
    src/profiler/dummy_callbacks.c \
    src/profiler/end_profiling.c \
    src/profiler/function_stack.c \
    src/profiler/hier_callbacks.c \
    src/profiler/hp_begin.c \
    src/profiler/hp_end.c \
    src/profiler/hp_list.c \
    src/profiler/profiler_clean.c \
    src/profiler/profiler_init.c \
    src/profiler/quanta.c \
    src/profiler/record_event.c \
    src/profiler/send_metrics.c \
    src/profiler/zend_execute.c \
    src/profiler/zend_functions_info.c \
    src/utils/cpu.c \
    src/utils/filename.c \
    src/utils/hash.c \
    src/utils/profiler.c \
    src/utils/safe_call_function.c \
    src/utils/safe_get_argument.c \
    src/utils/safe_get_constant.c \
    src/utils/this.c \
    src/utils/timing.c \
    src/utils/zend_hash.c \
    src/utils/zend_obj.c \
    src/utils/zend_zval.c \
    src/computed_functions_filter.c \
    ,
    $ext_shared,,-W -Wall -Wextra -Wno-unused-parameter -D_GNU_SOURCE)
fi
