/*
 *  Copyright (c) 2009 Facebook
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef linux
/* To enable CPU_ZERO and CPU_SET, etc.     */
//# define _GNU_SOURCE
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_quanta_mon.h"
#include "zend_extensions.h"
#include <SAPI.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef __FreeBSD__
# if __FreeBSD_version >= 700110
#   include <sys/resource.h>
#   include <sys/cpuset.h>
#   define cpu_set_t cpuset_t
#   define SET_AFFINITY(pid, size, mask) \
           cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, -1, size, mask)
#   define GET_AFFINITY(pid, size, mask) \
           cpuset_getaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, -1, size, mask)
# else
#   error "This version of FreeBSD does not support cpusets"
# endif /* __FreeBSD_version */
#elif __APPLE__
/*
 * Patch for compiling in Mac OS X Leopard
 * @author Svilen Spasov <s.spasov@gmail.com>
 */
#    include <mach/mach_init.h>
#    include <mach/thread_policy.h>
#    define cpu_set_t thread_affinity_policy_data_t
#    define CPU_SET(cpu_id, new_mask) \
        (*(new_mask)).affinity_tag = (cpu_id + 1)
#    define CPU_ZERO(new_mask)                 \
        (*(new_mask)).affinity_tag = THREAD_AFFINITY_TAG_NULL
#   define SET_AFFINITY(pid, size, mask)       \
        thread_policy_set(mach_thread_self(), THREAD_AFFINITY_POLICY, mask, \
                          THREAD_AFFINITY_POLICY_COUNT)
#else
/* For sched_getaffinity, sched_setaffinity */
# include <sched.h>
# define SET_AFFINITY(pid, size, mask) sched_setaffinity(0, size, mask)
# define GET_AFFINITY(pid, size, mask) sched_getaffinity(0, size, mask)
#endif /* __FreeBSD__ */



/**
 * **********************
 * GLOBAL MACRO CONSTANTS
 * **********************
 */

/* QuantaMon version                           */
#define QUANTA_MON_VERSION       "0.9.2"
/* Set the following cookie for enabling full monitoring. The cookie value
 * should be modified for each customers, maybe moved to a configuration file */
#define DEBUG_QUANTA

#ifdef DEBUG_QUANTA
  #define PRINTF_QUANTA printf
#else
  #define PRINTF_QUANTA dummy
#endif
static void dummy(char *unused, ...)
{
        return;
}

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
#define QUANTA_MON_IGNORED_FUNCTION_FILTER_SIZE                           \
               ((QUANTA_MON_MAX_IGNORED_FUNCTIONS + 7)/8)

#define QUANTA_EXTRA_CHECKS
#define QUANTA_MON_MAX_MONITORED_FUNCTIONS_HASH  256
#define QUANTA_MON_MAX_MONITORED_FUNCTIONS  13
#define QUANTA_MON_MONITORED_FUNCTION_FILTER_SIZE                           \
               ((QUANTA_MON_MAX_MONITORED_FUNCTIONS_HASH + 7)/8)

#if !defined(uint64)
typedef unsigned long long uint64;
#endif
#if !defined(uint32)
typedef unsigned int uint32;
#endif
#if !defined(uint8)
typedef unsigned char uint8;
#endif


/**
 * *****************************
 * GLOBAL DATATYPES AND TYPEDEFS
 * *****************************
 */

/* QuantaMon maintains a stack of entries being profiled. The memory for the entry
 * is passed by the layer that invokes BEGIN_PROFILING(), e.g. the hp_execute()
 * function. Often, this is just C-stack memory.
 *
 * This structure is a convenient place to track start time of a particular
 * profile operation, recursion depth, and the name of the function being
 * profiled. */
typedef struct hp_entry_t {
  char                   *name_hprof;                       /* function name */
  char                   *pathname_hprof;                /* path of the file */
  int                     rlvl_hprof;        /* recursion level for function */
  uint64                  tsc_start;         /* start value for TSC counter  */
  long int                mu_start_hprof;                    /* memory usage */
  long int                pmu_start_hprof;              /* peak memory usage */
  struct rusage           ru_start_hprof;             /* user/sys time start */
  struct hp_entry_t      *prev_hprof;    /* ptr to prev entry being profiled */
  uint8                   hash_code;     /* hash_code for the function name  */
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

typedef struct generate_renderize_block_details_t {
  uint64  tsc_generate_start;
  uint64  tsc_generate_stop;
  uint64  tsc_renderize_first_start; /* Might be re-entrant, record the first call */
  uint64  tsc_renderize_last_stop;   /* Always record the latest timestamp */
  char    *type;
  char    *name;
  char    *class;
  char    *template;
  struct generate_renderize_block_details_t *next_generate_renderize_block_detail;
} generate_renderize_block_details;

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

  /* Top of the profile stack */
  hp_entry_t      *entries;

  /* freelist of hp_entry_t chunks for reuse... */
  hp_entry_t      *entry_free_list;

  /* Callbacks for various quanta_mon modes */
  hp_mode_cb       mode_cb;

  /* Cookie value that must be received for enabling full profiling (all PHP calls, like xhprof) */
  char             *full_profiling_cookie_trigger;

  /* Cookie value that must be received for enabling magento profiling (Magento calls listed in hp_get_monitored_functions_fill) */
  char             *magento_profiling_cookie_trigger;

  /* Path of the forwarding agent */
  char             *path_quanta_agent_exe;

  /* Path of the quanta agent unix socket */
  char             *path_quanta_agent_socket;

  /*       ----------   Mode specific attributes:  -----------       */

  /* Global to track the time of the last sample in time and ticks */
  struct timeval   last_sample_time;
  uint64           last_sample_tsc;
  /* QUANTA_MON_SAMPLING_INTERVAL in ticks */
  uint64           sampling_interval_tsc;

  /* This array is used to store cpu frequencies for all available logical
   * cpus.  For now, we assume the cpu frequencies will not change for power
   * saving or other reasons. If we need to worry about that in the future, we
   * can use a periodical timer to re-calculate this arrary every once in a
   * while (for example, every 1 or 5 seconds). */
  double *cpu_frequencies;

  /* The number of logical CPUs this machine has. */
  uint32 cpu_num;

  /* The saved cpu affinity. */
  cpu_set_t prev_mask;

  /* The cpu id current process is bound to. (default 0) */
  uint32 cur_cpu_id;

  /* QuantaMon flags */
  uint32 quanta_mon_flags;

  /* counter table indexed by hash value of function names. */
  uint8  func_hash_counters[256];

  /* Table of ignored function names and their filter */
  char  **ignored_function_names;
  uint8   ignored_function_filter[QUANTA_MON_IGNORED_FUNCTION_FILTER_SIZE];

  /* Table of monitored function names and their filter */
  char   *monitored_function_names[QUANTA_MON_MAX_MONITORED_FUNCTIONS];
  uint8   monitored_function_filter[QUANTA_MON_MONITORED_FUNCTION_FILTER_SIZE];
  uint64  monitored_function_tsc_start[QUANTA_MON_MAX_MONITORED_FUNCTIONS];
  uint64  monitored_function_tsc_stop[QUANTA_MON_MAX_MONITORED_FUNCTIONS];
  generate_renderize_block_details *monitored_function_generate_renderize_block_first_linked_list;
  generate_renderize_block_details *monitored_function_generate_renderize_block_last_linked_list;
  generate_renderize_block_details *renderize_block_last_used;
  uint32  quanta_dbg;

} hp_global_t;


/**
 * ***********************
 * GLOBAL STATIC VARIABLES
 * ***********************
 */
/* QuantaMon global state */
static hp_global_t       hp_globals;

#if PHP_VERSION_ID < 50500
/* Pointer to the original execute function */
static ZEND_DLEXPORT void (*_zend_execute) (zend_op_array *ops TSRMLS_DC);

/* Pointer to the origianl execute_internal function */
static ZEND_DLEXPORT void (*_zend_execute_internal) (zend_execute_data *data,
                           int ret TSRMLS_DC);
#else
/* Pointer to the original execute function */
static void (*_zend_execute_ex) (zend_execute_data *execute_data TSRMLS_DC);

/* Pointer to the origianl execute_internal function */
static void (*_zend_execute_internal) (zend_execute_data *data,
                      struct _zend_fcall_info *fci, int ret TSRMLS_DC);
#endif

/* Pointer to the original compile function */
static zend_op_array * (*_zend_compile_file) (zend_file_handle *file_handle,
                                              int type TSRMLS_DC);

/* Pointer to the original compile string function (used by eval) */
static zend_op_array * (*_zend_compile_string) (zval *source_string, char *filename TSRMLS_DC);

/* Bloom filter for function names to be ignored */
#define INDEX_2_BYTE(index)  (index >> 3)
#define INDEX_2_BIT(index)   (1 << (index & 0x7));


/**
 * ****************************
 * STATIC FUNCTION DECLARATIONS
 * ****************************
 */
static void hp_register_constants(INIT_FUNC_ARGS);

static void hp_begin(long level, long quanta_mon_flags TSRMLS_DC);
static void hp_stop(TSRMLS_D);
static void hp_end(TSRMLS_D);

static inline uint64 cycle_timer();
static double get_cpu_frequency();
static void clear_frequencies();

static void hp_free_the_free_list();
static hp_entry_t *hp_fast_alloc_hprof_entry();
static void hp_fast_free_hprof_entry(hp_entry_t *p);
static inline uint8 hp_inline_hash(char * str);
static void get_all_cpu_frequencies();
static long get_us_interval(struct timeval *start, struct timeval *end);
static void incr_us_interval(struct timeval *start, uint64 incr);

static void hp_get_ignored_functions_from_arg(zval *args);
static void hp_get_monitored_functions_fill();
static void hp_ignored_functions_filter_clear();
static void hp_ignored_functions_filter_init();

static void hp_monitored_functions_filter_clear();
static void hp_monitored_functions_filter_init();

static inline zval  *hp_zval_at_key(char  *key,
                                    zval  *values);
static inline char **hp_strings_in_zval(zval  *values);
static inline void   hp_array_del(char **name_array);

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_quanta_mon_enable, 0, 0, 0)
  ZEND_ARG_INFO(0, flags)
  ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_quanta_mon_disable, 0)
ZEND_END_ARG_INFO()

/* }}} */

/**
 * *********************
 * FUNCTION PROTOTYPES
 * *********************
 */
int restore_cpu_affinity(cpu_set_t * prev_mask);
int bind_to_cpu(uint32 cpu_id);

/**
 * *********************
 * PHP EXTENSION GLOBALS
 * *********************
 */
/* List of functions implemented/exposed by quanta_mon */
zend_function_entry quanta_mon_functions[] = {
  PHP_FE(quanta_mon_enable, arginfo_quanta_mon_enable)
  PHP_FE(quanta_mon_disable, arginfo_quanta_mon_disable)
  {NULL, NULL, NULL}
};

/* Callback functions for the quanta_mon extension */
zend_module_entry quanta_mon_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
  STANDARD_MODULE_HEADER,
#endif
  "quanta_mon",                        /* Name of the extension */
  quanta_mon_functions,                /* List of functions exposed */
  PHP_MINIT(quanta_mon),               /* Module init callback */
  PHP_MSHUTDOWN(quanta_mon),           /* Module shutdown callback */
  PHP_RINIT(quanta_mon),               /* Request init callback */
  PHP_RSHUTDOWN(quanta_mon),           /* Request shutdown callback */
  PHP_MINFO(quanta_mon),               /* Module info callback */
#if ZEND_MODULE_API_NO >= 20010901
  QUANTA_MON_VERSION,
#endif
  STANDARD_MODULE_PROPERTIES
};

PHP_INI_BEGIN()

/* output directory:
 * Currently this is not used by the extension itself.
 * But some implementations of iQuantaMonRuns interface might
 * choose to save/restore QuantaMon profiler runs in the
 * directory specified by this ini setting.
 */
