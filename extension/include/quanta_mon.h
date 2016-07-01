#ifndef QUANTA_MON_H_
#define QUANTA_MON_H_

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
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

#include "cpu.h"

/*
** Full debug mode
*/

#define DEBUG_QUANTA

#ifdef DEBUG_QUANTA
# define PRINTF_QUANTA(...) dprintf(1, __VA_ARGS__)
#else
# define PRINTF_QUANTA dummy_printf
  static void dummy_printf(char *unused, ...)
  {
    (void)unused;
    return;
  }
#endif

/* Fictitious function name to represent top of the call tree. The paranthesis
 * in the name is to ensure we don't conflict with user function names.  */
#define ROOT_SYMBOL                "main()"

/* Size of a temp scratch buffer            */
#define SCRATCH_BUF_LEN            512

/* Various QUANTA_MON modes. If you are adding a new mode, register the appropriate
 * callbacks in hp_begin() */
#define QUANTA_MON_MODE_HIERARCHICAL            1      /* Complete profiling of every single PHP calls */
#define QUANTA_MON_MODE_SAMPLED                 2      /* Statistical sample of most PHP calls */
#define QUANTA_MON_MODE_MAGENTO_PROFILING       3      /* Profiling of selected Magento calls (see hp_get_monitored_functions_fill) */
#define QUANTA_MON_MODE_EVENTS_ONLY             4      /* No profiling, deal only with calls >= POS_ENTRY_EVENTS_ONLY */

/* Hierarchical profiling flags.
 *
 * Note: Function call counts and wall (elapsed) time are always profiled.
 * The following optional flags can be used to control other aspects of
 * profiling.
 */
#define QUANTA_MON_FLAGS_NO_BUILTINS   0x0001         /* do not profile builtins */
#define QUANTA_MON_FLAGS_CPU           0x0002      /* gather CPU times for funcs */
#define QUANTA_MON_FLAGS_MEMORY        0x0004   /* gather memory usage for funcs */

/* Constants for QUANTA_MON_MODE_SAMPLED        */
#define QUANTA_MON_SAMPLING_INTERVAL       100000      /* In microsecs        */

/* Constant for ignoring functions, transparent to hierarchical profile */
#define QUANTA_MON_MAX_IGNORED_FUNCTIONS  256
#define QUANTA_MON_IGNORED_FUNCTION_FILTER_SIZE ((QUANTA_MON_MAX_IGNORED_FUNCTIONS + 7)/8)

#define QUANTA_EXTRA_CHECKS
#define QUANTA_MON_MAX_MONITORED_FUNCTIONS_HASH  256
#define QUANTA_MON_MAX_MONITORED_FUNCTIONS  23
#define QUANTA_MON_MONITORED_FUNCTION_FILTER_SIZE ((QUANTA_MON_MAX_MONITORED_FUNCTIONS_HASH + 7)/8)

#define QUANTA_MAGENTO_VERSION_INDEX 1

/* Monitored functions positions */
#define POS_ENTRY_APP_RUN 0
#define POS_ENTRY_TOHTML     15
#define POS_ENTRY_PDO_EXECUTE      16
#define POS_ENTRY_EVENTS_ONLY      17 /* Anything below won't be processed unless the special cookie is set */
#define POS_ENTRY_EV_CACHE_FLUSH   17
#define POS_ENTRY_EV_CLEAN_TYPE    18
#define POS_ENTRY_EV_MAGE_CLEAN    19
#define POS_ENTRY_EV_BEFORE_SAVE   20
#define POS_ENTRY_PHP_TOTAL        21


/* Bloom filter for function names to be ignored */
#define INDEX_2_BYTE(index)  (index >> 3)
#define INDEX_2_BIT(index)   (1 << (index & 0x7));


extern zend_module_entry quanta_mon_module_entry;
#define phpext_quanta_mon_ptr &quanta_mon_module_entry

#ifdef PHP_WIN32
# define PHP_QUANTA_MON_API __declspec(dllexport)
#else
# define PHP_QUANTA_MON_API
#endif

#ifdef ZTS
# include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(quanta_mon);
PHP_MSHUTDOWN_FUNCTION(quanta_mon);
PHP_RINIT_FUNCTION(quanta_mon);
PHP_RSHUTDOWN_FUNCTION(quanta_mon);
PHP_MINFO_FUNCTION(quanta_mon);

#define QUANTA_MON_VERSION "1.1.0"

