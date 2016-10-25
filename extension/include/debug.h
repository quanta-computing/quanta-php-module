#ifndef QM_DEBUG_H_
#define QM_DEBUG_H_

/*
** Full debug mode
*/
// #define DEBUG_QUANTA

#ifdef DEBUG_QUANTA
# define PRINTF_QUANTA(...) printf(__VA_ARGS__)
#else
# define PRINTF_QUANTA(...) do {} while (0)
#endif

#include <inttypes.h>

#endif /* end of include guard: QM_DEBUG_H_ */