PHP_INI_ENTRY("quanta_mon.full_profiling_cookie_trigger", "", PHP_INI_SYSTEM, NULL)
PHP_INI_ENTRY("quanta_mon.magento_profiling_cookie_trigger", "", PHP_INI_SYSTEM, NULL)
PHP_INI_ENTRY("quanta_mon.path_quanta_agent_exe", "", PHP_INI_SYSTEM, NULL)
PHP_INI_ENTRY("quanta_mon.path_quanta_agent_socket", "", PHP_INI_SYSTEM, NULL)

PHP_INI_END()

/* Init module */
ZEND_GET_MODULE(quanta_mon)


/**
 * **********************************
 * PHP EXTENSION FUNCTION DEFINITIONS
 * **********************************
 */

/**
 * Start QuantaMon profiling in hierarchical mode.
 *
 * @param  long $flags  flags for hierarchical mode
 * @return void
 * @author kannan
 */
PHP_FUNCTION(quanta_mon_enable) {
  /* The module is now activated at each request, this function is no longer used. */
}

/**
 * Stops QuantaMon from profiling in hierarchical mode anymore and returns the
 * profile info.
 *
 * @param  void
 * @return array  hash-array of QuantaMon's profile info
 * @author kannan, hzhao
 */
PHP_FUNCTION(quanta_mon_disable) {
  /* Like quanta_mon_enable, this function is longer used. */
}

/**
 * Module init callback.
 * Called during Apache startup
 * @author cjiang
 */
PHP_MINIT_FUNCTION(quanta_mon) {
  int i;

  REGISTER_INI_ENTRIES();

  hp_globals.full_profiling_cookie_trigger = INI_STR("quanta_mon.full_profiling_cookie_trigger");
  if ((!hp_globals.full_profiling_cookie_trigger) || (strlen(hp_globals.full_profiling_cookie_trigger) < 4)) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "quanta_mon.full_profiling_cookie_trigger configuration missing or invalid. Module disabled.");
	return FAILURE;
  }

  hp_globals.magento_profiling_cookie_trigger = INI_STR("quanta_mon.magento_profiling_cookie_trigger");
  if ((!hp_globals.magento_profiling_cookie_trigger) || (strlen(hp_globals.magento_profiling_cookie_trigger) < 4)) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "quanta_mon.magento_profiling_cookie_trigger configuration missing or invalid. Module disabled.");
	return FAILURE;
  }

  hp_globals.path_quanta_agent_exe = INI_STR("quanta_mon.path_quanta_agent_exe");
  if ((!hp_globals.path_quanta_agent_exe) || (strlen(hp_globals.path_quanta_agent_exe) < 4)) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "quanta_mon.path_quanta_agent_exe configuration missing or invalid. Module disabled.");
	return FAILURE;
  }

  hp_globals.path_quanta_agent_socket = INI_STR("quanta_mon.path_quanta_agent_socket");
  if ((!hp_globals.path_quanta_agent_socket) || (strlen(hp_globals.path_quanta_agent_socket) < 4)) {
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "quanta_mon.path_quanta_agent_socket configuration missing or invalid. Module disabled.");
	return FAILURE;
  }

  hp_register_constants(INIT_FUNC_ARGS_PASSTHRU);

  /* Get the number of available logical CPUs. */
  hp_globals.cpu_num = sysconf(_SC_NPROCESSORS_CONF);

  /* Get the cpu affinity mask. */
#ifndef __APPLE__
  if (GET_AFFINITY(0, sizeof(cpu_set_t), &hp_globals.prev_mask) < 0) {
    perror("getaffinity");
    return FAILURE;
  }
#else
  CPU_ZERO(&(hp_globals.prev_mask));
#endif

  /* Initialize cpu_frequencies and cur_cpu_id. */
  hp_globals.cpu_frequencies = NULL;
  hp_globals.cur_cpu_id = 0;

  hp_globals.stats_count = NULL;

  /* no free hp_entry_t structures to start with */
  hp_globals.entry_free_list = NULL;

  for (i = 0; i < 256; i++) {
    hp_globals.func_hash_counters[i] = 0;
  }

  hp_ignored_functions_filter_clear();
  hp_monitored_functions_filter_clear();

#if defined(DEBUG)
  /* To make it random number generator repeatable to ease testing. */
  srand(0);
#endif
  return SUCCESS;
}

/**
 * Module shutdown callback.
 */
PHP_MSHUTDOWN_FUNCTION(quanta_mon) {
  /* Make sure cpu_frequencies is free'ed. */
  clear_frequencies();

  /* free any remaining items in the free list */
  hp_free_the_free_list();

  UNREGISTER_INI_ENTRIES();

  return SUCCESS;
}

/**
 * Request init callback. 
 */
PHP_RINIT_FUNCTION(quanta_mon) {
// xxx VERIFIER qu'on puisse bien faire un hp_begin avec des param differents
// a chaque appel
// puis remplir hp_mode_evnets_only_beginfn_cb
  int mode;
  long  quanta_mon_flags = 0;                                    /* QuantaMon flags */
  zval *optional_array = NULL;         /* optional array arg: for future use */

  if (strstr(SG(request_info).cookie_data, hp_globals.full_profiling_cookie_trigger))
	mode = QUANTA_MON_MODE_HIERARCHICAL;
  else if (strstr(SG(request_info).cookie_data, hp_globals.magento_profiling_cookie_trigger))
	mode = QUANTA_MON_MODE_MAGENTO_PROFILING;
  else
	mode = QUANTA_MON_MODE_EVENTS_ONLY;

  hp_get_ignored_functions_from_arg(optional_array);
  hp_get_monitored_functions_fill();

  hp_begin(mode, quanta_mon_flags TSRMLS_CC);

  return SUCCESS;
}

/**
 * Request shutdown callback. Stop profiling and return.
 */
PHP_RSHUTDOWN_FUNCTION(quanta_mon) {
  hp_end(TSRMLS_C);
  return SUCCESS;
}

/**
 * Module info callback. Returns the quanta_mon version.
 */
PHP_MINFO_FUNCTION(quanta_mon)
{
  char buf[SCRATCH_BUF_LEN];
  char tmp[SCRATCH_BUF_LEN];
  int i;
  int len;

  php_info_print_table_start();
  php_info_print_table_header(2, "quanta_mon", QUANTA_MON_VERSION);
  len = snprintf(buf, SCRATCH_BUF_LEN, "%d", hp_globals.cpu_num);
  buf[len] = 0;
  php_info_print_table_header(2, "CPU num", buf);

  if (hp_globals.cpu_frequencies) {
    /* Print available cpu frequencies here. */
    php_info_print_table_header(2, "CPU logical id", " Clock Rate (MHz) ");
    for (i = 0; i < hp_globals.cpu_num; ++i) {
      len = snprintf(buf, SCRATCH_BUF_LEN, " CPU %d ", i);
      buf[len] = 0;
      len = snprintf(tmp, SCRATCH_BUF_LEN, "%f", hp_globals.cpu_frequencies[i]);
      tmp[len] = 0;
      php_info_print_table_row(2, buf, tmp);
    }
  }

  php_info_print_table_end();
}


/**
 * ***************************************************
 * COMMON HELPER FUNCTION DEFINITIONS AND LOCAL MACROS
 * ***************************************************
 */

static void hp_register_constants(INIT_FUNC_ARGS) {
  REGISTER_LONG_CONSTANT("QUANTA_MON_FLAGS_NO_BUILTINS",
                         QUANTA_MON_FLAGS_NO_BUILTINS,
                         CONST_CS | CONST_PERSISTENT);

  REGISTER_LONG_CONSTANT("QUANTA_MON_FLAGS_CPU",
                         QUANTA_MON_FLAGS_CPU,
                         CONST_CS | CONST_PERSISTENT);

  REGISTER_LONG_CONSTANT("QUANTA_MON_FLAGS_MEMORY",
                         QUANTA_MON_FLAGS_MEMORY,
                         CONST_CS | CONST_PERSISTENT);
}

/**
 * A hash function to calculate a 8-bit hash code for a function name.
 * This is based on a small modification to 'zend_inline_hash_func' by summing
 * up all bytes of the ulong returned by 'zend_inline_hash_func'.
 *
 * @param str, char *, string to be calculated hash code for.
 *
 * @author cjiang
 */
static inline uint8 hp_inline_hash(char * str) {
  ulong h = 5381;
  uint i = 0;
  uint8 res = 0;

  while (*str) {
    h += (h << 5);
    h ^= (ulong) *str++;
  }

  for (i = 0; i < sizeof(ulong); i++) {
    res += ((uint8 *)&h)[i];
  }
  return res;
}

/**
 * Parse the list of ignored functions from the zval argument.
 *
 * @author mpal
 */
static void hp_get_ignored_functions_from_arg(zval *args) {

  if (hp_globals.ignored_function_names) {
    hp_array_del(hp_globals.ignored_function_names);
  }

  if (args != NULL) {
    zval  *zresult = NULL;

    zresult = hp_zval_at_key("ignored_functions", args);
    hp_globals.ignored_function_names = hp_strings_in_zval(zresult);
  } else {
    hp_globals.ignored_function_names = NULL;
  }
}

/**
 * Clear filter for functions which may be ignored during profiling.
 *
 * @author mpal
 */
static void hp_ignored_functions_filter_clear() {
  memset(hp_globals.ignored_function_filter, 0,
         QUANTA_MON_IGNORED_FUNCTION_FILTER_SIZE);
}

/**
 * Initialize filter for ignored functions using bit vector.
 *
 * @author mpal
 */
static void hp_ignored_functions_filter_init() {
  if (hp_globals.ignored_function_names != NULL) {
    int i = 0;
    for(; hp_globals.ignored_function_names[i] != NULL; i++) {
      char *str  = hp_globals.ignored_function_names[i];
      uint8 hash = hp_inline_hash(str);
      int   idx  = INDEX_2_BYTE(hash);
      hp_globals.ignored_function_filter[idx] |= INDEX_2_BIT(hash);
    }
  }
  memset(hp_globals.monitored_function_tsc_start, 0, sizeof(hp_globals.monitored_function_tsc_start));
  memset(hp_globals.monitored_function_tsc_stop, 0, sizeof(hp_globals.monitored_function_tsc_stop));
}

/**
 * Check if function collides in filter of functions to be ignored.
 *
 * @author mpal
 */
int hp_ignored_functions_filter_collision(uint8 hash) {
  uint8 mask = INDEX_2_BIT(hash);
  return hp_globals.ignored_function_filter[INDEX_2_BYTE(hash)] & mask;
}

/* Special handling */
#define POS_ENTRY_GENERATEBLOCK 6
#define POS_ENTRY_BEFORETOHTML  7
#define POS_ENTRY_AFTERTOHTML   8
#define POS_ENTRY_EVENTS_ONLY   9 /* Anything below won't be processed unless the special cookie is set */
/**
*/
/**
 * Parse the list of monitored functions from the zval argument.
 *
 * @author ch
 */
