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
  hp_globals.monitored_function_names[1][4] = "Magento\\Framework\\App\\Request\\Http::getFrontName";
  hp_globals.monitored_function_names[1][5] = "";
  hp_globals.monitored_function_names[1][6] = "";
  hp_globals.monitored_function_names[1][7] = "Magento\\Framework\\App\\FrontController\\Interceptor::dispatch";

  hp_globals.monitored_function_names[1][8] = "Magento\\Framework\\App\\Action\\Action::dispatch";
  // hp_globals.monitored_function_names[1][9] = "Magento\\Framework\\View\\Page\\Builder::loadLayoutUpdates"; TODO! Never called ?
  hp_globals.monitored_function_names[1][9] = "Magento\\Framework\\View\\Page\\Builder::generateLayoutBlocks";
  hp_globals.monitored_function_names[1][10] = "Magento\\Framework\\View\\Result\\Page\\Interceptor::renderResult";
  hp_globals.monitored_function_names[1][11] = "";
  // hp_globals.monitored_function_names[1][11] = "Magento\\Framework\\View\\Page\\Builder::generateLayoutXml"; //TODO! Check what it dows
  hp_globals.monitored_function_names[1][12] = "Magento\\Framework\\App\\Response\\Http\\Interceptor::sendResponse";

  hp_globals.monitored_function_names[1][13] = "";
  hp_globals.monitored_function_names[1][14] = "";
  hp_globals.monitored_function_names[1][15] = "Magento\\Framework\\View\\Layout::_renderBlock";

  hp_globals.monitored_function_names[1][16] = "PDOStatement::execute";

  /* POS_ENTRY_EVENTS_ONLY */
  hp_globals.monitored_function_names[1][17] = "Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushAll::execute";
  hp_globals.monitored_function_names[1][18] = "Magento\\Framework\\App\\Cache\\TypeList::cleanType";
  hp_globals.monitored_function_names[1][19] = "Magento\\Backend\\Controller\\Adminhtml\\Cache\\FlushSystem::execute";
  hp_globals.monitored_function_names[1][20] = "";
  hp_globals.monitored_function_names[1][21] = "";
  hp_globals.monitored_function_names[1][22] = NULL;

  // Don't forget to change QUANTA_MON_MAX_MONITORED_FUNCTIONS. It must be equal to the last entry ([x] = NULL) + 1
}

char **hp_globals_monitored_function_names(void) {
  return hp_globals.monitored_function_names[QUANTA_MAGENTO_VERSION_INDEX];
}

/**
 * Clear filter for functions which should be monitored during profiling.
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
