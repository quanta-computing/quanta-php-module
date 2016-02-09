#include "quanta_mon.h"


/**
 * Bind the current process to a specified CPU. This function is to ensure that
 * the OS won't schedule the process to different processors, which would make
 * values read by rdtsc unreliable.
 *
 * @param uint32 cpu_id, the id of the logical cpu to be bound to.
 * @return int, 0 on success, and -1 on failure.
 *
 * @author cjiang
 */
int bind_to_cpu(uint32_t cpu_id) {
  cpu_set_t new_mask;

  CPU_ZERO(&new_mask);
  CPU_SET(cpu_id, &new_mask);

  if (SET_AFFINITY(0, sizeof(cpu_set_t), &new_mask) < 0) {
    perror("setaffinity");
    return -1;
  }

  /* record the cpu_id the process is bound to. */
  hp_globals.cur_cpu_id = cpu_id;

  return 0;
}

/**
 * This is a microbenchmark to get cpu frequency the process is running on. The
 * returned value is used to convert TSC counter values to microseconds.
 *
 * @return double.
 * @author cjiang
 */
double get_cpu_frequency() {
  struct timeval start;
  struct timeval end;

  if (gettimeofday(&start, 0)) {
    perror("gettimeofday");
    return 0.0;
  }
  uint64_t tsc_start = cycle_timer();
  /* Sleep for 5 miliseconds. Comparaing with gettimeofday's  few microseconds
   * execution time, this should be enough. */
  usleep(5000);
  if (gettimeofday(&end, 0)) {
    perror("gettimeofday");
    return 0.0;
  }
  uint64_t tsc_end = cycle_timer();
  return (tsc_end - tsc_start) * 1.0 / (get_us_interval(&start, &end));
}

/**
 * Reclaim the memory allocated for cpu_frequencies.
 *
 * @author cjiang
 */
void clear_frequencies() {
  if (hp_globals.cpu_frequencies) {
    free(hp_globals.cpu_frequencies);
    hp_globals.cpu_frequencies = NULL;
  }
  restore_cpu_affinity(&hp_globals.prev_mask);
}

/**
 * Restore cpu affinity mask to a specified value. It returns 0 on success and
 * -1 on failure.
 *
 * @param cpu_set_t * prev_mask, previous cpu affinity mask to be restored to.
 * @return int, 0 on success, and -1 on failure.
 *
 * @author cjiang
 */
int restore_cpu_affinity(cpu_set_t * prev_mask) {
  if (SET_AFFINITY(0, sizeof(cpu_set_t), prev_mask) < 0) {
    perror("restore setaffinity");
    return -1;
  }
  /* default value ofor cur_cpu_id is 0. */
  hp_globals.cur_cpu_id = 0;
  return 0;
}

/**
 * Calculate frequencies for all available cpus.
 *
 * @author cjiang
 */
void get_all_cpu_frequencies() {
  uint32_t id;
  double frequency;

  hp_globals.cpu_frequencies = malloc(sizeof(double) * hp_globals.cpu_num);
  if (hp_globals.cpu_frequencies == NULL) {
    return;
  }

  /* Iterate over all cpus found on the machine. */
  for (id = 0; id < hp_globals.cpu_num; ++id) {
    /* Only get the previous cpu affinity mask for the first call. */
    if (bind_to_cpu(id)) {
      clear_frequencies();
      return;
    }

    /* Make sure the current process gets scheduled to the target cpu. This
     * might not be necessary though. */
    usleep(0);

    frequency = get_cpu_frequency();
    if (frequency == 0.0) {
      clear_frequencies();
      return;
    }
    hp_globals.cpu_frequencies[id] = frequency;
  }
}


float cpu_cycles_to_ms(float cpufreq, uint64_t count) {
  return count / (cpufreq * 1000000) * 1000;
}


float cpu_cycles_range_to_ms(float cpufreq, long long start, long long end) {
  // TODO check if it doesnt exceed long max value
  if (!start && !end)
     return -1.0;
  if (!start)
    return -2.0;
  if (!end)
    return -3.0;
  return cpu_cycles_to_ms(cpufreq, end - start);
}

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

/**
 * Convert from TSC counter values to equivalent microseconds.
 *
 * @param uint64 count, TSC count value
 * @param double cpu_frequency, the CPU clock rate (MHz)
 * @return 64 bit unsigned integer
 *
 * @author cjiang
 */
inline double get_us_from_tsc(uint64_t count, double cpu_frequency) {
  return count / cpu_frequency;
}

/**
 * Convert microseconds to equivalent TSC counter ticks
 *
 * @param uint64 microseconds
 * @param double cpu_frequency, the CPU clock rate (MHz)
 * @return 64 bit unsigned integer
 *
 * @author veeve
 */
inline uint64_t get_tsc_from_us(uint64_t usecs, double cpu_frequency) {
  return (uint64_t) (usecs * cpu_frequency);
}


/**
 * ***********************
 * High precision timer related functions.
 * ***********************
 */

/**
 * Get time delta in microseconds.
 */
long get_us_interval(struct timeval *start, struct timeval *end) {
  return (((end->tv_sec - start->tv_sec) * 1000000) + (end->tv_usec - start->tv_usec));
}

/**
 * Incr time with the given microseconds.
 */
void incr_us_interval(struct timeval *start, uint64_t incr) {
  incr += (start->tv_sec * 1000000 + start->tv_usec);
  start->tv_sec  = incr/1000000;
  start->tv_usec = incr%1000000;
  return;
}
