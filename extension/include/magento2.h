#ifndef QM_MAGENTO2_H_
#define QM_MAGENTO2_H_

int magento2_record_reindex_event(int profile_curr, zend_execute_data *execute_data TSRMLS_DC);
int magento2_record_cache_clean_event(int profile_curr, zend_execute_data *execute_data TSRMLS_DC);

int qm_before_tohtml(int profile_curr, zend_execute_data *execute_data TSRMLS_DC);
int qm_after_tohtml(zend_execute_data *execute_data TSRMLS_DC);


#endif /* end of include guard: QM_MAGENTO2_H_ */