static void hp_get_monitored_functions_fill() {
  /* Already initialized ? */
  if (hp_globals.monitored_function_names[0] != NULL) {
	return;
  }
  hp_globals.monitored_function_names[0] = "Mage::run";
  hp_globals.monitored_function_names[1] = "Mage_Core_Controller_Varien_Action::preDispatch";
  hp_globals.monitored_function_names[2] = "Mage_Core_Controller_Varien_Action::loadLayoutUpdates";
  hp_globals.monitored_function_names[3] = "Mage_Core_Controller_Varien_Action::renderLayout";
  hp_globals.monitored_function_names[4] = "Mage_Core_Controller_Varien_Action::postDispatch";
  hp_globals.monitored_function_names[5] = "Mage_Core_Controller_Response_Http::sendResponse";
  hp_globals.monitored_function_names[POS_ENTRY_GENERATEBLOCK] /* 6 */ = "Mage_Core_Model_Layout::_generateBlock";
  hp_globals.monitored_function_names[POS_ENTRY_BEFORETOHTML]  /* 7 */ = "Mage_Core_Block_Abstract::_beforeToHtml";
  hp_globals.monitored_function_names[POS_ENTRY_AFTERTOHTML]   /* 8 */ = "Mage_Core_Block_Abstract::_afterToHtml";
  /* POS_ENTRY_EVENTS_ONLY */
  hp_globals.monitored_function_names[POS_ENTRY_EVENTS_ONLY]   /* 9  */ = "Mage_Core_Model_Cache::flush";
  hp_globals.monitored_function_names[10]   /* 10 */ = "Mage_Core_Model_Cache::cleanType";
  hp_globals.monitored_function_names[11]   /* 11 */ = "Mage_Index_Model_Event::_beforeSave";
  hp_globals.monitored_function_names[12] = NULL;
  // Don't forget to change QUANTA_MON_MAX_MONITORED_FUNCTIONS. It must be equal to the last entry ([x] = NULL) + 1
}

/**
 * Clear filter for functions which may be ignored during profiling.
 *
 * @author ch
 */
static void hp_monitored_functions_filter_clear() {
  memset(hp_globals.monitored_function_filter, 0,
         QUANTA_MON_MONITORED_FUNCTION_FILTER_SIZE);
}

/**
 * Initialize filter for monitored functions using bit vector.
 *
 * @author ch
 */
static void hp_monitored_functions_filter_init() {
    int i = 0;
    for(; hp_globals.monitored_function_names[i] != NULL; i++) {
      char *str  = hp_globals.monitored_function_names[i];
      uint8 hash = hp_inline_hash(str);
      int   idx  = INDEX_2_BYTE(hash);
      hp_globals.monitored_function_filter[idx] |= INDEX_2_BIT(hash);
    }
    hp_globals.monitored_function_generate_renderize_block_first_linked_list = NULL;
    hp_globals.renderize_block_last_used = NULL;
}

/**
 * Check if function collides in filter of functions to be monitored
 *
 * @author ch
 */
int hp_monitored_functions_filter_collision(uint8 hash) {
  uint8 mask = INDEX_2_BIT(hash);
  return hp_globals.monitored_function_filter[INDEX_2_BYTE(hash)] & mask;
}


/**
 * Initialize profiler state
 *
 * @author kannan, veeve
 */
void hp_init_profiler_state(int level TSRMLS_DC) {
  /* Setup globals */
  if (!hp_globals.ever_enabled) {
    hp_globals.ever_enabled  = 1;
    hp_globals.entries = NULL;
  }
  hp_globals.profiler_level  = (int) level;

  if (level != QUANTA_MON_MODE_EVENTS_ONLY) {
	  /* Init stats_count */
	  if (hp_globals.stats_count) {
	    zval_dtor(hp_globals.stats_count);
	    FREE_ZVAL(hp_globals.stats_count);
	  }
	  MAKE_STD_ZVAL(hp_globals.stats_count);
	  array_init(hp_globals.stats_count);

	  /* NOTE(cjiang): some fields such as cpu_frequencies take relatively longer
	   * to initialize, (5 milisecond per logical cpu right now), therefore we
	   * calculate them lazily. */
	  if (hp_globals.cpu_frequencies == NULL) {
	    get_all_cpu_frequencies();
	    restore_cpu_affinity(&hp_globals.prev_mask);
	  }

	  /* bind to a random cpu so that we can use rdtsc instruction. */
	  bind_to_cpu((int) (rand() % hp_globals.cpu_num));
  }
  /* Call current mode's init cb */
  hp_globals.mode_cb.init_cb(TSRMLS_C);

  /* Set up filter of functions which may be ignored during profiling */
  hp_ignored_functions_filter_init();

  /* Set up filter of functions which are monitored */
  hp_monitored_functions_filter_init();
}

/**
 * Cleanup profiler state
 *
 * @author kannan, veeve
 */
void hp_clean_profiler_state(TSRMLS_D) {
  /* Call current mode's exit cb */
  hp_globals.mode_cb.exit_cb(TSRMLS_C);

  /* Clear globals */
  if (hp_globals.stats_count) {
    zval_dtor(hp_globals.stats_count);
    FREE_ZVAL(hp_globals.stats_count);
    hp_globals.stats_count = NULL;
  }
  hp_globals.entries = NULL;
  hp_globals.profiler_level = 1;
  hp_globals.ever_enabled = 0;

  /* Delete the array storing ignored function names */
  hp_array_del(hp_globals.ignored_function_names);
  hp_globals.ignored_function_names = NULL;

  /* We don't need to delete the array of monitored function names,
   * it's just pointers to static memory, unlike ignored function names
   * which are emalloced.
   *
   * Delete the array storing monitored function names
   hp_array_del(hp_globals.monitored_function_names);
   hp_globals.monitored_function_names = NULL;
  */
}

/*
 * Start profiling - called just before calling the actual function
 * NOTE:  PLEASE MAKE SURE TSRMLS_CC IS AVAILABLE IN THE CONTEXT
 *        OF THE FUNCTION WHERE THIS MACRO IS CALLED.
 *        TSRMLS_CC CAN BE MADE AVAILABLE VIA TSRMLS_DC IN THE
 *        CALLING FUNCTION OR BY CALLING TSRMLS_FETCH()
 *        TSRMLS_FETCH() IS RELATIVELY EXPENSIVE.
 *
 * return profile_curr with 
 *                          -1 if this function is not specifically profiled 
 *                          >=0 this function is specifically profiled, in this case contain the array position
 */
#define BEGIN_PROFILING(entries, symbol, profile_curr, pathname, execute_data)        \
  do {                                                                                \
    /* Use a hash code to filter most of the string comparisons. */                   \
    uint8 hash_code  = hp_inline_hash(symbol);                                        \
    if(hp_ignore_entry(hash_code, symbol))                                            \
	 profile_curr = -1;                                                           \
    else                                                                              \
         profile_curr = qm_record_timers_loading_time(hash_code, symbol, execute_data);    \
    if (hp_globals.profiler_level <= QUANTA_MON_MODE_SAMPLED) {                       \
      hp_entry_t *cur_entry = hp_fast_alloc_hprof_entry();                            \
      (cur_entry)->hash_code = hash_code;                                             \
      (cur_entry)->name_hprof = symbol;                                               \
      (cur_entry)->pathname_hprof = pathname;                                         \
      (cur_entry)->prev_hprof = (*(entries));                                         \
      /* Call the universal callback */                                               \
      hp_mode_common_beginfn((entries), (cur_entry) TSRMLS_CC);                       \
      /* Call the mode's beginfn callback */                                          \
      hp_globals.mode_cb.begin_fn_cb((entries), (cur_entry) TSRMLS_CC);               \
      /* Update entries linked list */                                                \
      (*(entries)) = (cur_entry);                                                     \
    }                                                                                 \
  } while (0)

/*
 * Stop profiling - called just after calling the actual function
 * NOTE:  PLEASE MAKE SURE TSRMLS_CC IS AVAILABLE IN THE CONTEXT
 *        OF THE FUNCTION WHERE THIS MACRO IS CALLED.
 *        TSRMLS_CC CAN BE MADE AVAILABLE VIA TSRMLS_DC IN THE
 *        CALLING FUNCTION OR BY CALLING TSRMLS_FETCH()
 *        TSRMLS_FETCH() IS RELATIVELY EXPENSIVE.
 */
#define END_PROFILING(entries, profile_curr, execute_data)                      \
  do {                                                                          \
    if (profile_curr >= 0) {                                                    \
         hp_globals.monitored_function_tsc_stop[profile_curr] = cycle_timer();  \
         if (profile_curr == POS_ENTRY_GENERATEBLOCK)                           \
             hp_globals.monitored_function_generate_renderize_block_last_linked_list->tsc_generate_stop = hp_globals.monitored_function_tsc_stop[profile_curr]; \
	else if (profile_curr == POS_ENTRY_AFTERTOHTML)                      \
             qm_after_tohmtl(execute_data);                                  \
    }                                                                        \
    if (hp_globals.profiler_level <= QUANTA_MON_MODE_SAMPLED) {              \
      hp_entry_t *cur_entry;                                                 \
      /* Call the mode's endfn callback. */                                  \
      /* NOTE(cjiang): we want to call this 'end_fn_cb' before */            \
      /* 'hp_mode_common_endfn' to avoid including the time in */            \
      /* 'hp_mode_common_endfn' in the profiling results.      */            \
      hp_globals.mode_cb.end_fn_cb((entries) TSRMLS_CC);                     \
      cur_entry = (*(entries));                                              \
      /* Call the universal callback */                                      \
      hp_mode_common_endfn((entries), (cur_entry) TSRMLS_CC);                \
      /* Free top entry and update entries linked list */                    \
      (*(entries)) = (*(entries))->prev_hprof;                               \
      hp_fast_free_hprof_entry(cur_entry);                                   \
    }                                                                        \
  } while (0)


/**
 * Returns formatted function name
 *
 * @param  entry        hp_entry
 * @param  result_buf   ptr to result buf
 * @param  result_len   max size of result buf
 * @return total size of the function name returned in result_buf
 * @author veeve
 */
size_t hp_get_entry_name(hp_entry_t  *entry,
                         char           *result_buf,
                         size_t          result_len) {

  /* Validate result_len */
  if (result_len <= 1) {
    /* Insufficient result_bug. Bail! */
    return 0;
  }

  /* Add '@recurse_level' if required */
  /* NOTE:  Dont use snprintf's return val as it is compiler dependent */
  if (entry->rlvl_hprof) {
    snprintf(result_buf, result_len,
             "%s@%d",
             entry->name_hprof, entry->rlvl_hprof);
  }
  else {
    snprintf(result_buf, result_len,
             "%s",
             entry->name_hprof);
  }

  /* Force null-termination at MAX */
  result_buf[result_len - 1] = 0;

  return strlen(result_buf);
}

/**
 * Check if this entry should be ignored, first with a conservative Bloomish
 * filter then with an exact check against the function names.
 *
 * @author mpal
 */
int  hp_ignore_entry_work(uint8 hash_code, char *curr_func) {
  int ignore = 0;
  if (hp_ignored_functions_filter_collision(hash_code)) {
    int i = 0;
    for (; hp_globals.ignored_function_names[i] != NULL; i++) {
      char *name = hp_globals.ignored_function_names[i];
      if ( !strcmp(curr_func, name)) {
        ignore++;
        break;
      }
    }
  }

  return ignore;
}

static inline int  hp_ignore_entry(uint8 hash_code, char *curr_func) {
  /* First check if ignoring functions is enabled */
  return hp_globals.ignored_function_names != NULL &&
         hp_ignore_entry_work(hash_code, curr_func);
}

/**
 * Extract relevant informations in the _generateBlock parameters
 *
 * @author ch
 */
