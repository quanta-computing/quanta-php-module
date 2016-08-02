#include "quanta_mon.h"

static profiled_function_t magento2_profiled_functions[] = {
{"Magento\\Framework\\App\\Bootstrap::run", 0, {0, 0}, magento2_fetch_version, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Magento\\Framework\\App\\Bootstrap::create", 1, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Magento\\Framework\\App\\Bootstrap::createApplication", 2, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Magento\\Framework\\App\\Request\\Http::getFrontName", 3, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Magento\\Framework\\App\\FrontController\\Interceptor::dispatch", 4, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Magento\\Framework\\App\\Action\\Action::dispatch", 5, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Magento\\Framework\\View\\Result\\Page\\Interceptor::renderResult", 6, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Magento\\Framework\\App\\Response\\Http\\Interceptor::sendResponse", 7, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Magento\\Framework\\View\\Layout::_renderBlock", 8, {1, 0}, magento2_block_before_render, magento_block_after_render, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushAll::execute", 9, {1, 0}, magento_record_cache_flush_event, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushSystem::execute", 10, {1, 0}, magento_record_cache_system_flush_event, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Magento\\Framework\\App\\Cache\\TypeList::cleanType", 11, {1, 0}, magento2_record_cache_clean_event, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Magento\\Indexer\\Model\\Indexer::reindexAll", 12, {1, 0}, NULL, magento2_record_reindex_event, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"PDOStatement::execute", 13, {1, 0}, NULL, magento_record_sql_query, {0, 0, 0, 0}, {0, 0, 0, 0}},
{NULL, 0, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}}
};

static const profiler_timer_t magento2_profiler_timers[] = {
{
  "create_bootstrap",
  {&magento2_profiled_functions[1], PROF_FIRST_START},
  {&magento2_profiled_functions[1], PROF_LAST_STOP},
  {0}
},
{
  "create_application",
  {&magento2_profiled_functions[2], PROF_FIRST_START},
  {&magento2_profiled_functions[2], PROF_LAST_STOP},
  {0}
},
{
  "after_create_application",
  {&magento2_profiled_functions[2], PROF_LAST_STOP},
  {&magento2_profiled_functions[3], PROF_FIRST_START},
  {0}
},
{
  "parse_request",
  {&magento2_profiled_functions[3], PROF_FIRST_START},
  {&magento2_profiled_functions[3], PROF_LAST_STOP},
  {0}
},
{
  "configure_area",
  {&magento2_profiled_functions[3], PROF_LAST_STOP},
  {&magento2_profiled_functions[4], PROF_FIRST_START},
  {0}
},
{
  "loading",
  {&magento2_profiled_functions[1], PROF_FIRST_START},
  {&magento2_profiled_functions[4], PROF_FIRST_START},
  {0}
},
{
  "dispatch",
  {&magento2_profiled_functions[4], PROF_FIRST_START},
  {&magento2_profiled_functions[4], PROF_LAST_STOP},
  {0}
},
{
  "routing",
  {&magento2_profiled_functions[4], PROF_FIRST_START},
  {&magento2_profiled_functions[5], PROF_FIRST_START},
  {0}
},
{
  "controller",
  {&magento2_profiled_functions[5], PROF_FIRST_START},
  {&magento2_profiled_functions[5], PROF_LAST_STOP},
  {0}
},
{
  "between_controller_and_layout_rendering",
  {&magento2_profiled_functions[5], PROF_LAST_STOP},
  {&magento2_profiled_functions[6], PROF_FIRST_START},
  {0}
},
{
  "layout_rendering",
  {&magento2_profiled_functions[6], PROF_FIRST_START},
  {&magento2_profiled_functions[6], PROF_LAST_STOP},
  {0}
},
{
  "before_sending_response",
  {&magento2_profiled_functions[6], PROF_LAST_STOP},
  {&magento2_profiled_functions[7], PROF_FIRST_START},
  {0}
},
{
  "sending_response",
  {&magento2_profiled_functions[7], PROF_FIRST_START},
  {&magento2_profiled_functions[7], PROF_LAST_STOP},
  {0}
},
{
  "total",
  {&magento2_profiled_functions[1], PROF_FIRST_START},
  {&magento2_profiled_functions[0], PROF_LAST_STOP},
  {1}
},
{NULL, {0, 0}, {0, 0}, {0}}
};

