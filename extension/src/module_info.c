#include "quanta_mon.h"

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
