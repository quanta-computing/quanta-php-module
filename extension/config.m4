PHP_ARG_ENABLE(quanta_mon, whether to enable quanta monitoring,
[ --enable-quanta      Enable quanta monitoring])

if test "$PHP_QUANTAMON" != "no"; then
  PHP_NEW_EXTENSION(quanta_mon, quanta_mon.c, $ext_shared)
fi
