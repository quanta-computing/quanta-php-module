#include "quanta_mon.h"

/*
** Configuration
*/
PHP_INI_BEGIN()
PHP_INI_ENTRY("quanta_mon.path_quanta_agent_socket", "", PHP_INI_SYSTEM, NULL)
PHP_INI_ENTRY("quanta_mon.admin_url", "", PHP_INI_SYSTEM, NULL)
PHP_INI_END()

PHP_MINIT_FUNCTION(quanta_mon) {
  int i;

  REGISTER_INI_ENTRIES();

  hp_globals.path_quanta_agent_socket = INI_STR("quanta_mon.path_quanta_agent_socket");
  hp_globals.admin_url = INI_STR("quanta_mon.admin_url");
  if (!hp_globals.admin_url || !*hp_globals.admin_url)
    hp_globals.admin_url = QUANTA_MON_DEFAULT_ADMIN_URL;
  if ((!hp_globals.path_quanta_agent_socket) || (strlen(hp_globals.path_quanta_agent_socket) < 4)) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING,
      "quanta_mon.path_quanta_agent_socket configuration missing or invalid. Module disabled.");
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
  hp_globals.ever_enabled = 0;
  hp_globals.enabled = 0;

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
