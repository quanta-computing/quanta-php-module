#ifndef QM_PROFILER_H_
#define QM_PROFILER_H_

/* Fictitious function name to represent top of the call tree. The paranthesis
 * in the name is to ensure we don't conflict with user function names.  */
#define ROOT_SYMBOL                "main()"

/*
** Misc #defines to handle profiling triggering
*/
#define QUANTA_HTTP_HEADER "HTTP_X_QUANTA"
#define QUANTA_HTTP_HEADER_MODE_MAGE "magento"
#define QUANTA_HTTP_HEADER_MODE_FULL "full"
#define QUANTA_MON_DEFAULT_ADMIN_URL "/admin/"

// Timeout for sending metrics to monikor agent
#define SEND_METRICS_TIMEOUT_US 50000

/* Various QUANTA_MON modes. If you are adding a new mode, register the appropriate
 * callbacks in hp_begin() */
#define QUANTA_MON_MODE_HIERARCHICAL            1      /* Complete profiling of every single PHP calls */
#define QUANTA_MON_MODE_SAMPLED                 2      /* Statistical sample of most PHP calls (not used anymore) */
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

#define MAX_METRIC_NAME_LENGTH 1024


/* QuantaMon maintains a stack of entries being profiled. The memory for the entry
 * is passed by the layer that invokes BEGIN_PROFILING(), e.g. the hp_execute()
 * function. Often, this is just C-stack memory.
 *
 * This structure is a convenient place to track start time of a particular
 * profile operation, recursion depth, and the name of the function being
 * profiled. */
typedef struct hp_entry_t {
  const char                   *name_hprof;                       /* function name */
  int                     rlvl_hprof;        /* recursion level for function */
  uint64_t                  tsc_start;         /* start value for TSC counter  */
  long int                mu_start_hprof;                    /* memory usage */
  long int                pmu_start_hprof;              /* peak memory usage */
  struct rusage           ru_start_hprof;             /* user/sys time start */
  struct hp_entry_t      *prev_hprof;    /* ptr to prev entry being profiled */
  uint8_t                   hash_code;     /* hash_code for the function name  */
} hp_entry_t;

typedef enum {
  APP_EV_CACHE_CLEAR = 1,
  APP_EV_REINDEX = 2
} applicative_event_class_t;

typedef struct applicative_event {
  struct applicative_event *prev;
  uint8_t class;
  char *type;
  char *subtype;
} applicative_event_t;


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

typedef struct profiled_application_t profiled_application_t;
typedef int (*profiled_function_callback_t)(profiled_application_t *app,
  zend_execute_data *ex TSRMLS_DC);

typedef struct profiled_function_t {
  const char *name;
  const size_t index;

  const struct {
    uint8_t ignore_in_stack;
    int min_profiling_level;
  } options;

  profiled_function_callback_t begin_callback;
  profiled_function_callback_t end_callback;

  struct {
    uint64_t first_start;
    uint64_t last_start;
    uint64_t first_stop;
    uint64_t last_stop;
  } tsc;

  struct {
    uint64_t cycles;
    uint64_t count;
    uint64_t cycles_after;
    uint64_t count_after;
  } sql_counters;

} profiled_function_t;

typedef enum {
  PROF_FIRST_START,
  PROF_LAST_START,
  PROF_FIRST_STOP,
  PROF_LAST_STOP
} profiler_timer_counter_t;

typedef struct {
  profiled_function_t *function;
  uint8_t counter;
} profiler_timer_function_t;

typedef struct {
  const char *name;
  profiler_timer_function_t start;
  profiler_timer_function_t end;
} profiler_timer_t;

struct profiled_application_t {
  const char *name;

  profiled_function_t *functions;
  size_t nb_functions;
  const profiler_timer_t *timers;
  size_t nb_timers;

  profiled_function_t *first_app_function;
  profiled_function_t *last_app_function;

  profiled_function_t *current_function;
  profiled_function_t *last_function;
  void *context;

  void *(*create_context)(struct profiled_application_t *app TSRMLS_DC);
  void (*cleanup_context)(struct profiled_application_t *app TSRMLS_DC);
  profiled_function_t *(*match_function)(const char *name, zend_execute_data *data TSRMLS_DC);
  void (*send_metrics)(struct profiled_application_t *app, monikor_metric_list_t *metrics,
    float cpufreq, struct timeval *clock TSRMLS_DC);
};


