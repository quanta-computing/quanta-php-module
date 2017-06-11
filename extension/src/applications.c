#include "quanta_mon.h"

static profiled_function_t orocommerce_profiled_functions[] = {
{"Oro\\Bundle\\DistributionBundle\\OroKernel::boot", 0, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Symfony\\Component\\HttpKernel\\HttpKernel::handle", 1, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Symfony\\Component\\HttpKernel\\EventListener\\RouterListener::onKernelRequest", 2, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Symfony\\Component\\HttpKernel\\Controller\\ControllerResolver::getController", 3, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Oro\\Bundle\\LayoutBundle\\EventListener\\LayoutListener::onKernelView", 4, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Oro\\Bundle\\LayoutBundle\\Layout\\LayoutManager::getLayout", 5, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Oro\\Component\\Layout\\Layout::render", 6, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Symfony\\Component\\HttpKernel\\HttpKernel::filterResponse", 7, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Symfony\\Component\\HttpFoundation\\Response::send", 8, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Symfony\\Component\\HttpKernel\\Kernel::terminate", 9, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Oro\\Component\\Layout\\LayoutBuilder::processBlockViewData", 10, {1, QUANTA_MON_MODE_APP_PROFILING}, oro_before_process_block, NULL, {0, 0, 0, 0}},
{"Oro\\Bundle\\LayoutBundle\\Form\\TwigRendererEngine::renderBlock", 11, {1, QUANTA_MON_MODE_APP_PROFILING}, oro_before_render_block, oro_after_render_block, {0, 0, 0, 0}},
{"PDOStatement::execute", 12, {1, QUANTA_MON_MODE_APP_PROFILING}, NULL, oro_record_sql_query, {0, 0, 0, 0}},
{NULL, 0, {0, 0}, NULL, NULL, {0, 0, 0, 0}}
};

