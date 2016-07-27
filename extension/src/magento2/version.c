#include "quanta_mon.h"

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

void magento2_fetch_version(TSRMLS_D) {
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