static int qm_extract_param_generate_block(int i, zend_execute_data *execute_data TSRMLS_DC) {
  zval *param_node;
  const char *class_name;
  zend_uint class_name_len;
  zval *arr, *arr_final, **data, **data_final;
  HashTable *arr_hash, *arr_hash_final;
  HashPosition pointer, pointer_final;
  char *key, *tmpstrbuf;
  int key_len, typekey;
  long index;
  generate_renderize_block_details *current_gen_block_details;

  /* Do some sanity check, protect ourself against any modification of the magento core */
  /* For calling the debuger : __asm__("int3"); */
  if (!execute_data) {
	PRINTF_QUANTA ("_generateBlock: execute_data NULL\n");
	return -1;
  }
  if (!execute_data->prev_execute_data) {
	PRINTF_QUANTA ("_generateBlock: execute_data->prev_execute_data NULL\n");
	return -1;
  }
  if ((execute_data->prev_execute_data->function_state.arguments)[0] != (void*)2) {
	PRINTF_QUANTA ("_generateBlock: execute_data->prev_execute_data->function_state.arguments is not 2 (%p)\n", \
		(execute_data->prev_execute_data->function_state.arguments)[0]);
	return -1;
  }
  param_node = (zval *)(execute_data->prev_execute_data->function_state.arguments)[-2];
  if (!param_node) {
	PRINTF_QUANTA ("_generateBlock: execute_data->prev_execute_data->function_state.arguments[-2] NULL\n");
	return -1;
  }

  if (Z_TYPE_P(param_node) != IS_OBJECT) {
	PRINTF_QUANTA ("_generateBlock: arguments[-2] is not an object\n");
	return -1;
  }

  if (!Z_OBJ_HANDLER(*param_node, get_class_name)) {
	PRINTF_QUANTA ("_generateBlock: arguments[-2] is an object of unknown class\n");
	return -1;
  }
  Z_OBJ_HANDLER(*param_node, get_class_name)(param_node, &class_name, &class_name_len, 0 TSRMLS_CC);
  if ((class_name_len != 30) && memcmp(class_name, "Mage_Core_Model_Layout_Element", 30) )
  {
	PRINTF_QUANTA ("_generateBlock: arguments[-1] is not a Mage_Core_Model_Layout_Element object (%s)\n", class_name);
	efree((char*)class_name);
	return -1;
  }
  efree((char*)class_name);

  /* Okay, we should have something that looks like what we want, now entering into the object
   * at first we have a single entry (@attributes) which is an array, grab the pointer and enumerate the array in the array */
  arr_hash = Z_OBJPROP_P(param_node);
  /* If we want to get a count : array_count = zend_hash_num_elements(arr_hash); */

  for (zend_hash_internal_pointer_reset_ex(arr_hash, &pointer);
			 zend_hash_get_current_data_ex(arr_hash, (void**) &data, &pointer) == SUCCESS;
			 zend_hash_move_forward_ex(arr_hash, &pointer)) {

	if (zend_hash_get_current_key_ex(arr_hash, &key, &key_len, &index, 0, &pointer) == HASH_KEY_IS_STRING) {
		if ((key_len != 11) && memcmp(key, "@attributes", 11)) {
#ifdef _QUANTA_MODULE_ULTRA_VERBOSE
			/* Sometimes we seen label, action, block */
			PRINTF_QUANTA ("_generateBlock: Unknown outer key '%s', skipping....\n", key);
#endif
			continue;
		}
	} else {
		PRINTF_QUANTA ("_generateBlock: Hash key is not a string (%ld)\n", index);
		continue;
	}

	if (Z_TYPE_PP(data) != IS_ARRAY) {
		PRINTF_QUANTA ("_generateBlock: Object doesn't contain an array, skipping... (type=%d)\n", Z_TYPE_PP(data));
		continue;
	}
	current_gen_block_details = ecalloc (1, sizeof(generate_renderize_block_details));

	if (hp_globals.monitored_function_generate_renderize_block_first_linked_list == NULL)
		hp_globals.monitored_function_generate_renderize_block_first_linked_list = current_gen_block_details;
	else
		hp_globals.monitored_function_generate_renderize_block_last_linked_list->next_generate_renderize_block_detail = current_gen_block_details;

	hp_globals.monitored_function_generate_renderize_block_last_linked_list = current_gen_block_details;

	current_gen_block_details->tsc_generate_start = hp_globals.monitored_function_tsc_start[i];

	arr_hash_final = Z_ARRVAL_PP(data);
	for (zend_hash_internal_pointer_reset_ex(arr_hash_final, &pointer_final);
			 zend_hash_get_current_data_ex(arr_hash_final, (void**) &data_final, &pointer_final) == SUCCESS;
			 zend_hash_move_forward_ex(arr_hash_final, &pointer_final)) {

		if (Z_TYPE_PP(data_final) != IS_STRING) {
			PRINTF_QUANTA ("_generateBlock: Final array doesn't contain a string, skipping... (type=%d)\n", Z_TYPE_PP(data_final));
			continue;
		}
		if (zend_hash_get_current_key_ex(arr_hash_final, &key, &key_len, &index, 0, &pointer_final) != HASH_KEY_IS_STRING) {
			PRINTF_QUANTA ("_generateBlock: Hash final key is numeric (%ld) => %s\n", index, Z_STRVAL_PP(data_final));
			continue;
		}
#ifdef _QUANTA_MODULE_ULTRA_VERBOSE
		PRINTF_QUANTA ("_generateBlock: final key '%s'=>'%s'\n", key, Z_STRVAL_PP(data_final)); //, Z_STRLEN_PP(data));
#endif

		typekey = 0;
		switch(key_len) {
			case 5:
				if (!memcmp(key, "type", 4))
					typekey = 1;
				else if (!memcmp(key, "name", 4))
					typekey = 2;
				break;
			case 6:
				if (!memcmp(key, "class", 5))
					typekey = 3;
				break;
			case 9:
				if (!memcmp(key, "template", 8))
					typekey = 4;
				break;
			case 12:
				if (!memcmp(key, "@attributes", 11))
					typekey = -1;
				break;
			default:
				break;
		}

		if (typekey == -1)
			continue;

		if (typekey == 0) {
#ifdef _QUANTA_MODULE_ULTRA_VERBOSE
			/* We also see keys : as, translate, before, after, output */
			PRINTF_QUANTA ("_generateBlock: Unknown final key='%s'=>'%s', skipping....\n", key, Z_STRVAL_PP(data_final));
#endif
			continue;
		}
		index = strlen (Z_STRVAL_PP(data_final));
		tmpstrbuf = emalloc (index + 1);

		memcpy (tmpstrbuf, Z_STRVAL_PP(data_final), index);
		tmpstrbuf[index] = 0;

		switch (typekey) {
			case 1:	current_gen_block_details->type = tmpstrbuf;  break;
			case 2:	current_gen_block_details->name = tmpstrbuf;  break;
			case 3:	current_gen_block_details->class = tmpstrbuf;  break;
			case 4:	current_gen_block_details->template = tmpstrbuf;  break;
			default: PRINTF_QUANTA ("_generateBlock: ** BAD ** Unknown typekey (%d)\n", typekey); break;
		}

	} /* for - inner data */
  } /* for - outer data */
  return i;
}

static int qm_extract_this_after_tohtml_do (char *name_in_layout)
{
	generate_renderize_block_details *current_render_block_details;

	if (hp_globals.renderize_block_last_used) {
		/* Check if the last linked list is still the correct one */
		if (!strcmp(hp_globals.renderize_block_last_used->name, name_in_layout))
		{
			/* Yes, we can use the shortcut */
#ifdef _QUANTA_MODULE_ULTRA_VERBOSE
			PRINTF_QUANTA ("_afterToHtml:  Block name '%s' was just processed by _beforeToHtml. Using shortcut.\n", name_in_layout);
#endif
			hp_globals.renderize_block_last_used->tsc_renderize_last_stop = cycle_timer();
			return 1;
		}
	}
	/* No, we are in a recursive call, we have to fallback to linked-list lookup */
	current_render_block_details = hp_globals.monitored_function_generate_renderize_block_first_linked_list;

	while (current_render_block_details) {
		if (!strcmp(current_render_block_details->name, name_in_layout))
			break;
		current_render_block_details = current_render_block_details->next_generate_renderize_block_detail;
	}
	if (!current_render_block_details) {
		/* Sometimes we get ANONYMOUS_xx blocks, ignore them */
		if (strlen(name_in_layout) > 10) {
			if (!memcmp(name_in_layout, "ANONYMOUS_", 10))
				return 0;
		}
		PRINTF_QUANTA ("_afterToHtml:  Block name '%s' just finished the renderize phase but was not seen in the _generateBlock phase.\n", name_in_layout);
		return 0;
	}
#ifdef _QUANTA_MODULE_ULTRA_VERBOSE
	PRINTF_QUANTA ("_afterToHtml:  Block name '%s' was NOT the last block processed by _beforeToHtml. Using slow path code.\n", name_in_layout);
#endif
	current_render_block_details->tsc_renderize_last_stop = cycle_timer();
  return 1;
}

static int qm_extract_this_before_tohtml_do (const char *class_name, zend_uint class_name_len, char *name_in_layout, char *template)
{
  int linked_list_pos = 1;
  generate_renderize_block_details *current_render_block_details;

  /* Ok, we have something, search into our linked list for nameInLayout */
  current_render_block_details = hp_globals.monitored_function_generate_renderize_block_first_linked_list;

  linked_list_pos = 1;
  while (current_render_block_details) {
	if (!strcmp(current_render_block_details->name, name_in_layout))
	{
		if (!current_render_block_details->class) {
			current_render_block_details->class = emalloc(class_name_len + 1);
			memcpy (current_render_block_details->class, class_name, class_name_len);
			current_render_block_details->class[class_name_len] = 0;
		}
		/* Usually already filled by _generateBlock, but sometimes it's missing */
		if ((!current_render_block_details->template) && (template))
			current_render_block_details->template = template;
		else
			efree(template);
		if (!current_render_block_details->tsc_renderize_first_start)
			current_render_block_details->tsc_renderize_first_start = cycle_timer();

		hp_globals.renderize_block_last_used = current_render_block_details;
#ifdef _QUANTA_MODULE_ULTRA_VERBOSE
		PRINTF_QUANTA ("_beforeToHtml: Block name '%s' found in the _generateBlock list at position %d\n", name_in_layout, linked_list_pos);
#endif
  		return 1;
	}
	linked_list_pos++;
	current_render_block_details = current_render_block_details->next_generate_renderize_block_detail;
  }
  /* Sometimes we get ANONYMOUS_xx blocks, ignore them */
  if (strlen(name_in_layout) > 10) {
	if (!memcmp(name_in_layout, "ANONYMOUS_", 10))
		return 0;
  }
  PRINTF_QUANTA ("_beforeToHtml: Trying to renderize block %s not seen in _generateBlock. Ignoring.\n", name_in_layout);
  if (template)
 	efree(template);
  return 0;
}