static const profiler_timer_function_t orocommerce_profiler_timer_kernel_boot_start[] = {
  {&orocommerce_profiled_functions[0], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_handle_request_start[] = {
  {&orocommerce_profiled_functions[0], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_routing_start[] = {
  {&orocommerce_profiled_functions[2], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_after_routing_start[] = {
  {&orocommerce_profiled_functions[2], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_controller_start[] = {
  {&orocommerce_profiled_functions[3], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_layout_loading_start[] = {
  {&orocommerce_profiled_functions[4], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_init_rendering_start[] = {
  {&orocommerce_profiled_functions[6], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_layout_rendering_start[] = {
  {&orocommerce_profiled_functions[11], PROF_FIRST_START},
  {&orocommerce_profiled_functions[6], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_filter_response_start[] = {
  {&orocommerce_profiled_functions[7], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_sending_response_start[] = {
  {&orocommerce_profiled_functions[8], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_total_start[] = {
  {&orocommerce_profiled_functions[0], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_kernel_boot_end[] = {
  {&orocommerce_profiled_functions[0], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_handle_request_end[] = {
  {&orocommerce_profiled_functions[2], PROF_FIRST_START},
  {&orocommerce_profiled_functions[3], PROF_FIRST_START},
  {&orocommerce_profiled_functions[4], PROF_FIRST_START},
  {&orocommerce_profiled_functions[7], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_routing_end[] = {
  {&orocommerce_profiled_functions[2], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_after_routing_end[] = {
  {&orocommerce_profiled_functions[3], PROF_FIRST_START},
  {&orocommerce_profiled_functions[4], PROF_FIRST_START},
  {&orocommerce_profiled_functions[7], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_controller_end[] = {
  {&orocommerce_profiled_functions[4], PROF_FIRST_START},
  {&orocommerce_profiled_functions[7], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_layout_loading_end[] = {
  {&orocommerce_profiled_functions[5], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_init_rendering_end[] = {
  {&orocommerce_profiled_functions[11], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_layout_rendering_end[] = {
  {&orocommerce_profiled_functions[6], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_filter_response_end[] = {
  {&orocommerce_profiled_functions[8], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_sending_response_end[] = {
  {&orocommerce_profiled_functions[9], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t orocommerce_profiler_timer_total_end[] = {
  {&orocommerce_profiled_functions[9], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_t orocommerce_profiler_timers[] = {
{
  "kernel_boot",
  orocommerce_profiler_timer_kernel_boot_start, 1,
  orocommerce_profiler_timer_kernel_boot_end, 1,
  {0}
},
{
  "handle_request",
  orocommerce_profiler_timer_handle_request_start, 1,
  orocommerce_profiler_timer_handle_request_end, 4,
  {0}
},
{
  "routing",
  orocommerce_profiler_timer_routing_start, 1,
  orocommerce_profiler_timer_routing_end, 1,
  {0}
},
{
  "after_routing",
  orocommerce_profiler_timer_after_routing_start, 1,
  orocommerce_profiler_timer_after_routing_end, 3,
  {0}
},
{
  "controller",
  orocommerce_profiler_timer_controller_start, 1,
  orocommerce_profiler_timer_controller_end, 2,
  {0}
},
{
  "layout_loading",
  orocommerce_profiler_timer_layout_loading_start, 1,
  orocommerce_profiler_timer_layout_loading_end, 1,
  {0}
},
{
  "init_rendering",
  orocommerce_profiler_timer_init_rendering_start, 1,
  orocommerce_profiler_timer_init_rendering_end, 1,
  {0}
},
{
  "layout_rendering",
  orocommerce_profiler_timer_layout_rendering_start, 2,
  orocommerce_profiler_timer_layout_rendering_end, 1,
  {0}
},
{
  "filter_response",
  orocommerce_profiler_timer_filter_response_start, 1,
  orocommerce_profiler_timer_filter_response_end, 1,
  {0}
},
{
  "sending_response",
  orocommerce_profiler_timer_sending_response_start, 1,
  orocommerce_profiler_timer_sending_response_end, 1,
  {0}
},
{
  "total",
  orocommerce_profiler_timer_total_start, 1,
  orocommerce_profiler_timer_total_end, 1,
  {0}
},
{NULL, NULL, 0, NULL, 0, {0}}
};

profiled_function_t *orocommerce_match_function(const char* function_name, zend_execute_data* data TSRMLS_DC) {
 ++hp_globals.internal_match_counters.total;
 if (function_name[0] == 'b') {
  if (function_name[1] == 'o') {
   if (function_name[2] == 'o') {
    if (function_name[3] == 't') {
     if (function_name[4] == '\0') {
      ++hp_globals.internal_match_counters.function;
      const char *class_name = hp_get_class_name(data TSRMLS_CC);
      if (!class_name) return NULL;
      if (!strcmp(class_name, "Oro\\Bundle\\DistributionBundle\\OroKernel"))
       return &orocommerce_profiled_functions[0];
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
 if (function_name[0] == 'h') {
  if (function_name[1] == 'a') {
   if (function_name[2] == 'n') {
    if (function_name[3] == 'd') {
     if (function_name[4] == 'l') {
      if (function_name[5] == 'e') {
       if (function_name[6] == '\0') {
        ++hp_globals.internal_match_counters.function;
        const char *class_name = hp_get_class_name(data TSRMLS_CC);
        if (!class_name) return NULL;
        if (!strcmp(class_name, "Symfony\\Component\\HttpKernel\\HttpKernel"))
         return &orocommerce_profiled_functions[1];
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
 if (function_name[0] == 'o') {
  if (function_name[1] == 'n') {
   if (function_name[2] == 'K') {
    if (function_name[3] == 'e') {
     if (function_name[4] == 'r') {
      if (function_name[5] == 'n') {
       if (function_name[6] == 'e') {
        if (function_name[7] == 'l') {
         if (function_name[8] == 'R') {
          if (function_name[9] == 'e') {
           if (function_name[10] == 'q') {
            if (function_name[11] == 'u') {
             if (function_name[12] == 'e') {
              if (function_name[13] == 's') {
               if (function_name[14] == 't') {
                if (function_name[15] == '\0') {
                 ++hp_globals.internal_match_counters.function;
                 const char *class_name = hp_get_class_name(data TSRMLS_CC);
                 if (!class_name) return NULL;
                 if (!strcmp(class_name, "Symfony\\Component\\HttpKernel\\EventListener\\RouterListener"))
                  return &orocommerce_profiled_functions[2];
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
         if (function_name[8] == 'V') {
          if (function_name[9] == 'i') {
           if (function_name[10] == 'e') {
            if (function_name[11] == 'w') {
             if (function_name[12] == '\0') {
              ++hp_globals.internal_match_counters.function;
              const char *class_name = hp_get_class_name(data TSRMLS_CC);
              if (!class_name) return NULL;
              if (!strcmp(class_name, "Oro\\Bundle\\LayoutBundle\\EventListener\\LayoutListener"))
               return &orocommerce_profiled_functions[4];
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
    if (function_name[3] == 'C') {
     if (function_name[4] == 'o') {
      if (function_name[5] == 'n') {
       if (function_name[6] == 't') {
        if (function_name[7] == 'r') {
         if (function_name[8] == 'o') {
          if (function_name[9] == 'l') {
           if (function_name[10] == 'l') {
            if (function_name[11] == 'e') {
             if (function_name[12] == 'r') {
              if (function_name[13] == '\0') {
               ++hp_globals.internal_match_counters.function;
               const char *class_name = hp_get_class_name(data TSRMLS_CC);
               if (!class_name) return NULL;
               if (!strcmp(class_name, "Symfony\\Component\\HttpKernel\\Controller\\ControllerResolver"))
                return &orocommerce_profiled_functions[3];
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
    if (function_name[3] == 'L') {
     if (function_name[4] == 'a') {
      if (function_name[5] == 'y') {
       if (function_name[6] == 'o') {
        if (function_name[7] == 'u') {
         if (function_name[8] == 't') {
          if (function_name[9] == '\0') {
           ++hp_globals.internal_match_counters.function;
           const char *class_name = hp_get_class_name(data TSRMLS_CC);
           if (!class_name) return NULL;
           if (!strcmp(class_name, "Oro\\Bundle\\LayoutBundle\\Layout\\LayoutManager"))
            return &orocommerce_profiled_functions[5];
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
 if (function_name[0] == 'r') {
  if (function_name[1] == 'e') {
   if (function_name[2] == 'n') {
    if (function_name[3] == 'd') {
     if (function_name[4] == 'e') {
      if (function_name[5] == 'r') {
       if (function_name[6] == 'B') {
        if (function_name[7] == 'l') {
         if (function_name[8] == 'o') {
          if (function_name[9] == 'c') {
           if (function_name[10] == 'k') {
            if (function_name[11] == '\0') {
             ++hp_globals.internal_match_counters.function;
             const char *class_name = hp_get_class_name(data TSRMLS_CC);
             if (!class_name) return NULL;
             if (!strcmp(class_name, "Oro\\Bundle\\LayoutBundle\\Form\\TwigRendererEngine"))
              return &orocommerce_profiled_functions[11];
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
       if (function_name[6] == '\0') {
        ++hp_globals.internal_match_counters.function;
        const char *class_name = hp_get_class_name(data TSRMLS_CC);
        if (!class_name) return NULL;
        if (!strcmp(class_name, "Oro\\Component\\Layout\\Layout"))
         return &orocommerce_profiled_functions[6];
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
  if (function_name[1] == 'i') {
   if (function_name[2] == 'l') {
    if (function_name[3] == 't') {
     if (function_name[4] == 'e') {
      if (function_name[5] == 'r') {
       if (function_name[6] == 'R') {
        if (function_name[7] == 'e') {
         if (function_name[8] == 's') {
          if (function_name[9] == 'p') {
           if (function_name[10] == 'o') {
            if (function_name[11] == 'n') {
             if (function_name[12] == 's') {
              if (function_name[13] == 'e') {
               if (function_name[14] == '\0') {
                ++hp_globals.internal_match_counters.function;
                const char *class_name = hp_get_class_name(data TSRMLS_CC);
                if (!class_name) return NULL;
                if (!strcmp(class_name, "Symfony\\Component\\HttpKernel\\HttpKernel"))
                 return &orocommerce_profiled_functions[7];
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
 if (function_name[0] == 's') {
  if (function_name[1] == 'e') {
   if (function_name[2] == 'n') {
    if (function_name[3] == 'd') {
     if (function_name[4] == '\0') {
      ++hp_globals.internal_match_counters.function;
      const char *class_name = hp_get_class_name(data TSRMLS_CC);
      if (!class_name) return NULL;
      if (!strcmp(class_name, "Symfony\\Component\\HttpFoundation\\Response"))
       return &orocommerce_profiled_functions[8];
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
 if (function_name[0] == 't') {
  if (function_name[1] == 'e') {
   if (function_name[2] == 'r') {
    if (function_name[3] == 'm') {
     if (function_name[4] == 'i') {
      if (function_name[5] == 'n') {
       if (function_name[6] == 'a') {
        if (function_name[7] == 't') {
         if (function_name[8] == 'e') {
          if (function_name[9] == '\0') {
           ++hp_globals.internal_match_counters.function;
           const char *class_name = hp_get_class_name(data TSRMLS_CC);
           if (!class_name) return NULL;
           if (!strcmp(class_name, "Symfony\\Component\\HttpKernel\\Kernel"))
            return &orocommerce_profiled_functions[9];
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
 if (function_name[0] == 'p') {
  if (function_name[1] == 'r') {
   if (function_name[2] == 'o') {
    if (function_name[3] == 'c') {
     if (function_name[4] == 'e') {
      if (function_name[5] == 's') {
       if (function_name[6] == 's') {
        if (function_name[7] == 'B') {
         if (function_name[8] == 'l') {
          if (function_name[9] == 'o') {
           if (function_name[10] == 'c') {
            if (function_name[11] == 'k') {
             if (function_name[12] == 'V') {
              if (function_name[13] == 'i') {
               if (function_name[14] == 'e') {
                if (function_name[15] == 'w') {
                 if (function_name[16] == 'D') {
                  if (function_name[17] == 'a') {
                   if (function_name[18] == 't') {
                    if (function_name[19] == 'a') {
                     if (function_name[20] == '\0') {
                      ++hp_globals.internal_match_counters.function;
                      const char *class_name = hp_get_class_name(data TSRMLS_CC);
                      if (!class_name) return NULL;
                      if (!strcmp(class_name, "Oro\\Component\\Layout\\LayoutBuilder"))
                       return &orocommerce_profiled_functions[10];
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
          return &orocommerce_profiled_functions[12];
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

static profiled_application_t orocommerce_profiled_application = {
"orocommerce",
orocommerce_profiled_functions,
13,
orocommerce_profiler_timers,
11,
&orocommerce_profiled_functions[0],
&orocommerce_profiled_functions[9],
{NULL, NULL},
NULL,
oro_init_context,
oro_cleanup_context,
orocommerce_match_function,
oro_send_metrics
};

static profiled_function_t magento2_profiled_functions[] = {
{"Magento\\Framework\\App\\Bootstrap::run", 0, {0, QUANTA_MON_MODE_APP_PROFILING}, magento2_fetch_version, NULL, {0, 0, 0, 0}},
{"Magento\\Framework\\App\\Bootstrap::create", 1, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Magento\\Framework\\App\\Bootstrap::createApplication", 2, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Magento\\Framework\\App\\Request\\Http::getFrontName", 3, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Magento\\Framework\\App\\FrontController\\Interceptor::dispatch", 4, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Magento\\Framework\\App\\FrontController::dispatch", 5, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Magento\\Framework\\App\\Action\\Action::dispatch", 6, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Magento\\Framework\\View\\Result\\Page\\Interceptor::renderResult", 7, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Magento\\Framework\\App\\Response\\Http\\Interceptor::sendResponse", 8, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Magento\\Framework\\View\\Layout::_renderBlock", 9, {1, QUANTA_MON_MODE_APP_PROFILING}, magento2_block_before_render, magento_block_after_render, {0, 0, 0, 0}},
{"Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushAll::execute", 10, {1, QUANTA_MON_MODE_EVENTS_ONLY}, magento_record_cache_flush_event, NULL, {0, 0, 0, 0}},
{"Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushSystem::execute", 11, {1, QUANTA_MON_MODE_EVENTS_ONLY}, magento_record_cache_system_flush_event, NULL, {0, 0, 0, 0}},
{"Magento\\Framework\\App\\Cache\\TypeList::cleanType", 12, {1, QUANTA_MON_MODE_EVENTS_ONLY}, magento2_record_cache_clean_event, NULL, {0, 0, 0, 0}},
{"Magento\\Indexer\\Model\\Indexer::reindexAll", 13, {1, QUANTA_MON_MODE_EVENTS_ONLY}, NULL, magento2_record_reindex_event, {0, 0, 0, 0}},
{"PDOStatement::execute", 14, {1, QUANTA_MON_MODE_APP_PROFILING}, NULL, magento_record_sql_query, {0, 0, 0, 0}},
{NULL, 0, {0, 0}, NULL, NULL, {0, 0, 0, 0}}
};

static const profiler_timer_function_t magento2_profiler_timer_create_bootstrap_start[] = {
  {&magento2_profiled_functions[1], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_create_application_start[] = {
  {&magento2_profiled_functions[2], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_after_create_application_start[] = {
  {&magento2_profiled_functions[2], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_parse_request_start[] = {
  {&magento2_profiled_functions[3], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_configure_area_start[] = {
  {&magento2_profiled_functions[3], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_loading_start[] = {
  {&magento2_profiled_functions[1], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_dispatch_start[] = {
  {&magento2_profiled_functions[4], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_before_frontcontroller_dispatch_start[] = {
  {&magento2_profiled_functions[4], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_routing_start[] = {
  {&magento2_profiled_functions[5], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_controller_start[] = {
  {&magento2_profiled_functions[6], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_between_controller_and_layout_rendering_start[] = {
  {&magento2_profiled_functions[6], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_layout_rendering_start[] = {
  {&magento2_profiled_functions[7], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_before_sending_response_start[] = {
  {&magento2_profiled_functions[7], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_sending_response_start[] = {
  {&magento2_profiled_functions[8], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_total_start[] = {
  {&magento2_profiled_functions[1], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_create_bootstrap_end[] = {
  {&magento2_profiled_functions[1], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_create_application_end[] = {
  {&magento2_profiled_functions[2], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_after_create_application_end[] = {
  {&magento2_profiled_functions[3], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_parse_request_end[] = {
  {&magento2_profiled_functions[3], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_configure_area_end[] = {
  {&magento2_profiled_functions[4], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_loading_end[] = {
  {&magento2_profiled_functions[4], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_dispatch_end[] = {
  {&magento2_profiled_functions[4], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_before_frontcontroller_dispatch_end[] = {
  {&magento2_profiled_functions[5], PROF_FIRST_START},
  {&magento2_profiled_functions[4], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_routing_end[] = {
  {&magento2_profiled_functions[6], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_controller_end[] = {
  {&magento2_profiled_functions[6], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_between_controller_and_layout_rendering_end[] = {
  {&magento2_profiled_functions[7], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_layout_rendering_end[] = {
  {&magento2_profiled_functions[7], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_before_sending_response_end[] = {
  {&magento2_profiled_functions[8], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_sending_response_end[] = {
  {&magento2_profiled_functions[8], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento2_profiler_timer_total_end[] = {
  {&magento2_profiled_functions[0], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_t magento2_profiler_timers[] = {
{
  "create_bootstrap",
  magento2_profiler_timer_create_bootstrap_start, 1,
  magento2_profiler_timer_create_bootstrap_end, 1,
  {0}
},
{
  "create_application",
  magento2_profiler_timer_create_application_start, 1,
  magento2_profiler_timer_create_application_end, 1,
  {0}
},
{
  "after_create_application",
  magento2_profiler_timer_after_create_application_start, 1,
  magento2_profiler_timer_after_create_application_end, 1,
  {0}
},
{
  "parse_request",
  magento2_profiler_timer_parse_request_start, 1,
  magento2_profiler_timer_parse_request_end, 1,
  {0}
},
{
  "configure_area",
  magento2_profiler_timer_configure_area_start, 1,
  magento2_profiler_timer_configure_area_end, 1,
  {0}
},
{
  "loading",
  magento2_profiler_timer_loading_start, 1,
  magento2_profiler_timer_loading_end, 1,
  {0}
},
{
  "dispatch",
  magento2_profiler_timer_dispatch_start, 1,
  magento2_profiler_timer_dispatch_end, 1,
  {0}
},
{
  "before_frontcontroller_dispatch",
  magento2_profiler_timer_before_frontcontroller_dispatch_start, 1,
  magento2_profiler_timer_before_frontcontroller_dispatch_end, 2,
  {0}
},
{
  "routing",
  magento2_profiler_timer_routing_start, 1,
  magento2_profiler_timer_routing_end, 1,
  {0}
},
{
  "controller",
  magento2_profiler_timer_controller_start, 1,
  magento2_profiler_timer_controller_end, 1,
  {0}
},
{
  "between_controller_and_layout_rendering",
  magento2_profiler_timer_between_controller_and_layout_rendering_start, 1,
  magento2_profiler_timer_between_controller_and_layout_rendering_end, 1,
  {0}
},
{
  "layout_rendering",
  magento2_profiler_timer_layout_rendering_start, 1,
  magento2_profiler_timer_layout_rendering_end, 1,
  {0}
},
{
  "before_sending_response",
  magento2_profiler_timer_before_sending_response_start, 1,
  magento2_profiler_timer_before_sending_response_end, 1,
  {0}
},
{
  "sending_response",
  magento2_profiler_timer_sending_response_start, 1,
  magento2_profiler_timer_sending_response_end, 1,
  {0}
},
{
  "total",
  magento2_profiler_timer_total_start, 1,
  magento2_profiler_timer_total_end, 1,
  {0}
},
{NULL, NULL, 0, NULL, 0, {0}}
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
               return &magento2_profiled_functions[9];
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
          return &magento2_profiled_functions[14];
         if (!strcmp(class_name, "Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushAll"))
          return &magento2_profiled_functions[10];
         if (!strcmp(class_name, "Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushSystem"))
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
           return &magento2_profiled_functions[6];
          if (!strcmp(class_name, "Magento\\Framework\\App\\FrontController"))
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
             return &magento2_profiled_functions[13];
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
15,
magento2_profiler_timers,
15,
&magento2_profiled_functions[1],
&magento2_profiled_functions[0],
{NULL, NULL},
NULL,
magento_init_context,
magento_cleanup_context,
magento2_match_function,
magento_send_metrics
};

static profiled_function_t magento_profiled_functions[] = {
{"Mage_Core_Model_App::__construct", 0, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Mage_Core_Model_App::run", 1, {0, QUANTA_MON_MODE_APP_PROFILING}, magento1_fetch_version, NULL, {0, 0, 0, 0}},
{"Mage_Core_Model_App::_initBaseConfig", 2, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Mage_Core_Model_App::_initCache", 3, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Mage_Core_Model_Cache::processRequest", 4, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Mage_Core_Model_Config::loadModulesCache", 5, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Mage_Core_Model_Config::loadModules", 6, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Mage_Core_Model_Resource_Setup::applyAllUpdates", 7, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Mage_Core_Model_Config::loadDb", 8, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Mage_Core_Model_App::loadAreaPart", 9, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Mage_Core_Model_App::_initStores", 10, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Mage_Core_Model_App::_initRequest", 11, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Mage_Core_Model_Resource_Setup::applyAllDataUpdates", 12, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Mage_Core_Controller_Varien_Action::preDispatch", 13, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Mage_Core_Controller_Varien_Action::loadLayoutUpdates", 14, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Mage_Core_Controller_Varien_Action::renderLayout", 15, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Mage_Core_Controller_Varien_Action::postDispatch", 16, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Mage_Core_Controller_Response_Http::sendResponse", 17, {0, QUANTA_MON_MODE_APP_PROFILING}, NULL, NULL, {0, 0, 0, 0}},
{"Mage_Core_Block_Abstract::toHtml", 18, {1, QUANTA_MON_MODE_APP_PROFILING}, magento1_block_before_render, magento_block_after_render, {0, 0, 0, 0}},
{"Mage_Core_Model_Cache::flush", 19, {1, QUANTA_MON_MODE_EVENTS_ONLY}, magento_record_cache_flush_event, NULL, {0, 0, 0, 0}},
{"Mage_Adminhtml_CacheController::flushSystemAction", 20, {1, QUANTA_MON_MODE_EVENTS_ONLY}, magento_record_cache_system_flush_event, NULL, {0, 0, 0, 0}},
{"Mage_Core_Model_Cache::cleanType", 21, {1, QUANTA_MON_MODE_EVENTS_ONLY}, magento1_record_cache_clean_event, NULL, {0, 0, 0, 0}},
{"Mage_Index_Model_Process::reindexAll", 22, {1, QUANTA_MON_MODE_EVENTS_ONLY}, NULL, magento1_record_reindex_event, {0, 0, 0, 0}},
{"Mage_Index_Model_Indexer_Abstract::processEvent", 23, {0, QUANTA_MON_MODE_EVENTS_ONLY}, magento1_record_process_index_event, NULL, {0, 0, 0, 0}},
{"PDOStatement::execute", 24, {1, QUANTA_MON_MODE_APP_PROFILING}, NULL, magento_record_sql_query, {0, 0, 0, 0}},
{NULL, 0, {0, 0}, NULL, NULL, {0, 0, 0, 0}}
};

static const profiler_timer_function_t magento_profiler_timer_before_init_config_start[] = {
  {&magento_profiled_functions[0], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_init_config_start[] = {
  {&magento_profiled_functions[2], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_init_cache_start[] = {
  {&magento_profiled_functions[3], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_fpc_process_request_start[] = {
  {&magento_profiled_functions[4], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_load_modules_start[] = {
  {&magento_profiled_functions[5], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_db_updates_start[] = {
  {&magento_profiled_functions[7], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_load_db_start[] = {
  {&magento_profiled_functions[8], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_load_area_start[] = {
  {&magento_profiled_functions[9], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_init_stores_start[] = {
  {&magento_profiled_functions[10], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_init_request_start[] = {
  {&magento_profiled_functions[11], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_db_data_updates_start[] = {
  {&magento_profiled_functions[12], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_routing_start[] = {
  {&magento_profiled_functions[12], PROF_LAST_STOP},
  {&magento_profiled_functions[9], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_loading_start[] = {
  {&magento_profiled_functions[0], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_before_layout_loading_start[] = {
  {&magento_profiled_functions[13], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_layout_loading_start[] = {
  {&magento_profiled_functions[14], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_between_layout_loading_and_rendering_start[] = {
  {&magento_profiled_functions[14], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_layout_rendering_start[] = {
  {&magento_profiled_functions[15], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_after_layout_rendering_start[] = {
  {&magento_profiled_functions[15], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_controller_start[] = {
  {&magento_profiled_functions[13], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_before_sending_response_start[] = {
  {&magento_profiled_functions[16], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_sending_response_start[] = {
  {&magento_profiled_functions[17], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_total_start[] = {
  {&magento_profiled_functions[0], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_before_init_config_end[] = {
  {&magento_profiled_functions[2], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_init_config_end[] = {
  {&magento_profiled_functions[2], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_init_cache_end[] = {
  {&magento_profiled_functions[3], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_fpc_process_request_end[] = {
  {&magento_profiled_functions[4], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_load_modules_end[] = {
  {&magento_profiled_functions[6], PROF_LAST_STOP},
  {&magento_profiled_functions[5], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_db_updates_end[] = {
  {&magento_profiled_functions[7], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_load_db_end[] = {
  {&magento_profiled_functions[8], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_load_area_end[] = {
  {&magento_profiled_functions[9], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_init_stores_end[] = {
  {&magento_profiled_functions[10], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_init_request_end[] = {
  {&magento_profiled_functions[11], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_db_data_updates_end[] = {
  {&magento_profiled_functions[12], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_routing_end[] = {
  {&magento_profiled_functions[13], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_loading_end[] = {
  {&magento_profiled_functions[13], PROF_FIRST_START},
  {&magento_profiled_functions[10], PROF_LAST_STOP},
  {&magento_profiled_functions[8], PROF_LAST_STOP},
  {&magento_profiled_functions[7], PROF_LAST_STOP},
  {&magento_profiled_functions[6], PROF_LAST_STOP},
  {&magento_profiled_functions[5], PROF_LAST_STOP},
  {&magento_profiled_functions[4], PROF_LAST_STOP},
  {&magento_profiled_functions[3], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_before_layout_loading_end[] = {
  {&magento_profiled_functions[14], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_layout_loading_end[] = {
  {&magento_profiled_functions[14], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_between_layout_loading_and_rendering_end[] = {
  {&magento_profiled_functions[15], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_layout_rendering_end[] = {
  {&magento_profiled_functions[15], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_after_layout_rendering_end[] = {
  {&magento_profiled_functions[16], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_controller_end[] = {
  {&magento_profiled_functions[16], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_before_sending_response_end[] = {
  {&magento_profiled_functions[17], PROF_FIRST_START},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_sending_response_end[] = {
  {&magento_profiled_functions[1], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_function_t magento_profiler_timer_total_end[] = {
  {&magento_profiled_functions[1], PROF_LAST_STOP},
  {NULL, 0}
};
static const profiler_timer_t magento_profiler_timers[] = {
{
  "before_init_config",
  magento_profiler_timer_before_init_config_start, 1,
  magento_profiler_timer_before_init_config_end, 1,
  {0}
},
{
  "init_config",
  magento_profiler_timer_init_config_start, 1,
  magento_profiler_timer_init_config_end, 1,
  {0}
},
{
  "init_cache",
  magento_profiler_timer_init_cache_start, 1,
  magento_profiler_timer_init_cache_end, 1,
  {0}
},
{
  "fpc_process_request",
  magento_profiler_timer_fpc_process_request_start, 1,
  magento_profiler_timer_fpc_process_request_end, 1,
  {0}
},
{
  "load_modules",
  magento_profiler_timer_load_modules_start, 1,
  magento_profiler_timer_load_modules_end, 2,
  {0}
},
{
  "db_updates",
  magento_profiler_timer_db_updates_start, 1,
  magento_profiler_timer_db_updates_end, 1,
  {0}
},
{
  "load_db",
  magento_profiler_timer_load_db_start, 1,
  magento_profiler_timer_load_db_end, 1,
  {0}
},
{
  "load_area",
  magento_profiler_timer_load_area_start, 1,
  magento_profiler_timer_load_area_end, 1,
  {0}
},
{
  "init_stores",
  magento_profiler_timer_init_stores_start, 1,
  magento_profiler_timer_init_stores_end, 1,
  {0}
},
{
  "init_request",
  magento_profiler_timer_init_request_start, 1,
  magento_profiler_timer_init_request_end, 1,
  {0}
},
{
  "db_data_updates",
  magento_profiler_timer_db_data_updates_start, 1,
  magento_profiler_timer_db_data_updates_end, 1,
  {0}
},
{
  "routing",
  magento_profiler_timer_routing_start, 2,
  magento_profiler_timer_routing_end, 1,
  {0}
},
{
  "loading",
  magento_profiler_timer_loading_start, 1,
  magento_profiler_timer_loading_end, 8,
  {0}
},
{
  "before_layout_loading",
  magento_profiler_timer_before_layout_loading_start, 1,
  magento_profiler_timer_before_layout_loading_end, 1,
  {0}
},
{
  "layout_loading",
  magento_profiler_timer_layout_loading_start, 1,
  magento_profiler_timer_layout_loading_end, 1,
  {0}
},
{
  "between_layout_loading_and_rendering",
  magento_profiler_timer_between_layout_loading_and_rendering_start, 1,
  magento_profiler_timer_between_layout_loading_and_rendering_end, 1,
  {0}
},
{
  "layout_rendering",
  magento_profiler_timer_layout_rendering_start, 1,
  magento_profiler_timer_layout_rendering_end, 1,
  {0}
},
{
  "after_layout_rendering",
  magento_profiler_timer_after_layout_rendering_start, 1,
  magento_profiler_timer_after_layout_rendering_end, 1,
  {0}
},
{
  "controller",
  magento_profiler_timer_controller_start, 1,
  magento_profiler_timer_controller_end, 1,
  {0}
},
{
  "before_sending_response",
  magento_profiler_timer_before_sending_response_start, 1,
  magento_profiler_timer_before_sending_response_end, 1,
  {0}
},
{
  "sending_response",
  magento_profiler_timer_sending_response_start, 1,
  magento_profiler_timer_sending_response_end, 1,
  {0}
},
{
  "total",
  magento_profiler_timer_total_start, 1,
  magento_profiler_timer_total_end, 1,
  {0}
},
{NULL, NULL, 0, NULL, 0, {0}}
};

profiled_function_t *magento_match_function(const char* function_name, zend_execute_data* data TSRMLS_DC) {
 ++hp_globals.internal_match_counters.total;
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
            return &magento_profiled_functions[21];
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
      if (function_name[5] == 'R') {
       if (function_name[6] == 'e') {
        if (function_name[7] == 'q') {
         if (function_name[8] == 'u') {
          if (function_name[9] == 'e') {
           if (function_name[10] == 's') {
            if (function_name[11] == 't') {
             if (function_name[12] == '\0') {
              ++hp_globals.internal_match_counters.function;
              const char *class_name = hp_get_class_name(data TSRMLS_CC);
              if (!class_name) return NULL;
              if (!strcmp(class_name, "Mage_Core_Model_App"))
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
                  return &magento_profiled_functions[7];
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
         if (function_name[8] == 'D') {
          if (function_name[9] == 'a') {
           if (function_name[10] == 't') {
            if (function_name[11] == 'a') {
             if (function_name[12] == 'U') {
              if (function_name[13] == 'p') {
               if (function_name[14] == 'd') {
                if (function_name[15] == 'a') {
                 if (function_name[16] == 't') {
                  if (function_name[17] == 'e') {
                   if (function_name[18] == 's') {
                    if (function_name[19] == '\0') {
                     ++hp_globals.internal_match_counters.function;
                     const char *class_name = hp_get_class_name(data TSRMLS_CC);
                     if (!class_name) return NULL;
                     if (!strcmp(class_name, "Mage_Core_Model_Resource_Setup"))
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
        return NULL;
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
          return &magento_profiled_functions[24];
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
        return &magento_profiled_functions[19];
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
          return NULL;
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
             return &magento_profiled_functions[22];
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
               return &magento_profiled_functions[15];
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
  return NULL;
 }
 if (function_name[0] == 'p') {
  if (function_name[1] == 'r') {
   if (function_name[2] == 'o') {
    if (function_name[3] == 'c') {
     if (function_name[4] == 'e') {
      if (function_name[5] == 's') {
       if (function_name[6] == 's') {
        if (function_name[7] == 'R') {
         if (function_name[8] == 'e') {
          if (function_name[9] == 'q') {
           if (function_name[10] == 'u') {
            if (function_name[11] == 'e') {
             if (function_name[12] == 's') {
              if (function_name[13] == 't') {
               if (function_name[14] == '\0') {
                ++hp_globals.internal_match_counters.function;
                const char *class_name = hp_get_class_name(data TSRMLS_CC);
                if (!class_name) return NULL;
                if (!strcmp(class_name, "Mage_Core_Model_Cache"))
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
          return NULL;
         }
         return NULL;
        }
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
               return &magento_profiled_functions[23];
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
            if (function_name[11] == '\0') {
             ++hp_globals.internal_match_counters.function;
             const char *class_name = hp_get_class_name(data TSRMLS_CC);
             if (!class_name) return NULL;
             if (!strcmp(class_name, "Mage_Core_Model_Config"))
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
     if (function_name[4] == 'D') {
      if (function_name[5] == 'b') {
       if (function_name[6] == '\0') {
        ++hp_globals.internal_match_counters.function;
        const char *class_name = hp_get_class_name(data TSRMLS_CC);
        if (!class_name) return NULL;
        if (!strcmp(class_name, "Mage_Core_Model_Config"))
         return &magento_profiled_functions[8];
        ++hp_globals.internal_match_counters.class_unmatched;
        return NULL;
       }
       return NULL;
      }
      return NULL;
     }
     if (function_name[4] == 'A') {
      if (function_name[5] == 'r') {
       if (function_name[6] == 'e') {
        if (function_name[7] == 'a') {
         if (function_name[8] == 'P') {
          if (function_name[9] == 'a') {
           if (function_name[10] == 'r') {
            if (function_name[11] == 't') {
             if (function_name[12] == '\0') {
              ++hp_globals.internal_match_counters.function;
              const char *class_name = hp_get_class_name(data TSRMLS_CC);
              if (!class_name) return NULL;
              if (!strcmp(class_name, "Mage_Core_Model_App"))
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
            return NULL;
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

static profiled_application_t magento_profiled_application = {
"magento",
magento_profiled_functions,
25,
magento_profiler_timers,
22,
&magento_profiled_functions[0],
&magento_profiled_functions[1],
{NULL, NULL},
NULL,
magento_init_context,
magento_cleanup_context,
magento_match_function,
magento_send_metrics
};

profiled_application_t *qm_match_first_app_function(const char* function_name,
zend_execute_data* data TSRMLS_DC) {
 ++hp_globals.internal_match_counters.total;
 if (function_name[0] == 'b') {
  if (function_name[1] == 'o') {
   if (function_name[2] == 'o') {
    if (function_name[3] == 't') {
     if (function_name[4] == '\0') {
      ++hp_globals.internal_match_counters.function;
      const char *class_name = hp_get_class_name(data TSRMLS_CC);
      if (!class_name) return NULL;
      if (!strcmp(class_name, "Oro\\Bundle\\DistributionBundle\\OroKernel"))
       return &orocommerce_profiled_application;
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

