<?php

class Mage_Core_Model_App {
  public function __construct() {
    echo "MAGENTO INITIALIZE...\n";
  }

  public function run() {
    echo "RUNNING MAGENTO \\o/\n";
    phpinfo();
  }
}

class Mage {
  public static function getVersion() {
    return "42.0";
  }

  public static function getEdition() {
    return "Korrigan special love build";
  }
}

// echo Mage::getVersion() . " " . Mage::getEdition() . "\n";
$app = new Mage_Core_Model_App();
$app->run();

?>
