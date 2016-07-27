#include "quanta_mon.h"

/**
 * Get time stamp counter (TSC) value via 'rdtsc' instruction.
 *
 * @return 64 bit unsigned integer
 * @author cjiang
 */
inline uint64_t cycle_timer() {
  uint32_t __a,__d;
  uint64_t val;
  asm volatile("rdtsc" : "=a" (__a), "=d" (__d));
  (val) = ((uint64_t)__a) | (((uint64_t)__d)<<32);
  return val;
}

float cpu_cycles_to_ms(float cpufreq, uint64_t count) {
  return count / (cpufreq * 1000000) * 1000;
}


float cpu_cycles_range_to_ms(float cpufreq, long long start, long long end) {
  if (!start && !end)
     return -1.0;
  if (!start)
    return -2.0;
  if (!end)
    return -3.0;
  if (start > end)
    return -4.0;
  return cpu_cycles_to_ms(cpufreq, end - start);
}

inline double get_us_from_tsc(uint64_t count, double cpu_frequency) {
  return count / cpu_frequency;
}

long get_us_interval(struct timeval *start, struct timeval *end) {
  return (((end->tv_sec - start->tv_sec) * 1000000) + (end->tv_usec - start->tv_usec));
}
