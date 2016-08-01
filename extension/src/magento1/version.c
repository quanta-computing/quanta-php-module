#include "quanta_mon.h"

void magento1_fetch_version(profiled_application_t *app, profiled_function_t *function,
zend_execute_data *data TSRMLS_DC) {
  zval version;
  zval edition;

  ZVAL_NULL(&version);
  ZVAL_NULL(&edition);
  if (safe_call_function("Mage::getVersion", &version, IS_STRING, 0, NULL TSRMLS_CC)
  || safe_call_function("Mage::getEdition", &edition, IS_STRING, 0, NULL)) {
    PRINTF_QUANTA("Could not get magento version\n");
    goto end;
  }
  // TODO! context
  // hp_globals.magento_version = estrdup(Z_STRVAL(version));
  // hp_globals.magento_edition = estrdup(Z_STRVAL(edition));

end:
  zval_dtor(&version);
  zval_dtor(&edition);
}
