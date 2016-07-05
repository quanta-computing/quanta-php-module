#include "quanta_mon.h"

void hp_fill_monitored_functions(char **function_names) {
 if (function_names[0] != NULL) return;
 function_names[0] = "Magento\\Framework\\App\\Bootstrap::run";
 function_names[1] = "Magento\\Framework\\App\\Bootstrap::create";
 function_names[2] = "Magento\\Framework\\App\\Bootstrap::createApplication";
 function_names[3] = "Magento\\Framework\\Interception\\Config\\Config::initialize";
 function_names[4] = "Magento\\Framework\\App\\Request\\Http::getFrontName";
 function_names[5] = "";
 function_names[6] = "";
 function_names[7] = "Magento\\Framework\\App\\FrontController\\Interceptor::dispatch";
 function_names[8] = "Magento\\Framework\\App\\Action\\Action::dispatch";
 function_names[9] = "Magento\\Framework\\View\\Page\\Builder::generateLayoutBlocks";
 function_names[10] = "Magento\\Framework\\View\\Result\\Page\\Interceptor::renderResult";
 function_names[11] = "";
 function_names[12] = "Magento\\Framework\\App\\Response\\Http\\Interceptor::sendResponse";
 function_names[13] = "";
 function_names[14] = "";
 function_names[15] = "Magento\\Framework\\View\\Layout::_renderBlock";
 function_names[16] = "PDOStatement::execute";
 function_names[17] = "Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushAll::execute";
 function_names[18] = "Magento\\Framework\\App\\Cache\\TypeList::cleanType";
 function_names[19] = "Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushSystem::execute";
 function_names[20] = "Magento\\Indexer\\Model\\Indexer::reindexAll";
 function_names[21] = "";
 function_names[22] = NULL;
}

