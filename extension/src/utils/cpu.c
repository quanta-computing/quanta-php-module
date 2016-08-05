#include "cpu.h"
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

  // Lazy initialize cpu frequencies
  if (!hp_globals.cpu_frequencies
  && !(hp_globals.cpu_frequencies = calloc(hp_globals.cpu_num, sizeof(double)))) {
    return -1;
  }
  /* record the cpu_id the process is bound to and init it's frequency if needed. */
  hp_globals.cur_cpu_id = cpu_id;
  if (hp_globals.cpu_frequencies[cpu_id] == 0.0)
    hp_globals.cpu_frequencies[cpu_id] = get_cpu_frequency();
  printf("CPUFREQ %f\n", hp_globals.cpu_frequencies[cpu_id]);
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
  uint64_t tsc_start;
  uint64_t tsc_end;

  if (gettimeofday(&start, 0)) {
    perror("gettimeofday");
    return 0.0;
  }
  tsc_start = cycle_timer();
  usleep(5000);
  if (gettimeofday(&end, 0)) {
    perror("gettimeofday");
    return 0.0;
  }
  tsc_end = cycle_timer();
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
