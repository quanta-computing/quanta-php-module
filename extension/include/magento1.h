#ifndef QM_MAGENTO1_H_
#define QM_MAGENTO1_H_

int magento1_record_reindex_event(profiled_application_t *app, profiled_function_t *function,
  zend_execute_data *execute_data TSRMLS_DC);
int magento1_record_cache_clean_event(profiled_application_t *app, profiled_function_t *function,
  zend_execute_data *execute_data TSRMLS_DC);

int magento1_block_before_render(profiled_application_t *app, profiled_function_t *function,
  zend_execute_data *execute_data TSRMLS_DC);

#endif /* end of include guard: QM_MAGENTO1_H_ */
