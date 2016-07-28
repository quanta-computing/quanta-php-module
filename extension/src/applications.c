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
{"Magento\\Framework\\View\\Layout::_renderBlock", 8, {0, 0}, magento2_block_before_render, magento2_block_after_render, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushAll::execute", 9, {0, 0}, magento_record_cache_flush_event, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushSystem::execute", 10, {0, 0}, magento_record_cache_system_flush_event, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Magento\\Framework\\App\\Cache\\TypeList::cleanType", 11, {0, 0}, magento2_record_cache_clean_event, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"Magento\\Indexer\\Model\\Indexer::reindexAll", 12, {0, 0}, NULL, magento2_record_reindex_event, {0, 0, 0, 0}, {0, 0, 0, 0}},
{"PDOStatement::execute", 13, {1, 0}, NULL, magento_record_sql_query, {0, 0, 0, 0}, {0, 0, 0, 0}},
{NULL, 0, {0, 0}, NULL, NULL, {0, 0, 0, 0}, {0, 0, 0, 0}}
};

static const profiler_timer_t magento2_profiler_timers[] = {
{
  "create_bootstrap",
  {&magento2_profiled_functions[1], PROF_FIRST_START},
  {&magento2_profiled_functions[1], PROF_LAST_STOP}
},
{
  "create_application",
  {&magento2_profiled_functions[2], PROF_FIRST_START},
  {&magento2_profiled_functions[2], PROF_LAST_STOP}
},
{
  "after_create_application",
  {&magento2_profiled_functions[2], PROF_LAST_STOP},
  {&magento2_profiled_functions[3], PROF_FIRST_START}
},
{
  "parse_request",
  {&magento2_profiled_functions[3], PROF_FIRST_START},
  {&magento2_profiled_functions[3], PROF_LAST_STOP}
},
{
  "configure_area",
  {&magento2_profiled_functions[3], PROF_LAST_STOP},
  {&magento2_profiled_functions[4], PROF_FIRST_START}
},
{
  "loading",
  {&magento2_profiled_functions[1], PROF_FIRST_START},
  {&magento2_profiled_functions[4], PROF_FIRST_START}
},
{
  "dispatch",
  {&magento2_profiled_functions[4], PROF_FIRST_START},
  {&magento2_profiled_functions[4], PROF_LAST_STOP}
},
{
  "routing",
  {&magento2_profiled_functions[4], PROF_FIRST_START},
  {&magento2_profiled_functions[5], PROF_FIRST_START}
},
{
  "controller",
  {&magento2_profiled_functions[5], PROF_FIRST_START},
  {&magento2_profiled_functions[5], PROF_LAST_STOP}
},
{
  "between_controller_and_layout_rendering",
  {&magento2_profiled_functions[5], PROF_LAST_STOP},
  {&magento2_profiled_functions[6], PROF_FIRST_START}
},
{
  "layout_rendering",
  {&magento2_profiled_functions[6], PROF_FIRST_START},
  {&magento2_profiled_functions[6], PROF_LAST_STOP}
},
{
  "before_sending_response",
  {&magento2_profiled_functions[6], PROF_LAST_STOP},
  {&magento2_profiled_functions[7], PROF_FIRST_START}
},
{
  "sending_response",
  {&magento2_profiled_functions[7], PROF_FIRST_START},
  {&magento2_profiled_functions[7], PROF_LAST_STOP}
},
{
  "total",
  {&magento2_profiled_functions[1], PROF_FIRST_START},
  {&magento2_profiled_functions[0], PROF_LAST_STOP}
},
{NULL, {0, 0}, {0, 0}}
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
NULL, NULL, NULL,
magento_init_context,
magento_cleanup_context,
magento2_match_function,
magento_send_metrics
};

void register_application(void) {
   hp_globals.profiled_application = &magento2_profiled_application;
   init_profiled_application(&magento2_profiled_application);
}