static int qm_extract_this_before_after_tohtml (int is_after_tohtml, zend_execute_data *execute_data TSRMLS_DC)
{
  zval *param_node, **data;
  const char *class_name;
  zend_uint class_name_len;
  HashTable *arr_hash;
  HashPosition pointer, pointer_final;
  char *key, *tmpstrbuf, *name_in_layout, *template = NULL;
  int key_len, typekey, i, ret;
  long index;

  /* Do some sanity check, protect ourself against any modification of the magento core */
  /* For calling the debuger : __asm__("int3"); */
  if (!execute_data) {
	PRINTF_QUANTA ("before/afterToHtml: execute_data NULL\n");
	return -1;
  }
  if (!execute_data->prev_execute_data) {
	PRINTF_QUANTA ("before/afterToHtml: execute_data->prev_execute_data NULL\n");
	return -1;
  }
  param_node = execute_data->prev_execute_data->current_this;
  if (Z_TYPE_P(param_node) != IS_OBJECT) {
	PRINTF_QUANTA ("before/afterToHtml: 'this' is not an object\n");
	return -1;
  }

  if (!Z_OBJ_HANDLER(*param_node, get_class_name)) {
	PRINTF_QUANTA ("before/afterToHtml: 'this' is an object of unknown class\n");
	return -1;
  }
  Z_OBJ_HANDLER(*param_node, get_class_name)(param_node, &class_name, &class_name_len, 0 TSRMLS_CC);
#ifdef _QUANTA_MODULE_ULTRA_VERBOSE
	PRINTF_QUANTA ("before/afterToHtml: 'this' class=%s\n", class_name);
#endif

  arr_hash = Z_OBJPROP_P(param_node);
  /* If we want to get a count : array_count = zend_hash_num_elements(arr_hash); */

  typekey = 0;

  for (zend_hash_internal_pointer_reset_ex(arr_hash, &pointer);
			 zend_hash_get_current_data_ex(arr_hash, (void**) &data, &pointer) == SUCCESS;
			 zend_hash_move_forward_ex(arr_hash, &pointer)) {

	int current_type_key = 0;
	if (zend_hash_get_current_key_ex(arr_hash, &key, &key_len, &index, 0, &pointer) != HASH_KEY_IS_STRING) {
		PRINTF_QUANTA ("before/afterToHtml: Hash key 'this' is not string (%ld)\n", index);
		continue;
	}
	if ((key_len == 17) && !memcmp(key, "\0*\0_nameInLayout", 17)) {
		typekey |= 0x1;
		current_type_key = 1;
	}
	else if ((key_len == 13) && !memcmp(key, "\0*\0_template", 13)) {
		if (Z_TYPE_PP(data) == IS_NULL)
			continue;
		typekey |= 0x2;
		current_type_key = 2;
	}
	else {
#ifdef _QUANTA_MODULE_ULTRA_VERBOSE
		/* Enable this code for dumping members name of the current class */
		if (key_len > 3)
			PRINTF_QUANTA ("before/afterToHtml: Ignoring 'this' key+3 '%s' of len %d\n", key+3, key_len);
		else
			PRINTF_QUANTA ("before/afterToHtml: Ignoring 'this' key of len %d\n", key_len);
#endif
		continue;
	}

	if (Z_TYPE_PP(data) != IS_STRING) {
		PRINTF_QUANTA ("before/afterToHtml: Key %s has a value which is not a string (%d)\n", key+3, Z_TYPE_PP(data));
__asm__("int3");
		continue;
	}
#ifdef _QUANTA_MODULE_ULTRA_VERBOSE
	PRINTF_QUANTA ("before/afterToHtml: typekey=%d value=%s\n", typekey, Z_STRVAL_PP(data));
#endif
	index = strlen (Z_STRVAL_PP(data));
	tmpstrbuf = emalloc (index + 1);
	memcpy (tmpstrbuf, Z_STRVAL_PP(data), index);
	tmpstrbuf[index] = 0;
	if (current_type_key == 1) {
		name_in_layout = tmpstrbuf;
		if (is_after_tohtml)
			break; /* We don't need anything more (like template) here */
	}
	else
		template = tmpstrbuf;

  }
  if (typekey == 0x0) {
	PRINTF_QUANTA ("before/afterToHtml: 'this' doesn't have a member 'nameInLayout'\n");
  	efree((char*)class_name);
  	return i;
  }
  else if (typekey == 0x2) {
	PRINTF_QUANTA ("before/afterToHtml: 'this' doesnt have a member 'nameInLayout' but do have 'template'\n");
	efree(template);
  	efree((char*)class_name);
  	return i;
  }
  if (is_after_tohtml)
	ret = qm_extract_this_after_tohtml_do(name_in_layout);
  else
	ret = qm_extract_this_before_tohtml_do(class_name, class_name_len, name_in_layout, template);

  efree(name_in_layout);
  efree((char*)class_name);
  return i;
}

/**
 * Extract relevant informations in the _generateBlock parameters
 *
 * @author ch
 */
static int qm_extract_this_before_tohtml(zend_execute_data *execute_data TSRMLS_DC)
{
  return qm_extract_this_before_after_tohtml(0, execute_data);
}

static int qm_after_tohmtl(zend_execute_data *execute_data TSRMLS_DC)
{
  return qm_extract_this_before_after_tohtml(1, execute_data);
}

/**
 * Check if this entry should be ignored, first with a conservative Bloomish
 * filter then with an exact check against the function names.
 *
 * @author ch
 * return -1 if we don't monitor specifically this function, -2 if we don't monitor at all
 */
int  qm_record_timers_loading_time(uint8 hash_code, char *curr_func, zend_execute_data *execute_data TSRMLS_DC) {
  int i;

  /* Search quickly if we may have a match */
  if (!hp_monitored_functions_filter_collision(hash_code))
		return -1;

  if (hp_globals.profiler_level == QUANTA_MON_MODE_EVENTS_ONLY)
	i = POS_ENTRY_EVENTS_ONLY;
  else
	i = 0;

  /* We MAY have a match, enumerate the function array for an exact match */
  for (; hp_globals.monitored_function_names[i] != NULL; i++) {
      char *name = hp_globals.monitored_function_names[i];
      if ( !strcmp(curr_func, name))
        break;
  }

  if ( hp_globals.monitored_function_names[i] == NULL)
		return -1; /* False match, we have nothing */

  hp_globals.monitored_function_tsc_start[i] = cycle_timer();

  if ( i == POS_ENTRY_BEFORETOHTML )
     return qm_extract_this_before_tohtml(execute_data);

  /* Something special for this function ? */
  if ( i == POS_ENTRY_GENERATEBLOCK )
     return qm_extract_param_generate_block(i, execute_data);
  return i; /* No, bailout */
}

/**
 * Build a caller qualified name for a callee.
 *
 * For example, if A() is caller for B(), then it returns "A==>B".
 * Recursive invokations are denoted with @<n> where n is the recursion
 * depth.
 *
 * For example, "foo==>foo@1", and "foo@2==>foo@3" are examples of direct
 * recursion. And  "bar==>foo@1" is an example of an indirect recursive
 * call to foo (implying the foo() is on the call stack some levels
 * above).
 *
 * @author kannan, veeve
 */
size_t hp_get_function_stack(hp_entry_t *entry,
                             int            level,
                             char          *result_buf,
                             size_t         result_len) {
  size_t         len = 0;

  /* End recursion if we dont need deeper levels or we dont have any deeper
   * levels */
  if (!entry->prev_hprof || (level <= 1)) {
    return hp_get_entry_name(entry, result_buf, result_len);
  }

  /* Take care of all ancestors first */
  len = hp_get_function_stack(entry->prev_hprof,
                              level - 1,
                              result_buf,
                              result_len);

  /* Append the delimiter */
# define    HP_STACK_DELIM        "==>"
# define    HP_STACK_DELIM_LEN    (sizeof(HP_STACK_DELIM) - 1)

  if (result_len < (len + HP_STACK_DELIM_LEN)) {
    /* Insufficient result_buf. Bail out! */
    return len;
  }

  /* Add delimiter only if entry had ancestors */
  if (len) {
    strncat(result_buf + len,
            HP_STACK_DELIM,
            result_len - len);
    len += HP_STACK_DELIM_LEN;
  }

# undef     HP_STACK_DELIM_LEN
# undef     HP_STACK_DELIM

  /* Append the current function name */
  return len + hp_get_entry_name(entry,
                                 result_buf + len,
                                 result_len - len);
}

/**
 * Takes an input of the form /a/b/c/d/foo.php and returns
 * a pointer to one-level directory and basefile name
 * (d/foo.php) in the same string.
 */
static const char *hp_get_base_filename(const char *filename) {
  const char *ptr;
  int   found = 0;

  if (!filename)
    return "";

  /* ch: we want the complete path, so return directly */
  /* reverse search for "/" and return a ptr to the next char */
  /*
  for (ptr = filename + strlen(filename) - 1; ptr >= filename; ptr--) {
    if (*ptr == '/') {
      found++;
    }
    if (found == 2) {
      return ptr + 1;
    }
  }
  */
  /* no "/" char found, so return the whole string */
  return filename;
}

/**
 * Get the name of the current function. The name is qualified with
 * the class name if the function is in a class.
 *
 * @author kannan, hzhao
 */
static char *hp_get_function_name(zend_op_array *ops, char **pathname TSRMLS_DC) {
  zend_execute_data *data;
  const char        *func = NULL;
  const char        *cls = NULL;
  char              *ret = NULL;
  int                len;
  zend_function      *curr_func;

  *pathname = "BUG";
  data = EG(current_execute_data);

  if (data) {
    /* shared meta data for function on the call stack */
    curr_func = data->function_state.function;

    /* extract function name from the meta info */
    func = curr_func->common.function_name;

    if (func) {
      /* previously, the order of the tests in the "if" below was
       * flipped, leading to incorrect function names in profiler
       * reports. When a method in a super-type is invoked the
       * profiler should qualify the function name with the super-type
       * class name (not the class name based on the run-time type
       * of the object.
       */
      if (curr_func->common.scope) {
        cls = curr_func->common.scope->name;
      } else if (data->object) {
        cls = Z_OBJCE(*data->object)->name;
      }

      if (cls) {
        const char *filename;
        int   len;
        if (ops)
	   filename = hp_get_base_filename(ops->filename);
        else
           filename = "null";
        *pathname = strdup(filename);
        len = strlen(cls) + strlen(func) + 8;
        ret = (char*)emalloc(len);
        snprintf(ret, len, "%s::%s", cls, func);
      } else {
        ret = estrdup(func);
      }
    } else {
      long     curr_op;
      int      add_filename = 1;

      /* we are dealing with a special directive/function like
       * include, eval, etc.
       */
#if ZEND_EXTENSION_API_NO >= 220121212
      if (data->prev_execute_data) {
        curr_op = data->prev_execute_data->opline->extended_value;
      } else {
        curr_op = data->opline->extended_value;
      }
#elif ZEND_EXTENSION_API_NO >= 220100525
      curr_op = data->opline->extended_value;
#else
      curr_op = data->opline->op2.u.constant.value.lval;
#endif

      switch (curr_op) {
        case ZEND_EVAL:
          func = "eval";
          break;
        case ZEND_INCLUDE:
          func = "include";
          add_filename = 1;
          break;
        case ZEND_REQUIRE:
          func = "require";
          add_filename = 1;
          break;
        case ZEND_INCLUDE_ONCE:
          func = "include_once";
          add_filename = 1;
          break;
        case ZEND_REQUIRE_ONCE:
          func = "require_once";
          add_filename = 1;
          break;
        default:
          func = "???_op";
          break;
      }

      /* For some operations, we'll add the filename as part of the function
       * name to make the reports more useful. So rather than just "include"
       * you'll see something like "run_init::foo.php" in your reports.
       */
      if (add_filename){
        const char *filename;
        int   len;
        filename = hp_get_base_filename((curr_func->op_array).filename);
        len      = strlen("run_init") + strlen(filename) + 3;
        ret      = (char *)emalloc(len);
        snprintf(ret, len, "run_init::%s", filename);
        *pathname = strdup(filename);
      } else {
        ret = estrdup(func);
      }
    }
  }
  return ret;
}

/**
 * Free any items in the free list.
 */
static void hp_free_the_free_list() {
  hp_entry_t *p = hp_globals.entry_free_list;
  hp_entry_t *cur;

  while (p) {
    cur = p;
    p = p->prev_hprof;
    free(cur);
  }
}

/**
 * Fast allocate a hp_entry_t structure. Picks one from the
 * free list if available, else does an actual allocate.
 *
 * Doesn't bother initializing allocated memory.
 *
 * @author kannan
 */
static hp_entry_t *hp_fast_alloc_hprof_entry() {
  hp_entry_t *p;

  p = hp_globals.entry_free_list;

  if (p) {
    hp_globals.entry_free_list = p->prev_hprof;
    return p;
  } else {
    return (hp_entry_t *)malloc(sizeof(hp_entry_t));
  }
}

/**
 * Fast free a hp_entry_t structure. Simply returns back
 * the hp_entry_t to a free list and doesn't actually
 * perform the free.
 *
 * @author kannan
 */
static void hp_fast_free_hprof_entry(hp_entry_t *p) {

  /* we use/overload the prev_hprof field in the structure to link entries in
   * the free list. */
  p->prev_hprof = hp_globals.entry_free_list;
  hp_globals.entry_free_list = p;
}

/**
 * Increment the count of the given stat with the given count
 * If the stat was not set before, inits the stat to the given count
 *
 * @param  zval *counts   Zend hash table pointer
 * @param  char *name     Name of the stat
 * @param  long  count    Value of the stat to incr by
 * @return void
 * @author kannan
 */
