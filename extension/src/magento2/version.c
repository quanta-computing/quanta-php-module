#include "quanta_mon.h"

static char *get_magento2_composer_version(FILE *composer_file_handle) {
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

int magento2_fetch_version(profiled_application_t *app, profiled_function_t *function,
zend_execute_data *ex) {
  int ret;
  zval edition;
  char composer_path[1024];
  zval *directory_root = NULL;
  FILE *composer_file_handle = NULL;
  magento_context_t *context = (magento_context_t *)app->context;

  (void)function;
  ZVAL_NULL(&edition);
  if (!(directory_root = safe_get_constant("BP", IS_STRING))) {
    PRINTF_QUANTA("Cannot get dir root\n");
    goto end;
  }
  ret = snprintf(composer_path, 1024, "%s/composer.json", Z_STRVAL_P(directory_root));
  if (ret == -1 || ret >= 1024
  || (composer_file_handle = fopen(composer_path, "r")) == NULL
  || !(context->version = get_magento2_composer_version(composer_file_handle))) {
    PRINTF_QUANTA("Cannot get magento2 version\n");
    goto end;
  }
  /* Try enterprise version first and then fallback to standard location
  ** This is a hack since we guess the correct implementation of ProductMetadataInterface, a better
  ** but more expensive way would be to fetch the actual ProductMetadataInterface implementation
  ** from the object Manager
  */
  ret = safe_get_class_constant("Magento\\Enterprise\\Model\\ProductMetadata",
    "EDITION_NAME", &edition, IS_STRING);
  if (ret) {
    ret = safe_get_class_constant("Magento\\Framework\\App\\ProductMetadata",
      "EDITION_NAME", &edition, IS_STRING);
  }
  if (!ret)
    context->edition = estrdup(Z_STRVAL(edition));

end:
  zval_dtor(&edition);
  if (composer_file_handle)
    fclose(composer_file_handle);
  return 0;
}
