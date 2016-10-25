#ifndef QUANTA_MON_H_
#define QUANTA_MON_H_

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/pdo/php_pdo_driver.h"
#include "zend_extensions.h"
#include <SAPI.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <monikor/metric.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef ZTS
# include "TSRM.h"
#endif

#include "cpu.h"
#include "compat.h"
#include "profiler.h"
#include "module.h"
#include "utils.h"
#include "debug.h"

#include "magento_common.h"
#include "magento1.h"
#include "magento2.h"

/* Size of a temp scratch buffer            */
#define SCRATCH_BUF_LEN            512
#define QUANTA_MON_VERSION "1.2.2"


/* Xhprof's global state.
 *
 * This structure is instantiated once.  Initialize defaults for attributes in
 * hp_init_profiler_state() Cleanup/free attributes in
 * hp_clean_profiler_state() */
typedef struct hp_global_t {

  /*       ----------   Global attributes:  -----------       */

  /* Indicates if quanta_mon is currently enabled */
  int              enabled;

  /* Indicates if quanta_mon was ever enabled during this request */
  int              ever_enabled;

  /* Indicates the current quanta_mon mode or level */
  int              profiler_level;

  /* The step ID to match step/scenario/site when the data come back to Quanta */
  uint64_t         quanta_step_id;

  /* The reference clock to map the metrics to */
  uint64_t         quanta_clock;

  /* Extracted from _SERVER['REQUEST_URI'] */
  char            *request_uri;
  char            *admin_url;

  zval            stats_count;

  /* Top of the profile stack */
  hp_entry_t      *entries;

  /* freelist of hp_entry_t chunks for reuse... */
  hp_entry_t      *entry_free_list;

  /* Path of the quanta agent unix socket */
  char             *path_quanta_agent_socket;

  /* This array is used to store cpu frequencies for all available logical
   * cpus.  For now, we assume the cpu frequencies will not change for power
   * saving or other reasons. If we need to worry about that in the future, we
   * can use a periodical timer to re-calculate this arrary every once in a
   * while (for example, every 1 or 5 seconds). */
  double *cpu_frequencies;

  /* The number of logical CPUs this machine has. */
  uint32_t cpu_num;

  /* The saved cpu affinity. */
  cpu_set_t prev_mask;

  /* The cpu id current process is bound to. (default 0) */
  uint32_t cur_cpu_id;

  /* counter table indexed by hash value of function names. */
  uint8_t  func_hash_counters[256];

  struct {
    uint64_t start;
    uint64_t stop;
  } global_tsc;

  profiled_application_t *profiled_application;

  applicative_event_t *app_events;

  struct {
    uint64_t total;
    uint64_t function;
    uint64_t class_unmatched;
    uint64_t cycles;
    uint64_t hash_cycles;
    uint64_t profiling_cycles;
    uint64_t init_cycles;
    uint64_t shutdown_cycles;
    int fd; // useful for logging all calls
  } internal_match_counters;
} hp_global_t;


// Global state
extern hp_global_t       hp_globals;

#endif /* end of include guard: QUANTA_MON_H_ */
