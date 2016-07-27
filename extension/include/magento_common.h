#ifndef QM_MAGENTO_COMMON_H_
#define QM_MAGENTO_COMMON_H_

/* Monitored functions positions */
#define POS_ENTRY_APP_RUN 0
#define POS_ENTRY_TOHTML     15
#define POS_ENTRY_PDO_EXECUTE      16
#define POS_ENTRY_EVENTS_ONLY      17 /* Anything below won't be processed unless the special cookie is set */
#define POS_ENTRY_EV_CACHE_FLUSH   17
#define POS_ENTRY_EV_CLEAN_TYPE    18
#define POS_ENTRY_EV_MAGE_CLEAN    19
#define POS_ENTRY_EV_BEFORE_SAVE   20
#define POS_ENTRY_PHP_TOTAL        21


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

#define MAGENTO_EVENT_CACHE_CLEAR 1
#define MAGENTO_EVENT_REINDEX 2
typedef struct magento_event {
  struct magento_event *prev;
  uint8_t class;
  char *type;
  char *subtype;
} magento_event_t;

// Block stack
magento_block_t *block_stack_pop(void);
void block_stack_push(magento_block_t *block);
magento_block_t *block_stack_top(void);

char *get_mage_model_data(HashTable *attrs, char *key TSRMLS_DC);
zval *get_mage_model_zdata(HashTable *attrs, char *key, int type TSRMLS_DC);

// void fetch_magento_version(TSRMLS_D);

#endif /* end of include guard: QM_MAGENTO_COMMON_H_ */
