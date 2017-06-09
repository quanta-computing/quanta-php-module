#ifndef QM_ORO_H_
#define QM_ORO_H_

typedef struct oro_block_t {
  uint64_t  tsc_renderize_first_start;
  uint64_t  tsc_renderize_last_stop;
  uint64_t  renderize_children_cycles;
  uint64_t  sql_cpu_cycles;
  uint64_t  sql_queries_count;
  char    *name;
  char    *template;
  struct oro_block_t *next;
} oro_block_t;

typedef struct oro_block_stack {
  struct oro_block_stack *prev;
  oro_block_t *block;
} oro_block_stack_t;


typedef struct {
  char *version;
  char *edition;

  struct {
    oro_block_t *first;
    oro_block_t *last;
  } blocks;
  oro_block_stack_t *block_stack;

} oro_context_t;

// Context
void *oro_init_context(profiled_application_t *app TSRMLS_DC);
void oro_cleanup_context(profiled_application_t *app TSRMLS_DC);

// Callbacks
int oro_before_process_block(profiled_application_t *app, profiled_function_t *function,
  zend_execute_data *ex TSRMLS_DC);
int oro_before_render_block(profiled_application_t *app, profiled_function_t *function,
  zend_execute_data *ex TSRMLS_DC);
int oro_after_render_block(profiled_application_t *app, profiled_function_t *function,
  zend_execute_data *ex TSRMLS_DC);
int oro_record_sql_query(profiled_application_t *app, profiled_function_t *function,
  zend_execute_data *data TSRMLS_DC);
void oro_send_metrics(profiled_application_t *app, monikor_metric_list_t *metrics,
  float cpufreq, struct timeval *clock TSRMLS_DC);

// Block stack
oro_block_t *oro_block_stack_pop(oro_context_t *context);
oro_block_t *oro_block_stack_top(oro_context_t *context);
void oro_block_stack_push(oro_context_t *context, oro_block_t *block);


#endif /* end of include guard: QM_ORO_H_ */
