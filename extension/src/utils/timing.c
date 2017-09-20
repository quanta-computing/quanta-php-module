#include "quanta_mon.h"

// Returns the diff in us from request start
inline uint64_t cycle_timer() {
  struct timespec val;
  clock_gettime(CLOCK_MONOTONIC_COARSE, &val);
  return (val.tv_sec - hp_globals.gettime_start.tv_sec) * 1000000
    + (val.tv_nsec - hp_globals.gettime_start.tv_nsec) / 1000 + 1;
}

// float cpu_cycles_to_ms(float cpufreq, uint64_t count) {
//   return count / (cpufreq * 1000000) * 1000;
// }

float timer_to_ms(uint64_t timer) {
  return timer / 1000.0;
}

float timers_range_to_ms(uint64_t start, uint64_t end) {
  if (!start && !end)
     return -1.0;
  if (!start)
    return -2.0;
  if (!end)
    return -3.0;
  if (start > end)
    return -4.0;
  return (end - start) / 1000.0;
}


// TODO!
inline double get_us_from_tsc(uint64_t count, double cpu_frequency) {
  return (double)count;
}

long get_us_interval(struct timeval *start, struct timeval *end) {
  return (((end->tv_sec - start->tv_sec) * 1000000) + (end->tv_usec - start->tv_usec));
}
