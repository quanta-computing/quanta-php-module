PHP_ARG_ENABLE(quanta_mon, whether to enable quanta monitoring,
[ --enable-quanta      Enable quanta monitoring])

if test "$PHP_QUANTAMON" != "no"; then
  LIBMONIKOR=monikor
  LIBMONIKOR_SYM=monikor_metric_list_push

  PHP_ADD_LIBPATH(/monikor/lib/monikor)
  PHP_CHECK_LIBRARY($LIBMONIKOR,$LIBMONIKOR_SYM,
  [
    PHP_ADD_LIBRARY($LIBMONIKOR,,QUANTA_MON_SHARED_LIBADD)
    PHP_ADD_LIBRARY($LIBMONIKOR)
  ],[
    AC_MSG_ERROR([wrong $LIBMONIKOR lib version or lib not found])
  ],[
    -lmonikor
  ])

  PHP_SUBST(QUANTA_MON_SHARED_LIBADD)
  PHP_NEW_EXTENSION(quanta_mon, quanta_mon.c, $ext_shared)
fi
