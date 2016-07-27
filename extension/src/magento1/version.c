#include "quanta_mon.h"

void magento1_fetch_version(struct timeval *clock, monikor_metric_list_t *metrics TSRMLS_DC) {
  zval version;
  zval edition;

  ZVAL_NULL(&version);
  ZVAL_NULL(&edition);
  if (safe_call_function("Mage::getVersion", &version, IS_STRING, 0, NULL TSRMLS_CC)
  || safe_call_function("Mage::getEdition", &edition, IS_STRING, 0, NULL)) {
    PRINTF_QUANTA("Could not get magento version\n");
    goto end;
  }
  hp_globals.magento_version = estrdup(Z_STRVAL(version));
  hp_globals.magento_edition = estrdup(Z_STRVAL(edition));

end:
  zval_dtor(&version);
  zval_dtor(&edition);
}