#define QUANTA_HTTP_HEADER "HTTP_X_QUANTA"
#define QUANTA_HTTP_HEADER_MODE_MAGE "magento"
#define QUANTA_HTTP_HEADER_MODE_FULL "full"
#define QUANTA_MON_DEFAULT_ADMIN_URL "/admin/"

#define SEND_METRICS_TIMEOUT_US 50000


/* QuantaMon maintains a stack of entries being profiled. The memory for the entry
 * is passed by the layer that invokes BEGIN_PROFILING(), e.g. the hp_execute()
 * function. Often, this is just C-stack memory.
 *
 * This structure is a convenient place to track start time of a particular
 * profile operation, recursion depth, and the name of the function being
 * profiled. */
typedef struct hp_entry_t {
  char                   *name_hprof;                       /* function name */
  int                     rlvl_hprof;        /* recursion level for function */
  uint64_t                  tsc_start;         /* start value for TSC counter  */
  long int                mu_start_hprof;                    /* memory usage */
  long int                pmu_start_hprof;              /* peak memory usage */
  struct rusage           ru_start_hprof;             /* user/sys time start */
  struct hp_entry_t      *prev_hprof;    /* ptr to prev entry being profiled */
  uint8_t                   hash_code;     /* hash_code for the function name  */
} hp_entry_t;

/* Various types for QUANTA_MON callbacks       */
typedef void (*hp_init_cb)           (TSRMLS_D);
typedef void (*hp_exit_cb)           (TSRMLS_D);
typedef void (*hp_begin_function_cb) (hp_entry_t **entries,
                                      hp_entry_t *current   TSRMLS_DC);
typedef void (*hp_end_function_cb)   (hp_entry_t **entries  TSRMLS_DC);

/* Struct to hold the various callbacks for a single quanta_mon mode */
typedef struct hp_mode_cb {
  hp_init_cb             init_cb;
  hp_exit_cb             exit_cb;
  hp_begin_function_cb   begin_fn_cb;
  hp_end_function_cb     end_fn_cb;
} hp_mode_cb;

typedef struct magento_block_t {
  uint64_t  tsc_renderize_first_start; /* Might be re-entrant, record the first call */
  uint64_t  tsc_renderize_last_stop;   /* Always record the latest timestamp */
  uint64_t  renderize_children_cycles;
  uint64_t  sql_cpu_cycles;
  uint64_t  sql_queries_count;
  char    *name;
  char    *class;
  char    *class_file;
  char    *template;
  struct magento_block_t *next;
} magento_block_t;

typedef struct block_stack {
  struct block_stack *prev;
  magento_block_t *block;
} block_stack_t;

#define MAGENTO_EVENT_CACHE_CLEAR 1
#define MAGENTO_EVENT_REINDEX 2
typedef struct magento_event {
  struct magento_event *prev;
  uint8_t class;
  char *type;
  char *subtype;
} magento_event_t;

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

  /* Holds all the quanta_mon statistics */
  zval            *stats_count;

  /* Indicates the current quanta_mon mode or level */
  int              profiler_level;

  /* The step ID to match step/scenario/site when the data come back to Quanta */
  uint64_t         quanta_step_id;

  /* The reference clock to map the metrics to */
  uint64_t         quanta_clock;

  /* Extracted from _SERVER['REQUEST_URI'] */
  char            *request_uri;
  char            *admin_url;

  /* Top of the profile stack */
  hp_entry_t      *entries;

  /* freelist of hp_entry_t chunks for reuse... */
  hp_entry_t      *entry_free_list;

  /* Callbacks for various quanta_mon modes */
  hp_mode_cb       mode_cb;

  /* Path of the quanta agent unix socket */
  char             *path_quanta_agent_socket;

  /*       ----------   Mode specific attributes:  -----------       */

  /* Global to track the time of the last sample in time and ticks */
  struct timeval   last_sample_time;
  uint64_t           last_sample_tsc;
  /* QUANTA_MON_SAMPLING_INTERVAL in ticks */
  uint64_t           sampling_interval_tsc;

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

  /* QuantaMon flags */
  uint32_t quanta_mon_flags;

  /* counter table indexed by hash value of function names. */
  uint8_t  func_hash_counters[256];

  /* Table of ignored function names and their filter */
  char  **ignored_function_names;
  uint8_t   ignored_function_filter[QUANTA_MON_IGNORED_FUNCTION_FILTER_SIZE];

  /* Table of monitored function names and their filter */
  char     *monitored_function_names[2][QUANTA_MON_MAX_MONITORED_FUNCTIONS];
  uint8_t  monitored_function_filter[QUANTA_MON_MONITORED_FUNCTION_FILTER_SIZE];
  uint64_t monitored_function_tsc_start[QUANTA_MON_MAX_MONITORED_FUNCTIONS];
  uint64_t monitored_function_tsc_stop[QUANTA_MON_MAX_MONITORED_FUNCTIONS];
  uint64_t monitored_function_sql_cpu_cycles[QUANTA_MON_MAX_MONITORED_FUNCTIONS];
  uint64_t monitored_function_sql_queries_count[QUANTA_MON_MAX_MONITORED_FUNCTIONS];
  uint64_t monitored_function_sql_cpu_cycles_after[QUANTA_MON_MAX_MONITORED_FUNCTIONS];
  uint64_t monitored_function_sql_queries_count_after[QUANTA_MON_MAX_MONITORED_FUNCTIONS];

  int current_monitored_function;
  int last_monitored_function;
  magento_block_t *magento_blocks_first;
  magento_block_t *magento_blocks_last;
  block_stack_t *block_stack;

  magento_event_t *magento_events;

  char *magento_version;
  char *magento_edition;
} hp_global_t;

