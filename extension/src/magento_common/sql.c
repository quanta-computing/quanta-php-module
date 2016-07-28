#include "quanta_mon.h"

int magento_record_sql_query(profiled_application_t *app, zend_execute_data *data TSRMLS_DC) {
  if (qm_record_sql_query(app, data TSRMLS_CC))
    return -1;
  //TODO! blocks
  return 0;
}
