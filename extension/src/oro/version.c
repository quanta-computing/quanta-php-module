#include "quanta_mon.h"

static char *get_oro_version_from_composer_installed(FILE *composer_file) {
  char *ptr = NULL;
  char *line = NULL;
  char *end;
  int found = 0;
  int ret;
  size_t line_size;

  while ((ret = getline(&line, &line_size, composer_file)) && ret != -1) {
    if (found && (ptr = strstr(line, "\"version\": \""))) {
      ptr += sizeof("\"version\": \"") - 1;
      if ((end = strchr(ptr, '\"'))) {
        *end = 0;
        ptr = estrdup(ptr);
      } else {
        ptr = NULL;
      }
      goto done;
    } else if (strstr(line, "\"name\": \"oro/commerce\"")) {
      found = 1;
    }
  }

done:
  free(line);
  return ptr;
}

static const char *installed_file = "vendor/composer/installed.json";

static char *get_oro_vendor_file(zend_execute_data *execute_data) {
  zval *this;
  zend_class_entry *ce;
  char *this_filename;
  char *filename;
  char *end;

  if (!(this = get_prev_this(execute_data))
  || !(ce = Z_OBJCE_P(this))
  || ce->type != ZEND_USER_CLASS
  || !ce->info.user.filename) {
    PRINTF_QUANTA("ORO DIRECTORY NOT FOUND\n");
    return NULL;
  }
  #if PHP_MAJOR_VERSION < 7
    this_filename = ce->info.user.filename;
  #else
    this_filename = ZSTR_VAL(ce->info.user.filename);
  #endif
  if (!(end = rindex(this_filename, '/')))
    return NULL;
  filename = ecalloc((end - this_filename) + 1 - strlen("app")+ strlen(installed_file), 1);
  if (!filename)
    return NULL;
  strncpy(filename, this_filename, end - this_filename - strlen("app"));
  strcat(filename, installed_file);
  return filename;
}

int oro_fetch_version(profiled_application_t *app, profiled_function_t *function,
zend_execute_data *execute_data) {
  oro_context_t *context = (oro_context_t *)app->context;
  char *filepath = NULL;
  FILE *composer_file_handle = NULL;

  (void)function;
  filepath = get_oro_vendor_file(execute_data);
  if (!filepath)
    return 0;
  if (!(composer_file_handle = fopen(filepath, "r"))) {
    efree(filepath);
    return 0;
  }
  context->version = get_oro_version_from_composer_installed(composer_file_handle);
  efree(filepath);
  fclose(composer_file_handle);
  return 0;
}