int hp_match_monitored_function(const char* function_name, zend_execute_data* data TSRMLS_DC) {
 ++hp_globals.internal_match_counters.total;
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
              if (!class_name) return -1;
              if (!strcmp(class_name, "Magento\\Framework\\App\\Request\\Http")) return 4;
              ++hp_globals.internal_match_counters.class_unmatched;
              return -1;
             }
             return -1;
            }
            return -1;
           }
           return -1;
          }
          return -1;
         }
         return -1;
        }
        return -1;
       }
       return -1;
      }
      return -1;
     }
     return -1;
    }
    return -1;
   }
   if (function_name[2] == 'n') {
    if (function_name[3] == 'e') {
     if (function_name[4] == 'r') {
      if (function_name[5] == 'a') {
       if (function_name[6] == 't') {
        if (function_name[7] == 'e') {
         if (function_name[8] == 'L') {
          if (function_name[9] == 'a') {
           if (function_name[10] == 'y') {
            if (function_name[11] == 'o') {
             if (function_name[12] == 'u') {
              if (function_name[13] == 't') {
               if (function_name[14] == 'B') {
                if (function_name[15] == 'l') {
                 if (function_name[16] == 'o') {
                  if (function_name[17] == 'c') {
                   if (function_name[18] == 'k') {
                    if (function_name[19] == 's') {
                     if (function_name[20] == '\0') {
                      ++hp_globals.internal_match_counters.function;
                      const char *class_name = hp_get_class_name(data TSRMLS_CC);
                      if (!class_name) return -1;
                      if (!strcmp(class_name, "Magento\\Framework\\View\\Page\\Builder")) return 9;
                      ++hp_globals.internal_match_counters.class_unmatched;
                      return -1;
                     }
                     return -1;
                    }
                    return -1;
                   }
                   return -1;
                  }
                  return -1;
                 }
                 return -1;
                }
                return -1;
               }
               return -1;
              }
              return -1;
             }
             return -1;
            }
            return -1;
           }
           return -1;
          }
          return -1;
         }
         return -1;
        }
        return -1;
       }
       return -1;
      }
      return -1;
     }
     return -1;
    }
    return -1;
   }
   return -1;
  }
  return -1;
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
              if (!class_name) return -1;
              if (!strcmp(class_name, "Magento\\Framework\\App\\Response\\Http\\Interceptor")) return 12;
              ++hp_globals.internal_match_counters.class_unmatched;
              return -1;
             }
             return -1;
            }
            return -1;
           }
           return -1;
          }
          return -1;
         }
         return -1;
        }
        return -1;
       }
       return -1;
      }
      return -1;
     }
     return -1;
    }
    return -1;
   }
   return -1;
  }
  return -1;
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
              if (!class_name) return -1;
              if (!strcmp(class_name, "Magento\\Framework\\View\\Layout")) return 15;
              ++hp_globals.internal_match_counters.class_unmatched;
              return -1;
             }
             return -1;
            }
            return -1;
           }
           return -1;
          }
          return -1;
         }
         return -1;
        }
        return -1;
       }
       return -1;
      }
      return -1;
     }
     return -1;
    }
    return -1;
   }
   return -1;
  }
  return -1;
 }
 if (function_name[0] == 'i') {
  if (function_name[1] == 'n') {
   if (function_name[2] == 'i') {
    if (function_name[3] == 't') {
     if (function_name[4] == 'i') {
      if (function_name[5] == 'a') {
       if (function_name[6] == 'l') {
        if (function_name[7] == 'i') {
         if (function_name[8] == 'z') {
          if (function_name[9] == 'e') {
           if (function_name[10] == '\0') {
            ++hp_globals.internal_match_counters.function;
            const char *class_name = hp_get_class_name(data TSRMLS_CC);
            if (!class_name) return -1;
            if (!strcmp(class_name, "Magento\\Framework\\Interception\\Config\\Config")) return 3;
            ++hp_globals.internal_match_counters.class_unmatched;
            return -1;
           }
           return -1;
          }
          return -1;
         }
         return -1;
        }
        return -1;
       }
       return -1;
      }
      return -1;
     }
     return -1;
    }
    return -1;
   }
   return -1;
  }
  return -1;
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
        if (!class_name) return -1;
        if (!strcmp(class_name, "Magento\\Framework\\App\\Bootstrap")) return 1;
        ++hp_globals.internal_match_counters.class_unmatched;
        return -1;
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
                   if (!class_name) return -1;
                   if (!strcmp(class_name, "Magento\\Framework\\App\\Bootstrap")) return 2;
                   ++hp_globals.internal_match_counters.class_unmatched;
                   return -1;
                  }
                  return -1;
                 }
                 return -1;
                }
                return -1;
               }
               return -1;
              }
              return -1;
             }
             return -1;
            }
            return -1;
           }
           return -1;
          }
          return -1;
         }
         return -1;
        }
        return -1;
       }
       return -1;
      }
      return -1;
     }
     return -1;
    }
    return -1;
   }
   return -1;
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
           if (!class_name) return -1;
           if (!strcmp(class_name, "Magento\\Framework\\App\\Cache\\TypeList")) return 18;
           ++hp_globals.internal_match_counters.class_unmatched;
           return -1;
          }
          return -1;
         }
         return -1;
        }
        return -1;
       }
       return -1;
      }
      return -1;
     }
     return -1;
    }
    return -1;
   }
   return -1;
  }
  return -1;
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
         if (!class_name) return -1;
         if (!strcmp(class_name, "PDOStatement")) return 16;
         if (!strcmp(class_name, "Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushAll")) return 17;
         if (!strcmp(class_name, "Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushSystem")) return 19;
         ++hp_globals.internal_match_counters.class_unmatched;
         return -1;
        }
        return -1;
       }
       return -1;
      }
      return -1;
     }
     return -1;
    }
    return -1;
   }
   return -1;
  }
  return -1;
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
          if (!class_name) return -1;
          if (!strcmp(class_name, "Magento\\Framework\\App\\FrontController\\Interceptor")) return 7;
          if (!strcmp(class_name, "Magento\\Framework\\App\\Action\\Action")) return 8;
          ++hp_globals.internal_match_counters.class_unmatched;
          return -1;
         }
         return -1;
        }
        return -1;
       }
       return -1;
      }
      return -1;
     }
     return -1;
    }
    return -1;
   }
   return -1;
  }
  return -1;
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
              if (!class_name) return -1;
              if (!strcmp(class_name, "Magento\\Framework\\View\\Result\\Page\\Interceptor")) return 10;
              ++hp_globals.internal_match_counters.class_unmatched;
              return -1;
             }
             return -1;
            }
            return -1;
           }
           return -1;
          }
          return -1;
         }
         return -1;
        }
        return -1;
       }
       return -1;
      }
      return -1;
     }
     return -1;
    }
    return -1;
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
            if (!class_name) return -1;
            if (!strcmp(class_name, "Magento\\Indexer\\Model\\Indexer")) return 20;
            ++hp_globals.internal_match_counters.class_unmatched;
            return -1;
           }
           return -1;
          }
          return -1;
         }
         return -1;
        }
        return -1;
       }
       return -1;
      }
      return -1;
     }
     return -1;
    }
    return -1;
   }
   return -1;
  }
  if (function_name[1] == 'u') {
   if (function_name[2] == 'n') {
    if (function_name[3] == '\0') {
     ++hp_globals.internal_match_counters.function;
     const char *class_name = hp_get_class_name(data TSRMLS_CC);
     if (!class_name) return -1;
     if (!strcmp(class_name, "Magento\\Framework\\App\\Bootstrap")) return 0;
     ++hp_globals.internal_match_counters.class_unmatched;
     return -1;
    }
    return -1;
   }
   return -1;
  }
  return -1;
 }
 return -1;
}
