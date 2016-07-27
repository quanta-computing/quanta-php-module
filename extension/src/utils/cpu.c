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
