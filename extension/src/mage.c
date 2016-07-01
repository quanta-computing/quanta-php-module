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

static char *get_magento2_composer_version(FILE *composer_file_handle TSRMLS_DC) {
  char *line = NULL;
  char *ptr = NULL;
  size_t line_size = 0;
  int ret;

  while ((ret = getline(&line, &line_size, composer_file_handle)) && ret != -1) {
    if ((ptr = strstr(line, "\"version\": \""))) {
      ptr = estrndup(ptr + sizeof("\"version\": \""),
        line_size - (ptr - line) - sizeof("\"version\": \"") - sizeof("\",\n"));
      goto end;
    }
  }

end:
  free(line);
  return ptr;
}

static void fetch_magento2_version(TSRMLS_D) {
  zval *objectManager;
  zval *productMetaData;
  zval version;
  zval edition;
  zval directory_root;
  zval *params[1];
  char composer_path[1024];
  FILE *composer_file_handle;
  int ret;

  MAKE_STD_ZVAL(objectManager);
  MAKE_STD_ZVAL(productMetaData);
  MAKE_STD_ZVAL(params[0]);
  ZVAL_NULL(&version);
  ZVAL_NULL(&edition);
  ZVAL_NULL(&directory_root);
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
  // if (safe_call_method(productMetaData, "getVersion", &version, IS_STRING, 0, NULL TSRMLS_CC)
  // || safe_call_method(productMetaData, "getEdition", &edition, IS_STRING, 0, NULL TSRMLS_CC)) {
  //   PRINTF_QUANTA("Cannot get magento2 version\n");
  //   goto end;
  // }
  if (safe_call_method(productMetaData, "getEdition", &edition, IS_STRING, 0, NULL TSRMLS_CC)) {
    PRINTF_QUANTA("Cannot get magento2 version\n");
    goto end;
  }
  if (zend_get_constant("BP", 2, &directory_root)) {
    PRINTF_QUANTA("Cannot get magento2 version\n");
    goto end;
  }

  if ((ret = snprintf(composer_path, 1024,
  "%s/composer.json", Z_STRVAL(directory_root))) == -1 || ret == 1024) {
    PRINTF_QUANTA("Cannot get magento2 version\n");
    goto end;
  }

  if ((composer_file_handle = fopen(composer_path, "r")) == NULL) {
    PRINTF_QUANTA("Cannot get magento2 version\n");
    goto end;
  }

  if (!(hp_globals.magento_version = get_magento2_composer_version(composer_file_handle))) {
    PRINTF_QUANTA("Cannot get magento2 version\n");
    goto end;
  }

  hp_globals.magento_edition = estrdup(Z_STRVAL(edition));

end:
  FREE_ZVAL(objectManager);
  FREE_ZVAL(productMetaData);
  FREE_ZVAL(params[0]);
  zval_dtor(&version);
  zval_dtor(&edition);
  zval_dtor(&directory_root);
  fclose(composer_file_handle);
}

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
  hp_globals.magento_version = estrdup(Z_STRVAL(version));
  hp_globals.magento_edition = estrdup(Z_STRVAL(edition));

end:
  zval_dtor(&version);
  zval_dtor(&edition);
}

void fetch_magento_version(TSRMLS_D) {
  if (!hp_globals.magento_version) {
    fetch_magento2_version(TSRMLS_C);
  }
}