// Profiling
void hp_begin(long level, long quanta_mon_flags TSRMLS_DC);
void hp_stop(TSRMLS_D);
void hp_end(TSRMLS_D);
void hp_init_profiler_state(int level TSRMLS_DC);
void hp_clean_profiler_state(TSRMLS_D);
void hp_inc_count(zval *counts, char *name, long count TSRMLS_DC);
size_t hp_get_entry_name(hp_entry_t  *entry, char *result_buf, size_t result_len);
size_t hp_get_function_stack(hp_entry_t *entry, int level, char *result_buf, size_t result_len);

int hp_begin_profiling(hp_entry_t **entries, const char *symbol, zend_execute_data *data TSRMLS_DC);
void hp_end_profiling(hp_entry_t **entries, int profile_curr, zend_execute_data *data TSRMLS_DC);
int qm_begin_profiling(const char *curr_func, zend_execute_data *execute_data TSRMLS_DC);
int qm_end_profiling(int function_idx, zend_execute_data *execute_data TSRMLS_DC);
void hp_hijack_zend_execute(uint32_t flags, long level);
void hp_restore_original_zend_execute(void);

// Profiling callbacks
void hp_mode_dummy_init_cb(TSRMLS_D);
void hp_mode_dummy_exit_cb(TSRMLS_D);
void hp_mode_dummy_beginfn_cb(hp_entry_t **entries, hp_entry_t *current  TSRMLS_DC);
void hp_mode_dummy_endfn_cb(hp_entry_t **entries   TSRMLS_DC);
void hp_mode_common_beginfn(hp_entry_t **entries, hp_entry_t *current  TSRMLS_DC);
void hp_mode_common_endfn(hp_entry_t **entries, hp_entry_t *current TSRMLS_DC);
void hp_mode_hier_beginfn_cb(hp_entry_t **entries, hp_entry_t *current  TSRMLS_DC);
void hp_mode_hier_endfn_cb(hp_entry_t **entries  TSRMLS_DC);

// Metrics stuff
void send_metrics(TSRMLS_D);
void qm_send_events_metrics(struct timeval *clock, monikor_metric_list_t *metrics TSRMLS_DC);
void qm_send_profiler_metrics(struct timeval *clock, monikor_metric_list_t *metrics,
  float cpufreq TSRMLS_DC);
void qm_send_selfprofiling_metrics(struct timeval *clock, monikor_metric_list_t *metrics,
  float cpufreq TSRMLS_DC);

  struct {
    uint64_t start;
    uint64_t stop;
  } global_tsc;

  struct {
    uint64_t start;
    uint64_t stop;
  } global_sql_counters;

// Application stuff
void init_profiled_application(profiled_application_t *app TSRMLS_DC);
void clean_profiled_application(profiled_application_t *app TSRMLS_DC);
int qm_record_event(applicative_event_class_t class, char *type, char *subtype);
int qm_record_sql_query(profiled_application_t *app, zend_execute_data *data TSRMLS_DC);

// HP list
void hp_free_the_free_list();
hp_entry_t *hp_fast_alloc_hprof_entry();
void hp_fast_free_hprof_entry(hp_entry_t *p);

// Monitored functions filter
void hp_fill_monitored_functions(char **function_names);
int hp_match_monitored_function(const char* function_name, zend_execute_data* data TSRMLS_DC);

// Zend hijacks
#if PHP_VERSION_ID < 50500
ZEND_DLEXPORT void hp_execute (zend_op_array *ops TSRMLS_DC);
ZEND_DLEXPORT void hp_execute_internal(zend_execute_data *execute_data, int ret TSRMLS_DC);
#elif PHP_MAJOR_VERSION < 7
ZEND_DLEXPORT void hp_execute_ex (zend_execute_data *execute_data TSRMLS_DC);
ZEND_DLEXPORT void hp_execute_internal(zend_execute_data *execute_data,
  struct _zend_fcall_info *fci, int ret TSRMLS_DC);
#else
ZEND_DLEXPORT void hp_execute_ex (zend_execute_data *execute_data TSRMLS_DC);
ZEND_DLEXPORT void hp_execute_internal(zend_execute_data *execute_data, zval *return_value);
#endif
ZEND_DLEXPORT zend_op_array* hp_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC);
ZEND_DLEXPORT zend_op_array* hp_compile_string(zval *source_string, char *filename TSRMLS_DC);


#endif /* end of include guard: QM_PROFILER_H_ */
