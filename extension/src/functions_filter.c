#include "quanta_mon.h"

/**
 * Parse the list of monitored functions from the zval argument.
 *
 * @author ch
 */
void hp_get_monitored_functions_fill() {
  /* Already initialized ? */
  if (hp_globals.monitored_function_names[0][0] != NULL) {
    return;
  }
  hp_globals.monitored_function_names[0][0] = "Mage::run";
  hp_globals.monitored_function_names[0][1] = "Mage_Core_Model_App::_initBaseConfig";
  hp_globals.monitored_function_names[0][2] = "Mage_Core_Model_App::_initCache";
  hp_globals.monitored_function_names[0][3] = "Mage_Core_Model_Config::loadModulesCache";
  hp_globals.monitored_function_names[0][4] = "Mage_Core_Model_Config::loadModules";
  hp_globals.monitored_function_names[0][5] = "Mage_Core_Model_Resource_Setup::applyAllUpdates";
  hp_globals.monitored_function_names[0][6] = "Mage_Core_Model_Config::loadDb";
  hp_globals.monitored_function_names[0][7] = "Mage_Core_Model_App::_initStores";

  hp_globals.monitored_function_names[0][8] = "Mage_Core_Controller_Varien_Action::preDispatch";
  hp_globals.monitored_function_names[0][9] = "Mage_Core_Controller_Varien_Action::loadLayoutUpdates";
  hp_globals.monitored_function_names[0][10] = "Mage_Core_Controller_Varien_Action::renderLayout";
  hp_globals.monitored_function_names[0][11] = "Mage_Core_Controller_Varien_Action::postDispatch";
  hp_globals.monitored_function_names[0][12] = "Mage_Core_Controller_Response_Http::sendResponse";

  hp_globals.monitored_function_names[0][13] = "";
  hp_globals.monitored_function_names[0][14] = "";
  hp_globals.monitored_function_names[0][15] = "Mage_Core_Block_Abstract::toHtml";

  hp_globals.monitored_function_names[0][16] = "PDOStatement::execute";

  /* POS_ENTRY_EVENTS_ONLY */
  hp_globals.monitored_function_names[0][17] = "Mage_Core_Model_Cache::flush";
  hp_globals.monitored_function_names[0][18] = "Mage_Core_Model_Cache::cleanType";
  hp_globals.monitored_function_names[0][19] = "Mage_Adminhtml_CacheController::flushSystemAction";
  hp_globals.monitored_function_names[0][20] = "Mage_Index_Model_Event::_beforeSave";
  hp_globals.monitored_function_names[0][21] = ""; // PHP total time
  hp_globals.monitored_function_names[0][22] = NULL;



  // MAGENTO 2
  // Allowed metrics for debug: 2,3,5,6,7
  hp_globals.monitored_function_names[1][0] = "Magento\\Framework\\App\\Bootstrap::run";
  //TODO! Maybe split: load scopes, load plugins ?
  hp_globals.monitored_function_names[1][1] = "Magento\\Framework\\App\\Bootstrap::create";
  hp_globals.monitored_function_names[1][2] = "Magento\\Framework\\App\\Bootstrap::createApplication";
  hp_globals.monitored_function_names[1][3] = "Magento\\Framework\\Interception\\Config\\Config::initialize";
  hp_globals.monitored_function_names[1][4] = "";
  hp_globals.monitored_function_names[1][5] = "Magento\\Framework\\App\\Response\\Http::sendResponse";
  hp_globals.monitored_function_names[1][6] = "";
  hp_globals.monitored_function_names[1][7] = "Magento\\Framework\\App\\FrontController::dispatch";

  hp_globals.monitored_function_names[1][8] = "Magento\\Framework\\App\\Action\\Action::dispatch";
  hp_globals.monitored_function_names[1][9] = "Magento\\Framework\\View\\Page\\Builder::loadLayoutUpdates"; // TODO! Never called ?
  hp_globals.monitored_function_names[1][10] = "Magento\\Framework\\View\\Page\\Builder::generateLayoutBlocks";
  hp_globals.monitored_function_names[1][11] = "Magento\\Framework\\View\\Page\\Builder::generateLayoutXml"; //TODO! Check what it dows
  hp_globals.monitored_function_names[1][12] = "";

  hp_globals.monitored_function_names[1][13] = "";
  hp_globals.monitored_function_names[1][14] = "";
  hp_globals.monitored_function_names[1][15] = "Magento\\Framework\\View\\Element\\AbstractBlock::toHtml";

  hp_globals.monitored_function_names[1][16] = "PDOStatement::execute";

  /* POS_ENTRY_EVENTS_ONLY */
  hp_globals.monitored_function_names[1][17] = "";
  hp_globals.monitored_function_names[1][18] = "";
  hp_globals.monitored_function_names[1][19] = "";
  hp_globals.monitored_function_names[1][20] = "";
  hp_globals.monitored_function_names[1][21] = "";
  hp_globals.monitored_function_names[1][22] = NULL;

  // Don't forget to change QUANTA_MON_MAX_MONITORED_FUNCTIONS. It must be equal to the last entry ([x] = NULL) + 1
}

char **hp_globals_monitored_function_names(void) {
  return hp_globals.monitored_function_names[QUANTA_MAGENTO_VERSION_INDEX];
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
    for(; hp_globals_monitored_function_names()[i] != NULL; i++) {
      if (!*hp_globals_monitored_function_names()[i])
        continue;
      char *str  = strstr(hp_globals_monitored_function_names()[i], "::");
      uint8_t hash = hp_inline_hash(str ? str + 2 : hp_globals_monitored_function_names()[i]);
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