// CPU
int restore_cpu_affinity(cpu_set_t * prev_mask);
int bind_to_cpu(uint32_t cpu_id);
uint64_t cycle_timer();
double get_cpu_frequency();
void clear_frequencies();
void get_all_cpu_frequencies();
long get_us_interval(struct timeval *start, struct timeval *end);
void incr_us_interval(struct timeval *start, uint64_t incr);
float cpu_cycles_range_to_ms(float cpufreq, long long start, long long end);
float cpu_cycles_to_ms(float cpufreq, uint64_t count);
inline double get_us_from_tsc(uint64_t count, double cpu_frequency);
inline uint64_t get_tsc_from_us(uint64_t usecs, double cpu_frequency);

// Zval
zval  *hp_zval_at_key(char *key, zval *values);
char **hp_strings_in_zval(zval *values);


// Constants
void hp_register_constants(INIT_FUNC_ARGS);


// HP list
void hp_free_the_free_list();
hp_entry_t *hp_fast_alloc_hprof_entry();
void hp_fast_free_hprof_entry(hp_entry_t *p);


// Utils
void   hp_array_del(char **name_array);
inline uint8_t hp_inline_hash(char * str);
zval * hp_hash_lookup(char *symbol  TSRMLS_DC);
const char *hp_get_base_filename(const char *filename);
char *hp_get_function_name(zend_execute_data *data TSRMLS_DC);
char *hp_get_function_name_fast(zend_execute_data *execute_data TSRMLS_DC);
size_t hp_get_function_stack(hp_entry_t *entry, int level, char *result_buf, size_t result_len);
void hp_trunc_time(struct timeval *tv, uint64_t intr);
char *get_mage_model_data(HashTable *attrs, char *key TSRMLS_DC);
void fetch_magento_version(TSRMLS_D);
int safe_call_function(char *function, zval *ret, int ret_type,
  size_t params_count, zval **params TSRMLS_DC);
int safe_call_method(zval *object, char *function, zval *ret, int ret_type,
  size_t params_count, zval **params TSRMLS_DC);
zval *safe_new(char *class, int params_count, zval **params TSRMLS_DC);
zval *safe_get_class_constant(char *class, char *name, int type TSRMLS_DC);

// Zend hijacks
#if PHP_VERSION_ID < 50500
ZEND_DLEXPORT void hp_execute (zend_op_array *ops TSRMLS_DC);
ZEND_DLEXPORT void hp_execute_internal(zend_execute_data *execute_data, int ret TSRMLS_DC);
#else
ZEND_DLEXPORT void hp_execute_ex (zend_execute_data *execute_data TSRMLS_DC);
ZEND_DLEXPORT void hp_execute_internal(zend_execute_data *execute_data,
  struct _zend_fcall_info *fci, int ret TSRMLS_DC);
#endif
ZEND_DLEXPORT zend_op_array* hp_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC);
ZEND_DLEXPORT zend_op_array* hp_compile_string(zval *source_string, char *filename TSRMLS_DC);


