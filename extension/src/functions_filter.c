#include "quanta_mon.h"

/**
 * Parse the list of monitored functions from the zval argument.
 *
 * @author ch
 */
void hp_get_monitored_functions_fill() {
  /* Already initialized ? */
  if (hp_globals.monitored_function_names[0] != NULL) {
    return;
  }
  hp_globals.monitored_function_names[0]  = "Mage::run";
  hp_globals.monitored_function_names[1]  = "Mage_Core_Model_App::_initBaseConfig";
  hp_globals.monitored_function_names[2]  = "Mage_Core_Model_App::_initCache";
  hp_globals.monitored_function_names[3]  = "Mage_Core_Model_Config::loadModulesCache";
  hp_globals.monitored_function_names[4]  = "Mage_Core_Model_Config::loadModules";
  hp_globals.monitored_function_names[5]  = "Mage_Core_Model_Resource_Setup::applyAllUpdates";
  hp_globals.monitored_function_names[6]  = "Mage_Core_Model_Config::loadDb";
  hp_globals.monitored_function_names[7]  = "Mage_Core_Model_App::_initStores";

  hp_globals.monitored_function_names[8]  = "Mage_Core_Controller_Varien_Action::preDispatch";
  hp_globals.monitored_function_names[9] = "Mage_Core_Controller_Varien_Action::loadLayoutUpdates";
  hp_globals.monitored_function_names[10] = "Mage_Core_Controller_Varien_Action::renderLayout";
  hp_globals.monitored_function_names[11] = "Mage_Core_Controller_Varien_Action::postDispatch";
  hp_globals.monitored_function_names[12] = "Mage_Core_Controller_Response_Http::sendResponse";

  hp_globals.monitored_function_names[13] = "";
  hp_globals.monitored_function_names[14] = "";
  hp_globals.monitored_function_names[15] = "Mage_Core_Block_Abstract::toHtml";

  hp_globals.monitored_function_names[16] = "PDOStatement::execute";

  /* POS_ENTRY_EVENTS_ONLY */
  hp_globals.monitored_function_names[17] = "Mage_Core_Model_Cache::flush";
  hp_globals.monitored_function_names[18] = "Mage_Core_Model_Cache::cleanType";
  hp_globals.monitored_function_names[19] = "Mage_Adminhtml_CacheController::flushSystemAction";
  hp_globals.monitored_function_names[20] = "Mage_Index_Model_Event::_beforeSave";
  hp_globals.monitored_function_names[21] = ""; // PHP total time
  hp_globals.monitored_function_names[22] = NULL;
  // Don't forget to change QUANTA_MON_MAX_MONITORED_FUNCTIONS. It must be equal to the last entry ([x] = NULL) + 1
}

/**
 * Clear filter for functions which may be ignored during profiling.
 *
 * @author ch
 */
void hp_monitored_functions_filter_clear() {
  memset(hp_globals.monitored_function_filter, 0,
         QUANTA_MON_MONITORED_FUNCTION_FILTER_SIZE);
}

/**
 * Initialize filter for monitored functions using bit vector.
 *
 * @author ch
 */
void hp_monitored_functions_filter_init() {
    int i = 0;
    for(; hp_globals.monitored_function_names[i] != NULL; i++) {
      if (!*hp_globals.monitored_function_names[i])
        continue;
      char *str  = strstr(hp_globals.monitored_function_names[i], "::");
      uint8_t hash = hp_inline_hash(str ? str + 2 : hp_globals.monitored_function_names[i]);
      int   idx  = INDEX_2_BYTE(hash);
      hp_globals.monitored_function_filter[idx] |= INDEX_2_BIT(hash);
    }
    hp_globals.magento_blocks_first = NULL;
}

/**
 * Check if function collides in filter of functions to be monitored
 *
 * @author ch
 */
int hp_monitored_functions_filter_collision(uint8_t hash) {
  uint8_t mask = INDEX_2_BIT(hash);
  return hp_globals.monitored_function_filter[INDEX_2_BYTE(hash)] & mask;
}

/**
 * Parse the list of ignored functions from the zval argument.
 *
 * @author mpal
 */
void hp_get_ignored_functions_from_arg(zval *args) {

  if (hp_globals.ignored_function_names) {
    hp_array_del(hp_globals.ignored_function_names);
  }

  if (args != NULL) {
    zval  *zresult = NULL;

    zresult = hp_zval_at_key("ignored_functions", args);
    hp_globals.ignored_function_names = hp_strings_in_zval(zresult);
  } else {
    hp_globals.ignored_function_names = NULL;
  }
}

/**
 * Clear filter for functions which may be ignored during profiling.
 *
 * @author mpal
 */
void hp_ignored_functions_filter_clear() {
  memset(hp_globals.ignored_function_filter, 0,
         QUANTA_MON_IGNORED_FUNCTION_FILTER_SIZE);
}

/**
 * Initialize filter for ignored functions using bit vector.
 *
 * @author mpal
 */
void hp_ignored_functions_filter_init() {
  if (hp_globals.ignored_function_names != NULL) {
    int i = 0;
    for(; hp_globals.ignored_function_names[i] != NULL; i++) {
      char *str  = hp_globals.ignored_function_names[i];
      uint8_t hash = hp_inline_hash(str);
      int   idx  = INDEX_2_BYTE(hash);
      hp_globals.ignored_function_filter[idx] |= INDEX_2_BIT(hash);
    }
  }
}

/**
 * Check if function collides in filter of functions to be ignored.
 *
 * @author mpal
 */
int hp_ignored_functions_filter_collision(uint8_t hash) {
  uint8_t mask = INDEX_2_BIT(hash);
  return hp_globals.ignored_function_filter[INDEX_2_BYTE(hash)] & mask;
}

/**
 * Check if this entry should be ignored, first with a conservative Bloomish
 * filter then with an exact check against the function names.
 *
 * @author mpal
 */
int hp_ignore_entry_work(uint8_t hash_code, char *curr_func) {
  int ignore = 0;
  if (hp_ignored_functions_filter_collision(hash_code)) {
    int i = 0;
    for (; hp_globals.ignored_function_names[i] != NULL; i++) {
      char *name = hp_globals.ignored_function_names[i];
      if ( !strcmp(curr_func, name)) {
        ignore++;
        break;
      }
    }
  }
  return ignore;
}

inline int hp_ignore_entry(uint8_t hash_code, char *curr_func) {
  /* First check if ignoring functions is enabled */
  return hp_globals.ignored_function_names != NULL &&
         hp_ignore_entry_work(hash_code, curr_func);
}
