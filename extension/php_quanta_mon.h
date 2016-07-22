/*
 *  Copyright (c) 2009 Facebook
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#ifndef PHP_QUANTA_MON_H
#define PHP_QUANTA_MON_H

extern zend_module_entry quanta_mon_module_entry;
#define phpext_quanta_mon_ptr &quanta_mon_module_entry

#ifdef PHP_WIN32
#define PHP_QUANTA_MON_API __declspec(dllexport)
#else
#define PHP_QUANTA_MON_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(quanta_mon);
PHP_MSHUTDOWN_FUNCTION(quanta_mon);
PHP_RINIT_FUNCTION(quanta_mon);
PHP_RSHUTDOWN_FUNCTION(quanta_mon);
PHP_MINFO_FUNCTION(quanta_mon);
//
// PHP_FUNCTION(quanta_mon_enable);
// PHP_FUNCTION(quanta_mon_disable);

#endif /* PHP_QUANTA_MON_H */
