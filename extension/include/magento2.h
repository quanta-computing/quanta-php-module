#ifndef QM_MAGENTO2_H_
#define QM_MAGENTO2_H_

int magento2_block_before_render(profiled_application_t *app, profiled_function_t *function,
  zend_execute_data *execute_data TSRMLS_DC);

int magento2_record_reindex_event(profiled_application_t *app, profiled_function_t *function,
  zend_execute_data *execute_data TSRMLS_DC);
int magento2_record_cache_clean_event(profiled_application_t *app, profiled_function_t *function,
  zend_execute_data *execute_data TSRMLS_DC);

int magento2_fetch_version(profiled_application_t *app, profiled_function_t *function,
  zend_execute_data *ex TSRMLS_DC);

#endif /* end of include guard: QM_MAGENTO2_H_ */
