#include "quanta_mon.h"

char *get_mage_model_data(HashTable *attrs, char *key TSRMLS_DC) {
  HashTable *model_data;
  zval **data;
  zval **ret;

  if (zend_hash_find(attrs, "\0*\0_data", 9, (void **)&data) == FAILURE) {
    PRINTF_QUANTA("Cannot fetch model data\n");
    return NULL;
  }
  model_data = Z_ARRVAL_PP(data);
  if (zend_hash_find(model_data, key, strlen(key) + 1, (void **)&ret) == FAILURE) {
    PRINTF_QUANTA("Cannot fetch %s in model data\n", key);
    return NULL;
  }
  if (Z_TYPE_PP(ret) != IS_STRING) {
    PRINTF_QUANTA("%s is not a string :(\n", key);
    return NULL;
  }
  return Z_STRVAL_PP(ret);
}

static void fetch_magento2_version(TSRMLS_D) {
  zval *objectManager;
  zval *productMetaData;
  zval version;
  zval edition;
  zval *params[1];
  int ret;

  MAKE_STD_ZVAL(objectManager);
  MAKE_STD_ZVAL(productMetaData);
  MAKE_STD_ZVAL(params[0]);
  ZVAL_NULL(&version);
  ZVAL_NULL(&edition);
  ret = safe_call_function("Magento\\Framework\\App\\ObjectManager::getInstance",
    objectManager, IS_OBJECT, 0, NULL TSRMLS_CC);
  if (ret) {
    PRINTF_QUANTA("Cannot get Magento\\Framework\\App\\ObjectManager\n");
    goto end;
  }
  ZVAL_STRING(params[0], "Magento\\Framework\\App\\ProductMetadataInterface", 1);
  if (safe_call_method(objectManager, "get", productMetaData, IS_OBJECT, 1, params TSRMLS_CC)) {
    PRINTF_QUANTA("Cannot get Magento\\Framework\\App\\ProductMetaData\n");
    goto end;
  }
  if (safe_call_method(productMetaData, "getVersion", &version, IS_STRING, 0, NULL TSRMLS_CC)
  || safe_call_method(productMetaData, "getEdition", &edition, IS_STRING, 0, NULL TSRMLS_CC)) {
    PRINTF_QUANTA("Cannot get magento2 version\n");
    goto end;
  }
  hp_globals.magento_version = estrdup(Z_STRVAL(version));
  hp_globals.magento_edition = estrdup(Z_STRVAL(edition));

end:
  FREE_ZVAL(objectManager);
  FREE_ZVAL(productMetaData);
  FREE_ZVAL(params[0]);
  zval_dtor(&version);
  zval_dtor(&edition);
}

//TODO! fix me
static void fetch_magento1_version(struct timeval *clock, monikor_metric_list_t *metrics TSRMLS_DC) {
  zval version;
  zval edition;

  ZVAL_NULL(&version);
  ZVAL_NULL(&edition);
  if (safe_call_function("Mage::getVersion", &version, IS_STRING, 0, NULL TSRMLS_CC)
  || safe_call_function("Mage::getEdition", &edition, IS_STRING, 0, NULL)) {
    PRINTF_QUANTA("Could not get magento version\n");
    goto end;
  }
  push_magento_version_metric(clock, metrics, Z_STRVAL(version), Z_STRVAL(edition));

end:
  zval_dtor(&version);
  zval_dtor(&edition);
}

void fetch_magento_version(TSRMLS_D) {
  if (!hp_globals.magento_version) {
    fetch_magento2_version(TSRMLS_C);
    PRINTF_QUANTA("Magento version '%s %s'\n", hp_globals.magento_version, hp_globals.magento_edition);
  }
}