// Profiling
void hp_begin(long level, long quanta_mon_flags TSRMLS_DC);
void hp_stop(TSRMLS_D);
void hp_end(TSRMLS_D);
void hp_init_profiler_state(int level TSRMLS_DC);
void hp_clean_profiler_state(TSRMLS_D);
void hp_inc_count(zval *counts, char *name, long count TSRMLS_DC);
size_t hp_get_entry_name(hp_entry_t  *entry, char *result_buf, size_t result_len);
char **hp_globals_monitored_function_names(void);

int hp_begin_profiling(hp_entry_t **entries, const char *symbol, zend_execute_data *data TSRMLS_DC);
void hp_end_profiling(hp_entry_t **entries, int profile_curr, zend_execute_data *data TSRMLS_DC);
void hp_hijack_zend_execute(uint32_t flags, long level);
void hp_restore_original_zend_execute(void);
void hp_sample_stack(hp_entry_t **entries  TSRMLS_DC);
void hp_sample_check(hp_entry_t **entries  TSRMLS_DC);

// Quanta stuff
void send_metrics(TSRMLS_D);
int qm_begin_profiling(uint8_t hash_code, const char *curr_func, zend_execute_data *execute_data TSRMLS_DC);
int qm_end_profiling(int profile_curr, zend_execute_data *execute_data TSRMLS_DC);
int qm_before_tohtml(int profile_curr, zend_execute_data *execute_data TSRMLS_DC);
int qm_after_tohtml(zend_execute_data *execute_data TSRMLS_DC);
int qm_record_reindex_event(int profile_curr, zend_execute_data *execute_data TSRMLS_DC);
int qm_record_cache_system_flush_event(int profile_curr, zend_execute_data *execute_data TSRMLS_DC);
int qm_record_cache_flush_event(int profile_curr, zend_execute_data *execute_data TSRMLS_DC);
int qm_record_cache_clean_event(int profile_curr, zend_execute_data *execute_data TSRMLS_DC);
int qm_record_sql_timers(void);


// Profiling callbacks
void hp_mode_dummy_init_cb(TSRMLS_D);
void hp_mode_dummy_exit_cb(TSRMLS_D);
void hp_mode_dummy_beginfn_cb(hp_entry_t **entries, hp_entry_t *current  TSRMLS_DC);
void hp_mode_dummy_endfn_cb(hp_entry_t **entries   TSRMLS_DC);
void hp_mode_common_beginfn(hp_entry_t **entries, hp_entry_t *current  TSRMLS_DC);
void hp_mode_common_endfn(hp_entry_t **entries, hp_entry_t *current TSRMLS_DC);
void hp_mode_sampled_init_cb(TSRMLS_D);
void hp_mode_hier_beginfn_cb(hp_entry_t **entries, hp_entry_t *current  TSRMLS_DC);
void hp_mode_magento_profil_beginfn_cb(hp_entry_t **entries, hp_entry_t *current  TSRMLS_DC);
void hp_mode_events_only_beginfn_cb(hp_entry_t **entries, hp_entry_t *current  TSRMLS_DC);
void hp_mode_sampled_beginfn_cb(hp_entry_t **entries, hp_entry_t *current  TSRMLS_DC);
zval * hp_mode_shared_endfn_cb(hp_entry_t *top, char *symbol  TSRMLS_DC);
void hp_mode_hier_endfn_cb(hp_entry_t **entries  TSRMLS_DC);
void hp_mode_sampled_endfn_cb(hp_entry_t **entries  TSRMLS_DC);
void hp_mode_magento_profil_endfn_cb(hp_entry_t **entries  TSRMLS_DC);
void hp_mode_events_only_endfn_cb(hp_entry_t **entries  TSRMLS_DC);


// Monitored/ignored functions filter
void hp_get_ignored_functions_from_arg(zval *args);
void hp_get_monitored_functions_fill();
void hp_ignored_functions_filter_clear();
void hp_ignored_functions_filter_init();

void hp_monitored_functions_filter_clear();
void hp_monitored_functions_filter_init();
int hp_monitored_functions_filter_collision(uint8_t hash);
int  hp_ignore_entry_work(uint8_t hash_code, char *curr_func);
inline int hp_ignore_entry(uint8_t hash_code, char *curr_func);


// Block stack
magento_block_t *block_stack_pop(void);
void block_stack_push(magento_block_t *block);
magento_block_t *block_stack_top(void);


// Global state
extern hp_global_t       hp_globals;

#endif /* end of include guard: QUANTA_MON_H_ */
