#include "quanta_mon.h"

void hp_fill_monitored_functions(char **function_names) {
 if (function_names[0] != NULL) return;
 function_names[0] = "Magento\\Framework\\App\\Bootstrap::run"
 function_names[1] = "Magento\\Framework\\App\\Bootstrap::create"
 function_names[2] = "Magento\\Framework\\App\\Bootstrap::createApplication"
 function_names[3] = "Magento\\Framework\\Interception\\Config\\Config::initialize"
 function_names[4] = "Magento\\Framework\\App\\Request\\Http::getFrontName"
 function_names[5] = ""
 function_names[6] = ""
 function_names[7] = "Magento\\Framework\\App\\FrontController\\Interceptor::dispatch"
 function_names[8] = "Magento\\Framework\\App\\Action\\Action::dispatch"
 function_names[9] = "Magento\\Framework\\View\\Page\\Builder::generateLayoutBlocks"
 function_names[10] = "Magento\\Framework\\View\\Result\\Page\\Interceptor::renderResult"
 function_names[11] = ""
 function_names[12] = "Magento\\Framework\\App\\Response\\Http\\Interceptor::sendResponse"
 function_names[13] = ""
 function_names[14] = ""
 function_names[15] = "Magento\\Framework\\View\\Layout::_renderBlock"
 function_names[16] = "PDOStatement::execute"
 function_names[17] = "Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushAll::execute"
 function_names[18] = "Magento\\Framework\\App\\Cache\\TypeList::cleanType"
 function_names[19] = "Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushSystem::execute"
 function_names[20] = ""
 function_names[21] = ""
 function_names[22] = NULL;
}