profiled_function_t *magento2_match_function(const char* function_name, zend_execute_data* data TSRMLS_DC) {
 ++hp_globals.internal_match_counters.total;
 if (function_name[0] == 's') {
  if (function_name[1] == 'e') {
   if (function_name[2] == 'n') {
    if (function_name[3] == 'd') {
     if (function_name[4] == 'R') {
      if (function_name[5] == 'e') {
       if (function_name[6] == 's') {
        if (function_name[7] == 'p') {
         if (function_name[8] == 'o') {
          if (function_name[9] == 'n') {
           if (function_name[10] == 's') {
            if (function_name[11] == 'e') {
             if (function_name[12] == '\0') {
              ++hp_globals.internal_match_counters.function;
              const char *class_name = hp_get_class_name(data TSRMLS_CC);
              if (!class_name) return NULL;
              if (!strcmp(class_name, "Magento\\Framework\\App\\Response\\Http\\Interceptor"))
               return &magento2_profiled_functions[7];
              ++hp_globals.internal_match_counters.class_unmatched;
              return NULL;
             }
             return NULL;
            }
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  return NULL;
 }
 if (function_name[0] == 'g') {
  if (function_name[1] == 'e') {
   if (function_name[2] == 't') {
    if (function_name[3] == 'F') {
     if (function_name[4] == 'r') {
      if (function_name[5] == 'o') {
       if (function_name[6] == 'n') {
        if (function_name[7] == 't') {
         if (function_name[8] == 'N') {
          if (function_name[9] == 'a') {
           if (function_name[10] == 'm') {
            if (function_name[11] == 'e') {
             if (function_name[12] == '\0') {
              ++hp_globals.internal_match_counters.function;
              const char *class_name = hp_get_class_name(data TSRMLS_CC);
              if (!class_name) return NULL;
              if (!strcmp(class_name, "Magento\\Framework\\App\\Request\\Http"))
               return &magento2_profiled_functions[3];
              ++hp_globals.internal_match_counters.class_unmatched;
              return NULL;
             }
             return NULL;
            }
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  return NULL;
 }
 if (function_name[0] == '_') {
  if (function_name[1] == 'r') {
   if (function_name[2] == 'e') {
    if (function_name[3] == 'n') {
     if (function_name[4] == 'd') {
      if (function_name[5] == 'e') {
       if (function_name[6] == 'r') {
        if (function_name[7] == 'B') {
         if (function_name[8] == 'l') {
          if (function_name[9] == 'o') {
           if (function_name[10] == 'c') {
            if (function_name[11] == 'k') {
             if (function_name[12] == '\0') {
              ++hp_globals.internal_match_counters.function;
              const char *class_name = hp_get_class_name(data TSRMLS_CC);
              if (!class_name) return NULL;
              if (!strcmp(class_name, "Magento\\Framework\\View\\Layout"))
               return &magento2_profiled_functions[8];
              ++hp_globals.internal_match_counters.class_unmatched;
              return NULL;
             }
             return NULL;
            }
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  return NULL;
 }
 if (function_name[0] == 'c') {
  if (function_name[1] == 'r') {
   if (function_name[2] == 'e') {
    if (function_name[3] == 'a') {
     if (function_name[4] == 't') {
      if (function_name[5] == 'e') {
       if (function_name[6] == '\0') {
        ++hp_globals.internal_match_counters.function;
        const char *class_name = hp_get_class_name(data TSRMLS_CC);
        if (!class_name) return NULL;
        if (!strcmp(class_name, "Magento\\Framework\\App\\Bootstrap"))
         return &magento2_profiled_functions[1];
        ++hp_globals.internal_match_counters.class_unmatched;
        return NULL;
       }
       if (function_name[6] == 'A') {
        if (function_name[7] == 'p') {
         if (function_name[8] == 'p') {
          if (function_name[9] == 'l') {
           if (function_name[10] == 'i') {
            if (function_name[11] == 'c') {
             if (function_name[12] == 'a') {
              if (function_name[13] == 't') {
               if (function_name[14] == 'i') {
                if (function_name[15] == 'o') {
                 if (function_name[16] == 'n') {
                  if (function_name[17] == '\0') {
                   ++hp_globals.internal_match_counters.function;
                   const char *class_name = hp_get_class_name(data TSRMLS_CC);
                   if (!class_name) return NULL;
                   if (!strcmp(class_name, "Magento\\Framework\\App\\Bootstrap"))
                    return &magento2_profiled_functions[2];
                   ++hp_globals.internal_match_counters.class_unmatched;
                   return NULL;
                  }
                  return NULL;
                 }
                 return NULL;
                }
                return NULL;
               }
               return NULL;
              }
              return NULL;
             }
             return NULL;
            }
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  if (function_name[1] == 'l') {
   if (function_name[2] == 'e') {
    if (function_name[3] == 'a') {
     if (function_name[4] == 'n') {
      if (function_name[5] == 'T') {
       if (function_name[6] == 'y') {
        if (function_name[7] == 'p') {
         if (function_name[8] == 'e') {
          if (function_name[9] == '\0') {
           ++hp_globals.internal_match_counters.function;
           const char *class_name = hp_get_class_name(data TSRMLS_CC);
           if (!class_name) return NULL;
           if (!strcmp(class_name, "Magento\\Framework\\App\\Cache\\TypeList"))
            return &magento2_profiled_functions[11];
           ++hp_globals.internal_match_counters.class_unmatched;
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  return NULL;
 }
 if (function_name[0] == 'e') {
  if (function_name[1] == 'x') {
   if (function_name[2] == 'e') {
    if (function_name[3] == 'c') {
     if (function_name[4] == 'u') {
      if (function_name[5] == 't') {
       if (function_name[6] == 'e') {
        if (function_name[7] == '\0') {
         ++hp_globals.internal_match_counters.function;
         const char *class_name = hp_get_class_name(data TSRMLS_CC);
         if (!class_name) return NULL;
         if (!strcmp(class_name, "PDOStatement"))
          return &magento2_profiled_functions[13];
         if (!strcmp(class_name, "Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushAll"))
          return &magento2_profiled_functions[9];
         if (!strcmp(class_name, "Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushSystem"))
          return &magento2_profiled_functions[10];
         ++hp_globals.internal_match_counters.class_unmatched;
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  return NULL;
 }
 if (function_name[0] == 'd') {
  if (function_name[1] == 'i') {
   if (function_name[2] == 's') {
    if (function_name[3] == 'p') {
     if (function_name[4] == 'a') {
      if (function_name[5] == 't') {
       if (function_name[6] == 'c') {
        if (function_name[7] == 'h') {
         if (function_name[8] == '\0') {
          ++hp_globals.internal_match_counters.function;
          const char *class_name = hp_get_class_name(data TSRMLS_CC);
          if (!class_name) return NULL;
          if (!strcmp(class_name, "Magento\\Framework\\App\\Action\\Action"))
           return &magento2_profiled_functions[5];
          if (!strcmp(class_name, "Magento\\Framework\\App\\FrontController\\Interceptor"))
           return &magento2_profiled_functions[4];
          ++hp_globals.internal_match_counters.class_unmatched;
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  return NULL;
 }
 if (function_name[0] == 'r') {
  if (function_name[1] == 'e') {
   if (function_name[2] == 'n') {
    if (function_name[3] == 'd') {
     if (function_name[4] == 'e') {
      if (function_name[5] == 'r') {
       if (function_name[6] == 'R') {
        if (function_name[7] == 'e') {
         if (function_name[8] == 's') {
          if (function_name[9] == 'u') {
           if (function_name[10] == 'l') {
            if (function_name[11] == 't') {
             if (function_name[12] == '\0') {
              ++hp_globals.internal_match_counters.function;
              const char *class_name = hp_get_class_name(data TSRMLS_CC);
              if (!class_name) return NULL;
              if (!strcmp(class_name, "Magento\\Framework\\View\\Result\\Page\\Interceptor"))
               return &magento2_profiled_functions[6];
              ++hp_globals.internal_match_counters.class_unmatched;
              return NULL;
             }
             return NULL;
            }
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   if (function_name[2] == 'i') {
    if (function_name[3] == 'n') {
     if (function_name[4] == 'd') {
      if (function_name[5] == 'e') {
       if (function_name[6] == 'x') {
        if (function_name[7] == 'A') {
         if (function_name[8] == 'l') {
          if (function_name[9] == 'l') {
           if (function_name[10] == '\0') {
            ++hp_globals.internal_match_counters.function;
            const char *class_name = hp_get_class_name(data TSRMLS_CC);
            if (!class_name) return NULL;
            if (!strcmp(class_name, "Magento\\Indexer\\Model\\Indexer"))
             return &magento2_profiled_functions[12];
            ++hp_globals.internal_match_counters.class_unmatched;
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  if (function_name[1] == 'u') {
   if (function_name[2] == 'n') {
    if (function_name[3] == '\0') {
     ++hp_globals.internal_match_counters.function;
     const char *class_name = hp_get_class_name(data TSRMLS_CC);
     if (!class_name) return NULL;
     if (!strcmp(class_name, "Magento\\Framework\\App\\Bootstrap"))
      return &magento2_profiled_functions[0];
     ++hp_globals.internal_match_counters.class_unmatched;
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  return NULL;
 }
 return NULL;
}

static profiled_application_t magento2_profiled_application = {
"magento2",
magento2_profiled_functions,
14,
magento2_profiler_timers,
14,
&magento2_profiled_functions[1],
&magento2_profiled_functions[0],
NULL, NULL, NULL,
magento_init_context,
magento_cleanup_context,
magento2_match_function,
magento_send_metrics
};

static profiled_function_t magento_profiled_functions[] = {
{"Mage_Core_Model_App::__construct", 0, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Mage_Core_Model_App::run", 1, {0, 0}, magento1_fetch_version, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Mage_Core_Model_App::_initBaseConfig", 2, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Mage_Core_Model_App::_initCache", 3, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Mage_Core_Model_Config::loadModulesCache", 4, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Mage_Core_Model_Config::loadModules", 5, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Mage_Core_Model_Resource_Setup::applyAllUpdates", 6, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Mage_Core_Model_Config::loadDb", 7, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Mage_Core_Model_App::_initStores", 8, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Mage_Core_Controller_Varien_Action::preDispatch", 9, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Mage_Core_Controller_Varien_Action::loadLayoutUpdates", 10, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Mage_Core_Controller_Varien_Action::renderLayout", 11, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Mage_Core_Controller_Varien_Action::postDispatch", 12, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Mage_Core_Controller_Response_Http::sendResponse", 13, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Mage_Core_Block_Abstract::toHtml", 14, {1, 0}, magento1_block_before_render, magento_block_after_render, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Mage_Core_Model_Cache::flush", 15, {1, 0}, magento_record_cache_flush_event, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Mage_Adminhtml_CacheController::flushSystemAction", 16, {1, 0}, magento_record_cache_system_flush_event, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Mage_Core_Model_Cache::cleanType", 17, {1, 0}, magento1_record_cache_clean_event, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Mage_Index_Model_Process::reindexAll", 18, {1, 0}, NULL, magento1_record_reindex_event, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Mage_Index_Model_Indexer_Abstract::processEvent", 19, {0, 0}, magento1_record_process_index_event, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"PDOStatement::execute", 20, {1, 0}, NULL, magento_record_sql_query, {0, 0, 0, 0}, {0, 0, 0, 0}},
{NULL, 0, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}}
};

static const profiler_timer_t magento_profiler_timers[] = {
{
  "before_init_config",
  {&magento_profiled_functions[0], PROF_FIRST_START},
  {&magento_profiled_functions[2], PROF_FIRST_START},
  {0}
},
{
  "init_config",
  {&magento_profiled_functions[2], PROF_FIRST_START},
  {&magento_profiled_functions[2], PROF_LAST_STOP},
  {0}
},
{
  "init_cache",
  {&magento_profiled_functions[3], PROF_FIRST_START},
  {&magento_profiled_functions[3], PROF_LAST_STOP},
  {0}
},
{
  "load_modules",
  {&magento_profiled_functions[4], PROF_FIRST_START},
  {&magento_profiled_functions[5], PROF_LAST_STOP},
  {0}
},
{
  "db_updates",
  {&magento_profiled_functions[6], PROF_FIRST_START},
  {&magento_profiled_functions[6], PROF_LAST_STOP},
  {0}
},
{
  "load_db",
  {&magento_profiled_functions[7], PROF_FIRST_START},
  {&magento_profiled_functions[7], PROF_LAST_STOP},
  {0}
},
{
  "init_stores",
  {&magento_profiled_functions[8], PROF_FIRST_START},
  {&magento_profiled_functions[8], PROF_LAST_STOP},
  {0}
},
{
  "routing",
  {&magento_profiled_functions[8], PROF_LAST_STOP},
  {&magento_profiled_functions[9], PROF_FIRST_START},
  {0}
},
{
  "loading",
  {&magento_profiled_functions[0], PROF_FIRST_START},
  {&magento_profiled_functions[9], PROF_FIRST_START},
  {0}
},
{
  "before_layout_loading",
  {&magento_profiled_functions[9], PROF_FIRST_START},
  {&magento_profiled_functions[10], PROF_FIRST_START},
  {0}
},
{
  "layout_loading",
  {&magento_profiled_functions[10], PROF_FIRST_START},
  {&magento_profiled_functions[10], PROF_LAST_STOP},
  {0}
},
{
  "between_layout_loading_and_rendering",
  {&magento_profiled_functions[10], PROF_LAST_STOP},
  {&magento_profiled_functions[11], PROF_FIRST_START},
  {0}
},
{
  "layout_rendering",
  {&magento_profiled_functions[11], PROF_FIRST_START},
  {&magento_profiled_functions[11], PROF_LAST_STOP},
  {0}
},
{
  "after_layout_rendering",
  {&magento_profiled_functions[11], PROF_LAST_STOP},
  {&magento_profiled_functions[12], PROF_LAST_STOP},
  {0}
},
{
  "before_sending_response",
  {&magento_profiled_functions[12], PROF_LAST_STOP},
  {&magento_profiled_functions[13], PROF_FIRST_START},
  {0}
},
{
  "sending_response",
  {&magento_profiled_functions[13], PROF_FIRST_START},
  {&magento_profiled_functions[13], PROF_LAST_STOP},
  {0}
},
{
  "total",
  {&magento_profiled_functions[0], PROF_FIRST_START},
  {&magento_profiled_functions[1], PROF_LAST_STOP},
  {1}
},
{NULL, {0, 0}, {0, 0}, {0}}
};

profiled_function_t *magento_match_function(const char* function_name, zend_execute_data* data TSRMLS_DC) {
 ++hp_globals.internal_match_counters.total;
 if (function_name[0] == '_') {
  if (function_name[1] == '_') {
   if (function_name[2] == 'c') {
    if (function_name[3] == 'o') {
     if (function_name[4] == 'n') {
      if (function_name[5] == 's') {
       if (function_name[6] == 't') {
        if (function_name[7] == 'r') {
         if (function_name[8] == 'u') {
          if (function_name[9] == 'c') {
           if (function_name[10] == 't') {
            if (function_name[11] == '\0') {
             ++hp_globals.internal_match_counters.function;
             const char *class_name = hp_get_class_name(data TSRMLS_CC);
             if (!class_name) return NULL;
             if (!strcmp(class_name, "Mage_Core_Model_App"))
              return &magento_profiled_functions[0];
             ++hp_globals.internal_match_counters.class_unmatched;
             return NULL;
            }
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  if (function_name[1] == 'i') {
   if (function_name[2] == 'n') {
    if (function_name[3] == 'i') {
     if (function_name[4] == 't') {
      if (function_name[5] == 'B') {
       if (function_name[6] == 'a') {
        if (function_name[7] == 's') {
         if (function_name[8] == 'e') {
          if (function_name[9] == 'C') {
           if (function_name[10] == 'o') {
            if (function_name[11] == 'n') {
             if (function_name[12] == 'f') {
              if (function_name[13] == 'i') {
               if (function_name[14] == 'g') {
                if (function_name[15] == '\0') {
                 ++hp_globals.internal_match_counters.function;
                 const char *class_name = hp_get_class_name(data TSRMLS_CC);
                 if (!class_name) return NULL;
                 if (!strcmp(class_name, "Mage_Core_Model_App"))
                  return &magento_profiled_functions[2];
                 ++hp_globals.internal_match_counters.class_unmatched;
                 return NULL;
                }
                return NULL;
               }
               return NULL;
              }
              return NULL;
             }
             return NULL;
            }
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      if (function_name[5] == 'C') {
       if (function_name[6] == 'a') {
        if (function_name[7] == 'c') {
         if (function_name[8] == 'h') {
          if (function_name[9] == 'e') {
           if (function_name[10] == '\0') {
            ++hp_globals.internal_match_counters.function;
            const char *class_name = hp_get_class_name(data TSRMLS_CC);
            if (!class_name) return NULL;
            if (!strcmp(class_name, "Mage_Core_Model_App"))
             return &magento_profiled_functions[3];
            ++hp_globals.internal_match_counters.class_unmatched;
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      if (function_name[5] == 'S') {
       if (function_name[6] == 't') {
        if (function_name[7] == 'o') {
         if (function_name[8] == 'r') {
          if (function_name[9] == 'e') {
           if (function_name[10] == 's') {
            if (function_name[11] == '\0') {
             ++hp_globals.internal_match_counters.function;
             const char *class_name = hp_get_class_name(data TSRMLS_CC);
             if (!class_name) return NULL;
             if (!strcmp(class_name, "Mage_Core_Model_App"))
              return &magento_profiled_functions[8];
             ++hp_globals.internal_match_counters.class_unmatched;
             return NULL;
            }
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  return NULL;
 }
 if (function_name[0] == 'r') {
  if (function_name[1] == 'u') {
   if (function_name[2] == 'n') {
    if (function_name[3] == '\0') {
     ++hp_globals.internal_match_counters.function;
     const char *class_name = hp_get_class_name(data TSRMLS_CC);
     if (!class_name) return NULL;
     if (!strcmp(class_name, "Mage_Core_Model_App"))
      return &magento_profiled_functions[1];
     ++hp_globals.internal_match_counters.class_unmatched;
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  if (function_name[1] == 'e') {
   if (function_name[2] == 'n') {
    if (function_name[3] == 'd') {
     if (function_name[4] == 'e') {
      if (function_name[5] == 'r') {
       if (function_name[6] == 'L') {
        if (function_name[7] == 'a') {
         if (function_name[8] == 'y') {
          if (function_name[9] == 'o') {
           if (function_name[10] == 'u') {
            if (function_name[11] == 't') {
             if (function_name[12] == '\0') {
              ++hp_globals.internal_match_counters.function;
              const char *class_name = hp_get_class_name(data TSRMLS_CC);
              if (!class_name) return NULL;
              if (!strcmp(class_name, "Mage_Core_Controller_Varien_Action"))
               return &magento_profiled_functions[11];
              ++hp_globals.internal_match_counters.class_unmatched;
              return NULL;
             }
             return NULL;
            }
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   if (function_name[2] == 'i') {
    if (function_name[3] == 'n') {
     if (function_name[4] == 'd') {
      if (function_name[5] == 'e') {
       if (function_name[6] == 'x') {
        if (function_name[7] == 'A') {
         if (function_name[8] == 'l') {
          if (function_name[9] == 'l') {
           if (function_name[10] == '\0') {
            ++hp_globals.internal_match_counters.function;
            const char *class_name = hp_get_class_name(data TSRMLS_CC);
            if (!class_name) return NULL;
            if (!strcmp(class_name, "Mage_Index_Model_Process"))
             return &magento_profiled_functions[18];
            ++hp_globals.internal_match_counters.class_unmatched;
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  return NULL;
 }
 if (function_name[0] == 'l') {
  if (function_name[1] == 'o') {
   if (function_name[2] == 'a') {
    if (function_name[3] == 'd') {
     if (function_name[4] == 'M') {
      if (function_name[5] == 'o') {
       if (function_name[6] == 'd') {
        if (function_name[7] == 'u') {
         if (function_name[8] == 'l') {
          if (function_name[9] == 'e') {
           if (function_name[10] == 's') {
            if (function_name[11] == 'C') {
             if (function_name[12] == 'a') {
              if (function_name[13] == 'c') {
               if (function_name[14] == 'h') {
                if (function_name[15] == 'e') {
                 if (function_name[16] == '\0') {
                  ++hp_globals.internal_match_counters.function;
                  const char *class_name = hp_get_class_name(data TSRMLS_CC);
                  if (!class_name) return NULL;
                  if (!strcmp(class_name, "Mage_Core_Model_Config"))
                   return &magento_profiled_functions[4];
                  ++hp_globals.internal_match_counters.class_unmatched;
                  return NULL;
                 }
                 return NULL;
                }
                return NULL;
               }
               return NULL;
              }
              return NULL;
             }
             return NULL;
            }
            if (function_name[11] == '\0') {
             ++hp_globals.internal_match_counters.function;
             const char *class_name = hp_get_class_name(data TSRMLS_CC);
             if (!class_name) return NULL;
             if (!strcmp(class_name, "Mage_Core_Model_Config"))
              return &magento_profiled_functions[5];
             ++hp_globals.internal_match_counters.class_unmatched;
             return NULL;
            }
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     if (function_name[4] == 'D') {
      if (function_name[5] == 'b') {
       if (function_name[6] == '\0') {
        ++hp_globals.internal_match_counters.function;
        const char *class_name = hp_get_class_name(data TSRMLS_CC);
        if (!class_name) return NULL;
        if (!strcmp(class_name, "Mage_Core_Model_Config"))
         return &magento_profiled_functions[7];
        ++hp_globals.internal_match_counters.class_unmatched;
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     if (function_name[4] == 'L') {
      if (function_name[5] == 'a') {
       if (function_name[6] == 'y') {
        if (function_name[7] == 'o') {
         if (function_name[8] == 'u') {
          if (function_name[9] == 't') {
           if (function_name[10] == 'U') {
            if (function_name[11] == 'p') {
             if (function_name[12] == 'd') {
              if (function_name[13] == 'a') {
               if (function_name[14] == 't') {
                if (function_name[15] == 'e') {
                 if (function_name[16] == 's') {
                  if (function_name[17] == '\0') {
                   ++hp_globals.internal_match_counters.function;
                   const char *class_name = hp_get_class_name(data TSRMLS_CC);
                   if (!class_name) return NULL;
                   if (!strcmp(class_name, "Mage_Core_Controller_Varien_Action"))
                    return &magento_profiled_functions[10];
                   ++hp_globals.internal_match_counters.class_unmatched;
                   return NULL;
                  }
                  return NULL;
                 }
                 return NULL;
                }
                return NULL;
               }
               return NULL;
              }
              return NULL;
             }
             return NULL;
            }
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  return NULL;
 }
 if (function_name[0] == 'a') {
  if (function_name[1] == 'p') {
   if (function_name[2] == 'p') {
    if (function_name[3] == 'l') {
     if (function_name[4] == 'y') {
      if (function_name[5] == 'A') {
       if (function_name[6] == 'l') {
        if (function_name[7] == 'l') {
         if (function_name[8] == 'U') {
          if (function_name[9] == 'p') {
           if (function_name[10] == 'd') {
            if (function_name[11] == 'a') {
             if (function_name[12] == 't') {
              if (function_name[13] == 'e') {
               if (function_name[14] == 's') {
                if (function_name[15] == '\0') {
                 ++hp_globals.internal_match_counters.function;
                 const char *class_name = hp_get_class_name(data TSRMLS_CC);
                 if (!class_name) return NULL;
                 if (!strcmp(class_name, "Mage_Core_Model_Resource_Setup"))
                  return &magento_profiled_functions[6];
                 ++hp_globals.internal_match_counters.class_unmatched;
                 return NULL;
                }
                return NULL;
               }
               return NULL;
              }
              return NULL;
             }
             return NULL;
            }
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  return NULL;
 }
 if (function_name[0] == 'p') {
  if (function_name[1] == 'r') {
   if (function_name[2] == 'e') {
    if (function_name[3] == 'D') {
     if (function_name[4] == 'i') {
      if (function_name[5] == 's') {
       if (function_name[6] == 'p') {
        if (function_name[7] == 'a') {
         if (function_name[8] == 't') {
          if (function_name[9] == 'c') {
           if (function_name[10] == 'h') {
            if (function_name[11] == '\0') {
             ++hp_globals.internal_match_counters.function;
             const char *class_name = hp_get_class_name(data TSRMLS_CC);
             if (!class_name) return NULL;
             if (!strcmp(class_name, "Mage_Core_Controller_Varien_Action"))
              return &magento_profiled_functions[9];
             ++hp_globals.internal_match_counters.class_unmatched;
             return NULL;
            }
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   if (function_name[2] == 'o') {
    if (function_name[3] == 'c') {
     if (function_name[4] == 'e') {
      if (function_name[5] == 's') {
       if (function_name[6] == 's') {
        if (function_name[7] == 'E') {
         if (function_name[8] == 'v') {
          if (function_name[9] == 'e') {
           if (function_name[10] == 'n') {
            if (function_name[11] == 't') {
             if (function_name[12] == '\0') {
              ++hp_globals.internal_match_counters.function;
              const char *class_name = hp_get_class_name(data TSRMLS_CC);
              if (!class_name) return NULL;
              if (!strcmp(class_name, "Mage_Index_Model_Indexer_Abstract"))
               return &magento_profiled_functions[19];
              ++hp_globals.internal_match_counters.class_unmatched;
              return NULL;
             }
             return NULL;
            }
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  if (function_name[1] == 'o') {
   if (function_name[2] == 's') {
    if (function_name[3] == 't') {
     if (function_name[4] == 'D') {
      if (function_name[5] == 'i') {
       if (function_name[6] == 's') {
        if (function_name[7] == 'p') {
         if (function_name[8] == 'a') {
          if (function_name[9] == 't') {
           if (function_name[10] == 'c') {
            if (function_name[11] == 'h') {
             if (function_name[12] == '\0') {
              ++hp_globals.internal_match_counters.function;
              const char *class_name = hp_get_class_name(data TSRMLS_CC);
              if (!class_name) return NULL;
              if (!strcmp(class_name, "Mage_Core_Controller_Varien_Action"))
               return &magento_profiled_functions[12];
              ++hp_globals.internal_match_counters.class_unmatched;
              return NULL;
             }
             return NULL;
            }
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  return NULL;
 }
 if (function_name[0] == 's') {
  if (function_name[1] == 'e') {
   if (function_name[2] == 'n') {
    if (function_name[3] == 'd') {
     if (function_name[4] == 'R') {
      if (function_name[5] == 'e') {
       if (function_name[6] == 's') {
        if (function_name[7] == 'p') {
         if (function_name[8] == 'o') {
          if (function_name[9] == 'n') {
           if (function_name[10] == 's') {
            if (function_name[11] == 'e') {
             if (function_name[12] == '\0') {
              ++hp_globals.internal_match_counters.function;
              const char *class_name = hp_get_class_name(data TSRMLS_CC);
              if (!class_name) return NULL;
              if (!strcmp(class_name, "Mage_Core_Controller_Response_Http"))
               return &magento_profiled_functions[13];
              ++hp_globals.internal_match_counters.class_unmatched;
              return NULL;
             }
             return NULL;
            }
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  return NULL;
 }
 if (function_name[0] == 't') {
  if (function_name[1] == 'o') {
   if (function_name[2] == 'H') {
    if (function_name[3] == 't') {
     if (function_name[4] == 'm') {
      if (function_name[5] == 'l') {
       if (function_name[6] == '\0') {
        ++hp_globals.internal_match_counters.function;
        const char *class_name = hp_get_class_name(data TSRMLS_CC);
        if (!class_name) return NULL;
        if (!strcmp(class_name, "Mage_Core_Block_Abstract"))
         return &magento_profiled_functions[14];
        ++hp_globals.internal_match_counters.class_unmatched;
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  return NULL;
 }
 if (function_name[0] == 'f') {
  if (function_name[1] == 'l') {
   if (function_name[2] == 'u') {
    if (function_name[3] == 's') {
     if (function_name[4] == 'h') {
      if (function_name[5] == '\0') {
       ++hp_globals.internal_match_counters.function;
       const char *class_name = hp_get_class_name(data TSRMLS_CC);
       if (!class_name) return NULL;
       if (!strcmp(class_name, "Mage_Core_Model_Cache"))
        return &magento_profiled_functions[15];
       ++hp_globals.internal_match_counters.class_unmatched;
       return NULL;
      }
      if (function_name[5] == 'S') {
       if (function_name[6] == 'y') {
        if (function_name[7] == 's') {
         if (function_name[8] == 't') {
          if (function_name[9] == 'e') {
           if (function_name[10] == 'm') {
            if (function_name[11] == 'A') {
             if (function_name[12] == 'c') {
              if (function_name[13] == 't') {
               if (function_name[14] == 'i') {
                if (function_name[15] == 'o') {
                 if (function_name[16] == 'n') {
                  if (function_name[17] == '\0') {
                   ++hp_globals.internal_match_counters.function;
                   const char *class_name = hp_get_class_name(data TSRMLS_CC);
                   if (!class_name) return NULL;
                   if (!strcmp(class_name, "Mage_Adminhtml_CacheController"))
                    return &magento_profiled_functions[16];
                   ++hp_globals.internal_match_counters.class_unmatched;
                   return NULL;
                  }
                  return NULL;
                 }
                 return NULL;
                }
                return NULL;
               }
               return NULL;
              }
              return NULL;
             }
             return NULL;
            }
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  return NULL;
 }
 if (function_name[0] == 'c') {
  if (function_name[1] == 'l') {
   if (function_name[2] == 'e') {
    if (function_name[3] == 'a') {
     if (function_name[4] == 'n') {
      if (function_name[5] == 'T') {
       if (function_name[6] == 'y') {
        if (function_name[7] == 'p') {
         if (function_name[8] == 'e') {
          if (function_name[9] == '\0') {
           ++hp_globals.internal_match_counters.function;
           const char *class_name = hp_get_class_name(data TSRMLS_CC);
           if (!class_name) return NULL;
           if (!strcmp(class_name, "Mage_Core_Model_Cache"))
            return &magento_profiled_functions[17];
           ++hp_globals.internal_match_counters.class_unmatched;
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  return NULL;
 }
 if (function_name[0] == 'e') {
  if (function_name[1] == 'x') {
   if (function_name[2] == 'e') {
    if (function_name[3] == 'c') {
     if (function_name[4] == 'u') {
      if (function_name[5] == 't') {
       if (function_name[6] == 'e') {
        if (function_name[7] == '\0') {
         ++hp_globals.internal_match_counters.function;
         const char *class_name = hp_get_class_name(data TSRMLS_CC);
         if (!class_name) return NULL;
         if (!strcmp(class_name, "PDOStatement"))
          return &magento_profiled_functions[20];
         ++hp_globals.internal_match_counters.class_unmatched;
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  return NULL;
 }
 return NULL;
}

static profiled_application_t magento_profiled_application = {
"magento",
magento_profiled_functions,
21,
magento_profiler_timers,
17,
&magento_profiled_functions[0],
&magento_profiled_functions[1],
NULL, NULL, NULL,
magento_init_context,
magento_cleanup_context,
magento_match_function,
magento_send_metrics
};

profiled_application_t *qm_match_first_app_function(const char* function_name,
zend_execute_data* data TSRMLS_DC) {
 ++hp_globals.internal_match_counters.total;
 if (function_name[0] == 'c') {
  if (function_name[1] == 'r') {
   if (function_name[2] == 'e') {
    if (function_name[3] == 'a') {
     if (function_name[4] == 't') {
      if (function_name[5] == 'e') {
       if (function_name[6] == '\0') {
        ++hp_globals.internal_match_counters.function;
        const char *class_name = hp_get_class_name(data TSRMLS_CC);
        if (!class_name) return NULL;
        if (!strcmp(class_name, "Magento\\Framework\\App\\Bootstrap"))
         return &magento2_profiled_application;
        ++hp_globals.internal_match_counters.class_unmatched;
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  return NULL;
 }
 if (function_name[0] == '_') {
  if (function_name[1] == '_') {
   if (function_name[2] == 'c') {
    if (function_name[3] == 'o') {
     if (function_name[4] == 'n') {
      if (function_name[5] == 's') {
       if (function_name[6] == 't') {
        if (function_name[7] == 'r') {
         if (function_name[8] == 'u') {
          if (function_name[9] == 'c') {
           if (function_name[10] == 't') {
            if (function_name[11] == '\0') {
             ++hp_globals.internal_match_counters.function;
             const char *class_name = hp_get_class_name(data TSRMLS_CC);
             if (!class_name) return NULL;
             if (!strcmp(class_name, "Mage_Core_Model_App"))
              return &magento_profiled_application;
             ++hp_globals.internal_match_counters.class_unmatched;
             return NULL;
            }
            return NULL;
           }
           return NULL;
          }
          return NULL;
         }
         return NULL;
        }
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     return NULL;
    }
    return NULL;
   }
   return NULL;
  }
  return NULL;
 }
 return NULL;
}