void hp_inc_count(zval *counts, char *name, long count TSRMLS_DC) {
  HashTable *ht;
  void *data;

  if (!counts) return;
  ht = HASH_OF(counts);
  if (!ht) return;

  if (zend_hash_find(ht, name, strlen(name) + 1, &data) == SUCCESS) {
    ZVAL_LONG(*(zval**)data, Z_LVAL_PP((zval**)data) + count);
  } else {
    add_assoc_long(counts, name, count);
  }
}

/**
 * Looksup the hash table for the given symbol
 * Initializes a new array() if symbol is not present
 *
 * @author kannan, veeve
 */
zval * hp_hash_lookup(char *symbol  TSRMLS_DC) {
  HashTable   *ht;
  void        *data;
  zval        *counts = (zval *) 0;

  /* Bail if something is goofy */
  if (!hp_globals.stats_count || !(ht = HASH_OF(hp_globals.stats_count))) {
    return (zval *) 0;
  }

  /* Lookup our hash table */
  if (zend_hash_find(ht, symbol, strlen(symbol) + 1, &data) == SUCCESS) {
    /* Symbol already exists */
    counts = *(zval **) data;
  }
  else {
    /* Add symbol to hash table */
    MAKE_STD_ZVAL(counts);
    array_init(counts);
    add_assoc_zval(hp_globals.stats_count, symbol, counts);
  }

  return counts;
}

/**
 * Truncates the given timeval to the nearest slot begin, where
 * the slot size is determined by intr
 *
 * @param  tv       Input timeval to be truncated in place
 * @param  intr     Time interval in microsecs - slot width
 * @return void
 * @author veeve
 */
void hp_trunc_time(struct timeval *tv,
                   uint64          intr) {
  uint64 time_in_micro;

  /* Convert to microsecs and trunc that first */
  time_in_micro = (tv->tv_sec * 1000000) + tv->tv_usec;
  time_in_micro /= intr;
  time_in_micro *= intr;

  /* Update tv */
  tv->tv_sec  = (time_in_micro / 1000000);
  tv->tv_usec = (time_in_micro % 1000000);
}

/**
 * Sample the stack. Add it to the stats_count global.
 *
 * @param  tv            current time
 * @param  entries       func stack as linked list of hp_entry_t
 * @return void
 * @author veeve
 */
void hp_sample_stack(hp_entry_t  **entries  TSRMLS_DC) {
  char key[SCRATCH_BUF_LEN];
  char symbol[SCRATCH_BUF_LEN * 1000];

  /* Build key */
  snprintf(key, sizeof(key),
           "%d.%06d",
           hp_globals.last_sample_time.tv_sec,
           hp_globals.last_sample_time.tv_usec);

  /* Init stats in the global stats_count hashtable */
  hp_get_function_stack(*entries,
                        INT_MAX,
                        symbol,
                        sizeof(symbol));

  add_assoc_string(hp_globals.stats_count,
                   key,
                   symbol,
                   1);
  return;
}

/**
 * Checks to see if it is time to sample the stack.
 * Calls hp_sample_stack() if its time.
 *
 * @param  entries        func stack as linked list of hp_entry_t
 * @param  last_sample    time the last sample was taken
 * @param  sampling_intr  sampling interval in microsecs
 * @return void
 * @author veeve
 */
void hp_sample_check(hp_entry_t **entries  TSRMLS_DC) {
  /* Validate input */
  if (!entries || !(*entries)) {
    return;
  }

  /* See if its time to sample.  While loop is to handle a single function
   * taking a long time and passing several sampling intervals. */
  while ((cycle_timer() - hp_globals.last_sample_tsc)
         > hp_globals.sampling_interval_tsc) {

    /* bump last_sample_tsc */
    hp_globals.last_sample_tsc += hp_globals.sampling_interval_tsc;

    /* bump last_sample_time - HAS TO BE UPDATED BEFORE calling hp_sample_stack */
    incr_us_interval(&hp_globals.last_sample_time, QUANTA_MON_SAMPLING_INTERVAL);

    /* sample the stack */
    hp_sample_stack(entries  TSRMLS_CC);
  }

  return;
}


/**
 * ***********************
 * High precision timer related functions.
 * ***********************
 */

/**
 * Get time stamp counter (TSC) value via 'rdtsc' instruction.
 *
 * @return 64 bit unsigned integer
 * @author cjiang
 */
