#include "quanta_mon.h"

zval *get_mage_model_zdata(HashTable *attrs, char *key, int type TSRMLS_DC) {
  HashTable *model_data;
  zval *data;
  zval *ret;

  if (!(data = zend_hash_find_compat(attrs, "\0*\0_data", 9))) {
    PRINTF_QUANTA("Cannot fetch model data\n");
    return NULL;
  }
  model_data = Z_ARRVAL_P(data);
  if (!(ret = zend_hash_find_compat(model_data, key, strlen(key)))) {
    PRINTF_QUANTA("Cannot fetch %s in model data\n", key);
    return NULL;
  }
  if (Z_TYPE_P(ret) != type) {
    PRINTF_QUANTA("%s is not a %d, it's a %d :(\n", key, type, Z_TYPE_P(ret));
    return NULL;
  }
  return ret;
}

char *get_mage_model_data(HashTable *attrs, char *key TSRMLS_DC) {
  zval *data;

  if (!(data = get_mage_model_zdata(attrs, key, IS_STRING TSRMLS_CC)))
    return NULL;
  return Z_STRVAL_P(data);
}

static char *get_magento2_composer_version(FILE *composer_file_handle TSRMLS_DC) {
  char *line = NULL;
  char *ptr = NULL;
  char *end;
  size_t line_size = 0;
  int ret;

  while ((ret = getline(&line, &line_size, composer_file_handle)) && ret != -1) {
    if ((ptr = strstr(line, "\"version\": \""))) {
      ptr += sizeof("\"version\": \"") - 1;
      if ((end = strchr(ptr, '\"'))) {
        *end = 0;
        ptr = estrdup(ptr);
      }
      goto end;
    }
  }

end:
  free(line);
  return ptr;
}

static void fetch_magento2_version(TSRMLS_D) {
  int ret;
  char composer_path[1024];
  zval *directory_root = NULL;
  FILE *composer_file_handle = NULL;
  zval edition;

  ZVAL_NULL(&edition);
  if (!(directory_root = safe_get_constant("BP", IS_STRING))) {
    PRINTF_QUANTA("Cannot get dir root\n");
    goto end;
  }
  ret = snprintf(composer_path, 1024, "%s/composer.json", Z_STRVAL_P(directory_root));
  if (ret == -1 || ret >= 1024
  || (composer_file_handle = fopen(composer_path, "r")) == NULL
  || !(hp_globals.magento_version = get_magento2_composer_version(composer_file_handle))) {
    PRINTF_QUANTA("Cannot get magento2 version\n");
    goto end;
  }
  ret = safe_get_class_constant("Magento\\Framework\\App\\ProductMetadata",
    "EDITION_NAME", &edition, IS_STRING TSRMLS_CC);
  if (!ret)
    hp_globals.magento_edition = estrdup(Z_STRVAL(edition));

end:
  zval_dtor(&edition);
  if (composer_file_handle)
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
