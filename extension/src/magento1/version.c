#include "quanta_mon.h"

int magento1_fetch_version(profiled_application_t *app, profiled_function_t *function,
zend_execute_data *data) {
  zval version;
  zval edition;
  magento_context_t *context = (magento_context_t *)app->context;

  (void)function;
  (void)data;
  ZVAL_NULL(&version);
  ZVAL_NULL(&edition);
  if (safe_call_function("Mage::getVersion", &version, IS_STRING, 0, NULL)
  || safe_call_function("Mage::getEdition", &edition, IS_STRING, 0, NULL)) {
    PRINTF_QUANTA("Could not get magento version\n");
    goto end;
  }
  context->version = estrdup(Z_STRVAL(version));
  context->edition = estrdup(Z_STRVAL(edition));

end:
  zval_dtor(&version);
  zval_dtor(&edition);
  return 0;
}
