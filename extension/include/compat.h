#ifndef QUANTA_MON_COMPAT_H_
#define QUANTA_MON_COMPAT_H_

#if PHP_MAJOR_VERSION < 7
  #define ZVAL_STRING_COMPAT(z, s) ZVAL_STRING(z, s, 1)
#else
  #define ZVAL_STRING_COMPAT(z, s) ZVAL_STRING(z, s)
#endif

#endif /* end of include guard: QUANTA_MON_COMPAT_H_ */
