#ifndef QM_MODULE_H_
#define QM_MODULE_H_

extern zend_module_entry quanta_mon_module_entry;
#define phpext_quanta_mon_ptr &quanta_mon_module_entry

PHP_MINIT_FUNCTION(quanta_mon);
PHP_MSHUTDOWN_FUNCTION(quanta_mon);
PHP_RINIT_FUNCTION(quanta_mon);
PHP_RSHUTDOWN_FUNCTION(quanta_mon);
PHP_MINFO_FUNCTION(quanta_mon);

#endif /* end of include guard: QM_MODULE_H_ */