static inline uint64 cycle_timer() {
  uint32 __a,__d;
  uint64 val;
  asm volatile("rdtsc" : "=a" (__a), "=d" (__d));
  (val) = ((uint64)__a) | (((uint64)__d)<<32);
  return val;
}

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
int bind_to_cpu(uint32 cpu_id) {
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
 * Get time delta in microseconds.
 */
static long get_us_interval(struct timeval *start, struct timeval *end) {
  return (((end->tv_sec - start->tv_sec) * 1000000)
          + (end->tv_usec - start->tv_usec));
}

/**
 * Incr time with the given microseconds.
 */
static void incr_us_interval(struct timeval *start, uint64 incr) {
  incr += (start->tv_sec * 1000000 + start->tv_usec);
  start->tv_sec  = incr/1000000;
  start->tv_usec = incr%1000000;
  return;
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
static inline double get_us_from_tsc(uint64 count, double cpu_frequency) {
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
static inline uint64 get_tsc_from_us(uint64 usecs, double cpu_frequency) {
  return (uint64) (usecs * cpu_frequency);
}

/**
 * This is a microbenchmark to get cpu frequency the process is running on. The
 * returned value is used to convert TSC counter values to microseconds.
 *
 * @return double.
 * @author cjiang
 */
static double get_cpu_frequency() {
  struct timeval start;
  struct timeval end;

  if (gettimeofday(&start, 0)) {
    perror("gettimeofday");
    return 0.0;
  }
  uint64 tsc_start = cycle_timer();
  /* Sleep for 5 miliseconds. Comparaing with gettimeofday's  few microseconds
   * execution time, this should be enough. */
  usleep(5000);
  if (gettimeofday(&end, 0)) {
    perror("gettimeofday");
    return 0.0;
  }
  uint64 tsc_end = cycle_timer();
  return (tsc_end - tsc_start) * 1.0 / (get_us_interval(&start, &end));
}

/**
 * Calculate frequencies for all available cpus.
 *
 * @author cjiang
 */
static void get_all_cpu_frequencies() {
  int id;
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
 * Reclaim the memory allocated for cpu_frequencies.
 *
 * @author cjiang
 */
static void clear_frequencies() {
  if (hp_globals.cpu_frequencies) {
    free(hp_globals.cpu_frequencies);
    hp_globals.cpu_frequencies = NULL;
  }
  restore_cpu_affinity(&hp_globals.prev_mask);
}


/**
 * ***************************
 * QUANTA_MON DUMMY CALLBACKS
 * ***************************
 */
void hp_mode_dummy_init_cb(TSRMLS_D) { }


void hp_mode_dummy_exit_cb(TSRMLS_D) { }


void hp_mode_dummy_beginfn_cb(hp_entry_t **entries,
                              hp_entry_t *current  TSRMLS_DC) { }

void hp_mode_dummy_endfn_cb(hp_entry_t **entries   TSRMLS_DC) { }


/**
 * ****************************
 * QUANTA_MON COMMON CALLBACKS
 * ****************************
 */
/**
 * QUANTA_MON universal begin function.
 * This function is called for all modes before the
 * mode's specific begin_function callback is called.
 *
 * @param  hp_entry_t **entries  linked list (stack)
 *                                  of hprof entries
 * @param  hp_entry_t  *current  hprof entry for the current fn
 * @return void
 * @author kannan, veeve
 */
void hp_mode_common_beginfn(hp_entry_t **entries,
                            hp_entry_t  *current  TSRMLS_DC) {
  hp_entry_t   *p;

  /* This symbol's recursive level */
  int    recurse_level = 0;

  if (hp_globals.func_hash_counters[current->hash_code] > 0) {
    /* Find this symbols recurse level */
    for(p = (*entries); p; p = p->prev_hprof) {
      if (!strcmp(current->name_hprof, p->name_hprof)) {
        recurse_level = (p->rlvl_hprof) + 1;
        break;
      }
    }
  }
  hp_globals.func_hash_counters[current->hash_code]++;

  /* Init current function's recurse level */
  current->rlvl_hprof = recurse_level;
}

/**
 * QUANTA_MON universal end function.  This function is called for all modes after
 * the mode's specific end_function callback is called.
 *
 * @param  hp_entry_t **entries  linked list (stack) of hprof entries
 * @return void
 * @author kannan, veeve
 */
void hp_mode_common_endfn(hp_entry_t **entries, hp_entry_t *current TSRMLS_DC) {
  hp_globals.func_hash_counters[current->hash_code]--;
}


/**
 * *********************************
 * QUANTA_MON INIT MODULE CALLBACKS
 * *********************************
 */
/**
 * QUANTA_MON_MODE_SAMPLED's init callback
 *
 * @author veeve
 */
void hp_mode_sampled_init_cb(TSRMLS_D) {
  struct timeval  now;
  uint64 truncated_us;
  uint64 truncated_tsc;
  double cpu_freq = hp_globals.cpu_frequencies[hp_globals.cur_cpu_id];

  /* Init the last_sample in tsc */
  hp_globals.last_sample_tsc = cycle_timer();

  /* Find the microseconds that need to be truncated */
  gettimeofday(&hp_globals.last_sample_time, 0);
  now = hp_globals.last_sample_time;
  hp_trunc_time(&hp_globals.last_sample_time, QUANTA_MON_SAMPLING_INTERVAL);

  /* Subtract truncated time from last_sample_tsc */
  truncated_us  = get_us_interval(&hp_globals.last_sample_time, &now);
  truncated_tsc = get_tsc_from_us(truncated_us, cpu_freq);
  if (hp_globals.last_sample_tsc > truncated_tsc) {
    /* just to be safe while subtracting unsigned ints */
    hp_globals.last_sample_tsc -= truncated_tsc;
  }

  /* Convert sampling interval to ticks */
  hp_globals.sampling_interval_tsc =
    get_tsc_from_us(QUANTA_MON_SAMPLING_INTERVAL, cpu_freq);
}


/**
 * ************************************
 * QUANTA_MON BEGIN FUNCTION CALLBACKS
 * ************************************
 */

/**
 * QUANTA_MON_MODE_HIERARCHICAL's begin function callback
 *
 * @author kannan
 */
void hp_mode_hier_beginfn_cb(hp_entry_t **entries,
                             hp_entry_t  *current  TSRMLS_DC) {
  /* Get start tsc counter */
  current->tsc_start = cycle_timer();

  /* Get CPU usage */
  if (hp_globals.quanta_mon_flags & QUANTA_MON_FLAGS_CPU) {
    getrusage(RUSAGE_SELF, &(current->ru_start_hprof));
  }

  /* Get memory usage */
  if (hp_globals.quanta_mon_flags & QUANTA_MON_FLAGS_MEMORY) {
    current->mu_start_hprof  = zend_memory_usage(0 TSRMLS_CC);
    current->pmu_start_hprof = zend_memory_peak_usage(0 TSRMLS_CC);
  }
}

/**
 * QUANTA_MON_MODE_MAGENTO_PROFILING's begin function callback
 *
 * @author ch
 */
void hp_mode_magento_profil_beginfn_cb(hp_entry_t **entries,
                             hp_entry_t  *current  TSRMLS_DC) {
}
/**
 * QUANTA_MON_MODE_EVENTS_ONLY's begin function callback
 *
 * @author ch
 */
void hp_mode_events_only_beginfn_cb(hp_entry_t **entries,
                             hp_entry_t  *current  TSRMLS_DC) {
}

/**
 * QUANTA_MON_MODE_SAMPLED's begin function callback
 *
 * @author veeve
 */
void hp_mode_sampled_beginfn_cb(hp_entry_t **entries,
                                hp_entry_t  *current  TSRMLS_DC) {
  /* See if its time to take a sample */
  hp_sample_check(entries  TSRMLS_CC);
}


/**
 * **********************************
 * QUANTA_MON END FUNCTION CALLBACKS
 * **********************************
 */

/**
 * QUANTA_MON shared end function callback
 *
 * @author kannan
 */
zval * hp_mode_shared_endfn_cb(hp_entry_t *top,
                               char          *symbol  TSRMLS_DC) {
  zval    *counts;
  uint64   tsc_end;

  /* Get end tsc counter */
  tsc_end = cycle_timer();

  /* Get the stat array */
  if (!(counts = hp_hash_lookup(symbol TSRMLS_CC))) {
    return (zval *) 0;
  }

  /* Bump stats in the counts hashtable */
  hp_inc_count(counts, "ct", 1  TSRMLS_CC);

  hp_inc_count(counts, "wt", get_us_from_tsc(tsc_end - top->tsc_start,
        hp_globals.cpu_frequencies[hp_globals.cur_cpu_id]) TSRMLS_CC);
  return counts;
}

/**
 * QUANTA_MON_MODE_HIERARCHICAL's end function callback
 *
 * @author kannan
 */
void hp_mode_hier_endfn_cb(hp_entry_t **entries  TSRMLS_DC) {
  hp_entry_t   *top = (*entries);
  zval            *counts;
  struct rusage    ru_end;
  char             symbol[SCRATCH_BUF_LEN];
  long int         mu_end;
  long int         pmu_end;

  /* Get the stat array */
  hp_get_function_stack(top, 2, symbol, sizeof(symbol));
  if (!(counts = hp_mode_shared_endfn_cb(top,
                                         symbol  TSRMLS_CC))) {
    return;
  }

  if (hp_globals.quanta_mon_flags & QUANTA_MON_FLAGS_CPU) {
    /* Get CPU usage */
    getrusage(RUSAGE_SELF, &ru_end);

    /* Bump CPU stats in the counts hashtable */
    hp_inc_count(counts, "cpu", (get_us_interval(&(top->ru_start_hprof.ru_utime),
                                              &(ru_end.ru_utime)) +
                              get_us_interval(&(top->ru_start_hprof.ru_stime),
                                              &(ru_end.ru_stime)))
              TSRMLS_CC);
  }

  if (hp_globals.quanta_mon_flags & QUANTA_MON_FLAGS_MEMORY) {
    /* Get Memory usage */
    mu_end  = zend_memory_usage(0 TSRMLS_CC);
    pmu_end = zend_memory_peak_usage(0 TSRMLS_CC);

    /* Bump Memory stats in the counts hashtable */
    hp_inc_count(counts, "mu",  mu_end - top->mu_start_hprof    TSRMLS_CC);
    hp_inc_count(counts, "pmu", pmu_end - top->pmu_start_hprof  TSRMLS_CC);
  }
}

/**
 * QUANTA_MON_MODE_SAMPLED's end function callback
 *
 * @author veeve
 */
void hp_mode_sampled_endfn_cb(hp_entry_t **entries  TSRMLS_DC) {
  /* See if its time to take a sample */
  hp_sample_check(entries  TSRMLS_CC);
}

/**
 * QUANTA_MON_MODE_MAGENTO_PROFILING's end function callback
 *
 * @author ch
 */
void hp_mode_magento_profil_endfn_cb(hp_entry_t **entries  TSRMLS_DC) {
}


/**
 * QUANTA_MON_MODE_EVENTS_ONLY's end function callback
 *
 * @author ch
 */
void hp_mode_events_only_endfn_cb(hp_entry_t **entries  TSRMLS_DC) {
}


/**
 * ***************************
 * PHP EXECUTE/COMPILE PROXIES
 * ***************************
 */

/**
 * QuantaMon enable replaced the zend_execute function with this
 * new execute function. We can do whatever profiling we need to
 * before and after calling the actual zend_execute().
 *
 * @author hzhao, kannan
 */
#if PHP_VERSION_ID < 50500
ZEND_DLEXPORT void hp_execute (zend_op_array *ops TSRMLS_DC) {
#else
ZEND_DLEXPORT void hp_execute_ex (zend_execute_data *execute_data TSRMLS_DC) {
  zend_op_array *ops = execute_data->op_array;
#endif
  char          *func = NULL;
  char          *pathname;
  int hp_profile_flag = 1;

  func = hp_get_function_name(ops, &pathname TSRMLS_CC);
  if (!func) {
#if PHP_VERSION_ID < 50500
    _zend_execute(ops TSRMLS_CC);
#else
    _zend_execute_ex(execute_data TSRMLS_CC);
#endif
    return;
  }

  BEGIN_PROFILING(&hp_globals.entries, func, hp_profile_flag, pathname, execute_data);
#if PHP_VERSION_ID < 50500
  _zend_execute(ops TSRMLS_CC);
#else
  _zend_execute_ex(execute_data TSRMLS_CC);
#endif
  if (hp_globals.entries) {
    END_PROFILING(&hp_globals.entries, hp_profile_flag, execute_data);
  }
  efree(func);
}

#undef EX
#define EX(element) ((execute_data)->element)

/**
 * Very similar to hp_execute. Proxy for zend_execute_internal().
 * Applies to zend builtin functions.
 *
 * @author hzhao, kannan
 */

#if PHP_VERSION_ID < 50500
#define EX_T(offset) (*(temp_variable *)((char *) EX(Ts) + offset))

ZEND_DLEXPORT void hp_execute_internal(zend_execute_data *execute_data,
                                       int ret TSRMLS_DC) {
#else
#define EX_T(offset) (*EX_TMP_VAR(execute_data, offset))

ZEND_DLEXPORT void hp_execute_internal(zend_execute_data *execute_data,
                                       struct _zend_fcall_info *fci, int ret TSRMLS_DC) {
#endif
  zend_execute_data *current_data;
  char             *func = NULL;
  char             *pathname;
  int    hp_profile_flag = 1;

  current_data = EG(current_execute_data);
  func = hp_get_function_name(current_data->op_array, &pathname TSRMLS_CC);
  if (func) {
    BEGIN_PROFILING(&hp_globals.entries, func, hp_profile_flag, pathname, execute_data);
  }

  if (!_zend_execute_internal) {
    /* no old override to begin with. so invoke the builtin's implementation  */

#if ZEND_EXTENSION_API_NO >= 220121212
    /* PHP 5.5. This is just inlining a copy of execute_internal(). */

    if (fci != NULL) {
      ((zend_internal_function *) execute_data->function_state.function)->handler(
        fci->param_count,
        *fci->retval_ptr_ptr,
        fci->retval_ptr_ptr,
        fci->object_ptr,
        1 TSRMLS_CC);
    } else {
      zval **return_value_ptr = &EX_TMP_VAR(execute_data, execute_data->opline->result.var)->var.ptr;
      ((zend_internal_function *) execute_data->function_state.function)->handler(
        execute_data->opline->extended_value,
        *return_value_ptr,
        (execute_data->function_state.function->common.fn_flags & ZEND_ACC_RETURN_REFERENCE)
          ? return_value_ptr
          : NULL,
        execute_data->object,
        ret TSRMLS_CC);
    }
#elif ZEND_EXTENSION_API_NO >= 220100525
    zend_op *opline = EX(opline);
    temp_variable *retvar = &EX_T(opline->result.var);
    ((zend_internal_function *) EX(function_state).function)->handler(
                       opline->extended_value,
                       retvar->var.ptr,
                       (EX(function_state).function->common.fn_flags & ZEND_ACC_RETURN_REFERENCE) ?
                       &retvar->var.ptr:NULL,
                       EX(object), ret TSRMLS_CC);
#else
    zend_op *opline = EX(opline);
    ((zend_internal_function *) EX(function_state).function)->handler(
                       opline->extended_value,
                       EX_T(opline->result.u.var).var.ptr,
                       EX(function_state).function->common.return_reference ?
                       &EX_T(opline->result.u.var).var.ptr:NULL,
                       EX(object), ret TSRMLS_CC);
#endif
  } else {
    /* call the old override */
#if PHP_VERSION_ID < 50500
    _zend_execute_internal(execute_data, ret TSRMLS_CC);
#else
    _zend_execute_internal(execute_data, fci, ret TSRMLS_CC);
#endif
  }

  if (func) {
    if (hp_globals.entries) {
      END_PROFILING(&hp_globals.entries, hp_profile_flag, execute_data);
    }
    efree(func);
  }

}

/**
 * Proxy for zend_compile_file(). Used to profile PHP compilation time.
 *
 * @author kannan, hzhao
 */
ZEND_DLEXPORT zend_op_array* hp_compile_file(zend_file_handle *file_handle,
                                             int type TSRMLS_DC) {

  char           *filename;
  char           *func;
  int             len;
  zend_op_array  *ret;
  int             hp_profile_flag = 1;

  filename = strdup(hp_get_base_filename(file_handle->filename));
  len      = strlen("load") + strlen(filename) + 3;
  func      = (char *)emalloc(len);
  snprintf(func, len, "load::%s", filename);

  BEGIN_PROFILING(&hp_globals.entries, func, hp_profile_flag, filename, NULL);
  ret = _zend_compile_file(file_handle, type TSRMLS_CC);
  if (hp_globals.entries) {
    END_PROFILING(&hp_globals.entries, hp_profile_flag, NULL);
  }

  efree(func);
  return ret;
}

/**
 * Proxy for zend_compile_string(). Used to profile PHP eval compilation time.
 */
ZEND_DLEXPORT zend_op_array* hp_compile_string(zval *source_string, char *filename TSRMLS_DC) {

    char          *func;
    int            len;
    zend_op_array *ret;
    int            hp_profile_flag = 1;

    len  = strlen("eval") + strlen(filename) + 3;
    func = (char *)emalloc(len);
    snprintf(func, len, "eval::%s", filename);

    BEGIN_PROFILING(&hp_globals.entries, func, hp_profile_flag, filename, NULL);
    ret = _zend_compile_string(source_string, filename TSRMLS_CC);
    if (hp_globals.entries) {
        END_PROFILING(&hp_globals.entries, hp_profile_flag, NULL);
    }

    efree(func);
    return ret;
}

/**
 * **************************
 * MAIN QUANTA_MON CALLBACKS
 * **************************
 */

/**
 * This function gets called once when quanta_mon gets enabled.
 * It replaces all the functions like zend_execute, zend_execute_internal,
 * etc that needs to be instrumented with their corresponding proxies.
 */
static void hp_begin(long level, long quanta_mon_flags TSRMLS_DC) {
  if (!hp_globals.enabled) {
    int hp_profile_flag = 1;

    hp_globals.enabled      = 1;
    hp_globals.quanta_mon_flags = (uint32)quanta_mon_flags;

    /* Replace zend_compile with our proxy */
    _zend_compile_file = zend_compile_file;
    zend_compile_file  = hp_compile_file;

    /* Replace zend_compile_string with our proxy */
    _zend_compile_string = zend_compile_string;
    zend_compile_string = hp_compile_string;

    /* Replace zend_execute with our proxy */
#if PHP_VERSION_ID < 50500
    _zend_execute = zend_execute;
    zend_execute  = hp_execute;
#else
    _zend_execute_ex = zend_execute_ex;
    zend_execute_ex  = hp_execute_ex;
#endif

    /* Replace zend_execute_internal with our proxy */
    _zend_execute_internal = zend_execute_internal;
    if (!(hp_globals.quanta_mon_flags & QUANTA_MON_FLAGS_NO_BUILTINS)) {
      /* if NO_BUILTINS is not set (i.e. user wants to profile builtins),
       * then we intercept internal (builtin) function calls.
       */
      zend_execute_internal = hp_execute_internal;
    }

    /* Initialize with the dummy mode first Having these dummy callbacks saves
     * us from checking if any of the callbacks are NULL everywhere. */
    hp_globals.mode_cb.init_cb     = hp_mode_dummy_init_cb;
    hp_globals.mode_cb.exit_cb     = hp_mode_dummy_exit_cb;
    hp_globals.mode_cb.begin_fn_cb = hp_mode_dummy_beginfn_cb;
    hp_globals.mode_cb.end_fn_cb   = hp_mode_dummy_endfn_cb;

    /* Register the appropriate callback functions Override just a subset of
     * all the callbacks is OK. */
    switch(level) {
      case QUANTA_MON_MODE_HIERARCHICAL:
        hp_globals.mode_cb.begin_fn_cb = hp_mode_hier_beginfn_cb;
        hp_globals.mode_cb.end_fn_cb   = hp_mode_hier_endfn_cb;
        break;
      case QUANTA_MON_MODE_SAMPLED:
        hp_globals.mode_cb.init_cb     = hp_mode_sampled_init_cb;
        hp_globals.mode_cb.begin_fn_cb = hp_mode_sampled_beginfn_cb;
        hp_globals.mode_cb.end_fn_cb   = hp_mode_sampled_endfn_cb;
        break;
      case QUANTA_MON_MODE_MAGENTO_PROFILING:
        hp_globals.mode_cb.begin_fn_cb = hp_mode_magento_profil_beginfn_cb;
        hp_globals.mode_cb.end_fn_cb   = hp_mode_magento_profil_endfn_cb;
      case QUANTA_MON_MODE_EVENTS_ONLY:
        hp_globals.mode_cb.begin_fn_cb = hp_mode_events_only_beginfn_cb;
        hp_globals.mode_cb.end_fn_cb   = hp_mode_events_only_endfn_cb;
        break;
    }

    /* one time initializations */
    hp_init_profiler_state(level TSRMLS_CC);

    /* start profiling from fictitious main() */
    BEGIN_PROFILING(&hp_globals.entries, ROOT_SYMBOL, hp_profile_flag, "main", NULL);
  }
}

/**
 * Called at request shutdown time. Cleans the profiler's global state.
 */
static void hp_end(TSRMLS_D) {
  /* Bail if not ever enabled */
  if (!hp_globals.ever_enabled) {
    return;
  }

  /* Stop profiler if enabled */
  if (hp_globals.enabled) {
    hp_stop(TSRMLS_C);
  }

  /* Clean up state */
  hp_clean_profiler_state(TSRMLS_C);
}

#define BEGIN_OUTPUT_JSON_FORMAT \
"{\n"

#define PROFILING_OUTPUT_JSON_FORMAT \
"  \"profiling\": {\n\
    \"magento_loading\": %f,\n\
    \"before_layout_loading\": %f,\n\
    \"layout_loading\": %f,\n\
    \"between_layout_loading_and_rendering\": %f,\n\
    \"layout_rendering\": %f,\n\
    \"after_layout_rendering\": %f,\n\
    \"before_sending_response\": %f\n\
  },\n"

#define BLOCKS_BEGIN_OUTPUT_JSON_FORMAT \
"  \"blocks\": ["

#define BLOCK_OUTPUT_JSON_FORMAT \
"{\n\
    \"name\": \"%s\",\n\
    \"type\": \"%s\",\n\
    \"template\": \"%s\",\n\
    \"class\": \"%s\",\n\
    \"generate\": %f\n\
    \"renderize\": %f\n\
  }"

#define BLOCKS_END_OUTPUT_JSON_FORMAT \
"],\n"

#define END_OUTPUT_JSON_FORMAT \
"  \"flag\": %x\n\
}\n"

static float cpu_cycles_to_ms(float cpufreq, long long start, long long end) {
  // TODO check if it doesnt exceed long max value
  if (!start && !end)
     return -1.0;
  if (!start) 
    return -2.0;
  if (!end)
    return -3.0;
  return (end - start) / (cpufreq * 1000000) * 1000;
}

#define OUTBUF_QUANTA_SIZE 2048

// returns ouput file FD or -1
static int init_output(char *bufout) {
  struct timeval tv;
  return gettimeofday(&tv, NULL) == -1 ||
    snprintf(bufout, OUTBUF_QUANTA_SIZE, "/tmp/quanta-%d.%d-%d.txt",
    tv.tv_sec, tv.tv_usec, getpid()) < 4 ? -1 :
    open(bufout, O_WRONLY | O_APPEND | O_CREAT, 0644);
}

// writes content into the output file and set the debug info if necessary
static void output(int size, char *bufout, int fd_log_out) {
  if (size > 0 && write(fd_log_out, bufout, size) < 0)
    hp_globals.quanta_dbg |= 0x10000;
}

// begins the output
static void begin_output(int fd_log_out) {
  output(strlen(BEGIN_OUTPUT_JSON_FORMAT),
    BEGIN_OUTPUT_JSON_FORMAT, fd_log_out);
}

// ouputs the profiling data
static void profiler_output(char *bufout, int fd_log_out, float cpufreq) {
  long long *starts = hp_globals.monitored_function_tsc_start,
    *stops = hp_globals.monitored_function_tsc_stop;
  output(snprintf(bufout, OUTBUF_QUANTA_SIZE,
    PROFILING_OUTPUT_JSON_FORMAT,
    cpu_cycles_to_ms(cpufreq, starts[0], starts[1]),
    cpu_cycles_to_ms(cpufreq, starts[1], starts[2]),
    cpu_cycles_to_ms(cpufreq, starts[2], stops[2]),
    cpu_cycles_to_ms(cpufreq, stops[2], starts[3]),
    cpu_cycles_to_ms(cpufreq, starts[3], stops[3]),
    cpu_cycles_to_ms(cpufreq, stops[3], stops[4]),
    cpu_cycles_to_ms(cpufreq, stops[4], stops[5])),
  bufout, fd_log_out);
}

// outputs a single block data
static void block_output(char *bufout, int fd_log_out,
float cpufreq, generate_renderize_block_details *block) {
  output(snprintf(bufout, OUTBUF_QUANTA_SIZE, BLOCK_OUTPUT_JSON_FORMAT,
    block->name, block->type, block->template, block->class,
    cpu_cycles_to_ms(cpufreq, block->tsc_generate_start, block->tsc_generate_stop),
    cpu_cycles_to_ms(cpufreq, block->tsc_renderize_first_start, block->tsc_renderize_last_stop))
  , bufout, fd_log_out);
  if (block->name) efree(block->name);
  if (block->type) efree(block->type);
  if (block->template) efree(block->template);
}

// outputs a blocks data
static void blocks_output(char *bufout, int fd_log_out, float cpufreq) {
  output(strlen(BLOCKS_BEGIN_OUTPUT_JSON_FORMAT),
    BLOCKS_BEGIN_OUTPUT_JSON_FORMAT, fd_log_out);
  generate_renderize_block_details *current_block, *prev_block;
  current_block = hp_globals.monitored_function_generate_renderize_block_first_linked_list;
  while (current_block) {
    block_output(bufout, fd_log_out, cpufreq, prev_block = current_block);
    current_block = current_block->next_generate_renderize_block_detail;
    if (current_block) output(2, ", ", fd_log_out);
    efree(prev_block);
  }
  output(strlen(BLOCKS_END_OUTPUT_JSON_FORMAT),
    BLOCKS_END_OUTPUT_JSON_FORMAT, fd_log_out);
}

// ends output
static void end_output(char *bufout, int fd_log_out) {
  output(snprintf(bufout, OUTBUF_QUANTA_SIZE, END_OUTPUT_JSON_FORMAT,
    hp_globals.quanta_dbg), bufout, fd_log_out);
}

static void restore_original_zend_execute(void) {
  /* Remove proxies, restore the originals */
  #if PHP_VERSION_ID < 50500
    zend_execute          = _zend_execute;
  #else
    zend_execute_ex       = _zend_execute_ex;
  #endif
    zend_execute_internal = _zend_execute_internal;
    zend_compile_file     = _zend_compile_file;
    zend_compile_string   = _zend_compile_string;
}

/**
 * Called from quanta_mon_disable(). Removes all the proxies setup by
 * hp_begin() and restores the original values.
 */
static void hp_stop(TSRMLS_D) {
  char  *bufout;
  int   hp_profile_flag = 1, fd_log_out;
  float cpufreq = hp_globals.cpu_frequencies[hp_globals.cur_cpu_id];

  /* End any unfinished calls */
  while (hp_globals.entries)
    END_PROFILING(&hp_globals.entries, hp_profile_flag, NULL);
  restore_original_zend_execute();
  /* Resore cpu affinity. */
  restore_cpu_affinity(&hp_globals.prev_mask);
  /* Stop profiling */
  hp_globals.enabled = 0;
  if ((bufout = emalloc(OUTBUF_QUANTA_SIZE))) {
    if ((fd_log_out = init_output(bufout)) > 0) {
      begin_output(fd_log_out);
      profiler_output(bufout, fd_log_out, cpufreq);
      blocks_output(bufout, fd_log_out, cpufreq);
      end_output(bufout, fd_log_out);
      close(fd_log_out);
    }
    efree(bufout);
  }
}


/**
 * *****************************
 * QUANTA_MON ZVAL UTILITY FUNCTIONS
 * *****************************
 */

/** Look in the PHP assoc array to find a key and return the zval associated
 *  with it.
 *
 *  @author mpal
 **/
static zval *hp_zval_at_key(char  *key,
                            zval  *values) {
  zval *result = NULL;

  if (values->type == IS_ARRAY) {
    HashTable *ht;
    zval     **value;
    uint       len = strlen(key) + 1;

    ht = Z_ARRVAL_P(values);
    if (zend_hash_find(ht, key, len, (void**)&value) == SUCCESS) {
      result = *value;
    }
  } else {
    result = NULL;
  }

  return result;
}

/** Convert the PHP array of strings to an emalloced array of strings. Note,
 *  this method duplicates the string data in the PHP array.
 *
 *  @author mpal
 **/
static char **hp_strings_in_zval(zval  *values) {
  char   **result;
  size_t   count;
  size_t   ix = 0;

  if (!values) {
    return NULL;
  }

  if (values->type == IS_ARRAY) {
    HashTable *ht;

    ht    = Z_ARRVAL_P(values);
    count = zend_hash_num_elements(ht);

    if((result =
         (char**)emalloc(sizeof(char*) * (count + 1))) == NULL) {
      return result;
    }

    for (zend_hash_internal_pointer_reset(ht);
         zend_hash_has_more_elements(ht) == SUCCESS;
         zend_hash_move_forward(ht)) {
      char  *str;
      uint   len;
      ulong  idx;
      int    type;
      zval **data;

      type = zend_hash_get_current_key_ex(ht, &str, &len, &idx, 0, NULL);
      /* Get the names stored in a standard array */
      if(type == HASH_KEY_IS_LONG) {
        if ((zend_hash_get_current_data(ht, (void**)&data) == SUCCESS) &&
            Z_TYPE_PP(data) == IS_STRING &&
            strcmp(Z_STRVAL_PP(data), ROOT_SYMBOL)) { /* do not ignore "main" */
          result[ix] = estrdup(Z_STRVAL_PP(data));
          ix++;
        }
      }
    }
  } else if(values->type == IS_STRING) {
    if((result = (char**)emalloc(sizeof(char*) * 2)) == NULL) {
      return result;
    }
    result[0] = estrdup(Z_STRVAL_P(values));
    ix = 1;
  } else {
    result = NULL;
  }

  /* NULL terminate the array */
  if (result != NULL) {
    result[ix] = NULL;
  }

  return result;
}

/* Free this memory at the end of profiling */
static inline void hp_array_del(char **name_array) {
  if (name_array != NULL) {
    int i = 0;
    for(; name_array[i] != NULL && i < QUANTA_MON_MAX_IGNORED_FUNCTIONS; i++) {
      efree(name_array[i]);
    }
    efree(name_array);
  }
}
