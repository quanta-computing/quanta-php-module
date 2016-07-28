#ifndef QM_MAGENTO_COMMON_H_
#define QM_MAGENTO_COMMON_H_

typedef struct magento_block_t {
  uint64_t  tsc_renderize_first_start; /* Might be re-entrant, record the first call */
  uint64_t  tsc_renderize_last_stop;   /* Always record the latest timestamp */
  uint64_t  renderize_children_cycles;
  uint64_t  sql_cpu_cycles;
  uint64_t  sql_queries_count;
  char    *name;
  char    *class;
  char    *class_file;
  char    *template;
  struct magento_block_t *next;
} magento_block_t;

typedef struct block_stack {
  struct block_stack *prev;
  magento_block_t *block;
} block_stack_t;

typedef struct {
  char *version;
  char *edition;

  struct {
    magento_block_t *first;
    magento_block_t *last;
  } blocks;
  block_stack_t *block_stack;

} magento_context_t;

// Block stack
magento_block_t *block_stack_pop(magento_context_t *context);
magento_block_t *block_stack_top(magento_context_t *context);
void block_stack_push(magento_context_t *context, magento_block_t *block);

char *get_mage_model_data(HashTable *attrs, char *key TSRMLS_DC);
zval *get_mage_model_zdata(HashTable *attrs, char *key, int type TSRMLS_DC);

int magento_record_cache_system_flush_event(profiled_application_t *app, zend_execute_data *execute_data TSRMLS_DC);
int magento_record_cache_flush_event(profiled_application_t *app, zend_execute_data *execute_data TSRMLS_DC);

int magento_record_sql_query(profiled_application_t *app, zend_execute_data *data TSRMLS_DC);

void *magento_init_context(profiled_application_t *app TSRMLS_DC);
void magento_cleanup_context(profiled_application_t *app TSRMLS_DC);
void magento_send_metrics(profiled_application_t *app, monikor_metric_list_t *metrics,
float cpufreq, struct timeval *clock TSRMLS_DC);

#endif /* end of include guard: QM_MAGENTO_COMMON_H_ */