int hp_match_monitored_function(const char* function_name, zend_execute_data* data TSRMLS_DC) {
 if (function_name[1] == 'r') {
  if (function_name[2] == 'u') {
   if (function_name[3] == 'n') {
    const char *class_name = hp_get_class_name(data TSRMLS_CC);
    if (!class_name) return -1;
    if (!strcmp(class_name, "Magento\\Framework\\App\\Bootstrap")) return 0;
    return -1;
   }
  }
  if (function_name[2] == 'e') {
   if (function_name[3] == 'n') {
    if (function_name[4] == 'd') {
     if (function_name[5] == 'e') {
      if (function_name[6] == 'r') {
       if (function_name[7] == 'R') {
        if (function_name[8] == 'e') {
         if (function_name[9] == 's') {
          if (function_name[10] == 'u') {
           if (function_name[11] == 'l') {
            if (function_name[12] == 't') {
             const char *class_name = hp_get_class_name(data TSRMLS_CC);
             if (!class_name) return -1;
             if (!strcmp(class_name, "Magento\\Framework\\View\\Result\\Page\\Interceptor")) return 10;
             return -1;
            }
           }
          }
         }
        }
       }
      }
     }
    }
   }
  }
 }
 if (function_name[1] == 'c') {
  if (function_name[2] == 'r') {
   if (function_name[3] == 'e') {
    if (function_name[4] == 'a') {
     if (function_name[5] == 't') {
      if (function_name[6] == 'e') {
       if (function_name[7] == 'A') {
        if (function_name[8] == 'p') {
         if (function_name[9] == 'p') {
          if (function_name[10] == 'l') {
           if (function_name[11] == 'i') {
            if (function_name[12] == 'c') {
             if (function_name[13] == 'a') {
              if (function_name[14] == 't') {
               if (function_name[15] == 'i') {
                if (function_name[16] == 'o') {
                 if (function_name[17] == 'n') {
                  const char *class_name = hp_get_class_name(data TSRMLS_CC);
                  if (!class_name) return -1;
                  if (!strcmp(class_name, "Magento\\Framework\\App\\Bootstrap")) return 2;
                  return -1;
                 }
                }
               }
              }
             }
            }
           }
          }
         }
        }
       }
      }
     }
    }
   }
  }
  if (function_name[2] == 'l') {
   if (function_name[3] == 'e') {
    if (function_name[4] == 'a') {
     if (function_name[5] == 'n') {
      if (function_name[6] == 'T') {
       if (function_name[7] == 'y') {
        if (function_name[8] == 'p') {
         if (function_name[9] == 'e') {
          const char *class_name = hp_get_class_name(data TSRMLS_CC);
          if (!class_name) return -1;
          if (!strcmp(class_name, "Magento\\Framework\\App\\Cache\\TypeList")) return 18;
          return -1;
         }
        }
       }
      }
     }
    }
   }
  }
 }
 if (function_name[1] == 'i') {
  if (function_name[2] == 'n') {
   if (function_name[3] == 'i') {
    if (function_name[4] == 't') {
     if (function_name[5] == 'i') {
      if (function_name[6] == 'a') {
       if (function_name[7] == 'l') {
        if (function_name[8] == 'i') {
         if (function_name[9] == 'z') {
          if (function_name[10] == 'e') {
           const char *class_name = hp_get_class_name(data TSRMLS_CC);
           if (!class_name) return -1;
           if (!strcmp(class_name, "Magento\\Framework\\Interception\\Config\\Config")) return 3;
           return -1;
          }
         }
        }
       }
      }
     }
    }
   }
  }
 }
 if (function_name[1] == 'g') {
  if (function_name[2] == 'e') {
   if (function_name[3] == 't') {
    if (function_name[4] == 'F') {
     if (function_name[5] == 'r') {
      if (function_name[6] == 'o') {
       if (function_name[7] == 'n') {
        if (function_name[8] == 't') {
         if (function_name[9] == 'N') {
          if (function_name[10] == 'a') {
           if (function_name[11] == 'm') {
            if (function_name[12] == 'e') {
             const char *class_name = hp_get_class_name(data TSRMLS_CC);
             if (!class_name) return -1;
             if (!strcmp(class_name, "Magento\\Framework\\App\\Request\\Http")) return 4;
             return -1;
            }
           }
          }
         }
        }
       }
      }
     }
    }
   }
   if (function_name[3] == 'n') {
    if (function_name[4] == 'e') {
     if (function_name[5] == 'r') {
      if (function_name[6] == 'a') {
       if (function_name[7] == 't') {
        if (function_name[8] == 'e') {
         if (function_name[9] == 'L') {
          if (function_name[10] == 'a') {
           if (function_name[11] == 'y') {
            if (function_name[12] == 'o') {
             if (function_name[13] == 'u') {
              if (function_name[14] == 't') {
               if (function_name[15] == 'B') {
                if (function_name[16] == 'l') {
                 if (function_name[17] == 'o') {
                  if (function_name[18] == 'c') {
                   if (function_name[19] == 'k') {
                    if (function_name[20] == 's') {
                     const char *class_name = hp_get_class_name(data TSRMLS_CC);
                     if (!class_name) return -1;
                     if (!strcmp(class_name, "Magento\\Framework\\View\\Page\\Builder")) return 9;
                     return -1;
                    }
                   }
                  }
                 }
                }
               }
              }
             }
            }
           }
          }
         }
        }
       }
      }
     }
    }
   }
  }
 }
 if (function_name[1] == 'd') {
  if (function_name[2] == 'i') {
   if (function_name[3] == 's') {
    if (function_name[4] == 'p') {
     if (function_name[5] == 'a') {
      if (function_name[6] == 't') {
       if (function_name[7] == 'c') {
        if (function_name[8] == 'h') {
         const char *class_name = hp_get_class_name(data TSRMLS_CC);
         if (!class_name) return -1;
         if (!strcmp(class_name, "Magento\\Framework\\App\\FrontController\\Interceptor")) return 7;
         if (!strcmp(class_name, "Magento\\Framework\\App\\Action\\Action")) return 8;
         return -1;
        }
       }
      }
     }
    }
   }
  }
 }
 if (function_name[1] == 's') {
  if (function_name[2] == 'e') {
   if (function_name[3] == 'n') {
    if (function_name[4] == 'd') {
     if (function_name[5] == 'R') {
      if (function_name[6] == 'e') {
       if (function_name[7] == 's') {
        if (function_name[8] == 'p') {
         if (function_name[9] == 'o') {
          if (function_name[10] == 'n') {
           if (function_name[11] == 's') {
            if (function_name[12] == 'e') {
             const char *class_name = hp_get_class_name(data TSRMLS_CC);
             if (!class_name) return -1;
             if (!strcmp(class_name, "Magento\\Framework\\App\\Response\\Http\\Interceptor")) return 12;
             return -1;
            }
           }
          }
         }
        }
       }
      }
     }
    }
   }
  }
 }
 if (function_name[1] == '_') {
  if (function_name[2] == 'r') {
   if (function_name[3] == 'e') {
    if (function_name[4] == 'n') {
     if (function_name[5] == 'd') {
      if (function_name[6] == 'e') {
       if (function_name[7] == 'r') {
        if (function_name[8] == 'B') {
         if (function_name[9] == 'l') {
          if (function_name[10] == 'o') {
           if (function_name[11] == 'c') {
            if (function_name[12] == 'k') {
             const char *class_name = hp_get_class_name(data TSRMLS_CC);
             if (!class_name) return -1;
             if (!strcmp(class_name, "Magento\\Framework\\View\\Layout")) return 15;
             return -1;
            }
           }
          }
         }
        }
       }
      }
     }
    }
   }
  }
 }
 if (function_name[1] == 'e') {
  if (function_name[2] == 'x') {
   if (function_name[3] == 'e') {
    if (function_name[4] == 'c') {
     if (function_name[5] == 'u') {
      if (function_name[6] == 't') {
       if (function_name[7] == 'e') {
        const char *class_name = hp_get_class_name(data TSRMLS_CC);
        if (!class_name) return -1;
        if (!strcmp(class_name, "PDOStatement")) return 16;
        if (!strcmp(class_name, "Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushAll")) return 17;
        if (!strcmp(class_name, "Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushSystem")) return 19;
        return -1;
       }
      }
     }
    }
   }
  }
 }
 return -1;
}
