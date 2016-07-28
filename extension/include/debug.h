#ifndef QM_DEBUG_H_
#define QM_DEBUG_H_

/*
** Full debug mode
*/
// #define DEBUG_QUANTA

#ifdef DEBUG_QUANTA
# define PRINTF_QUANTA(...) dprintf(1, __VA_ARGS__)
#else
# define PRINTF_QUANTA(...) do {} while (0)
#endif

#endif /* end of include guard: QM_DEBUG_H_ */
