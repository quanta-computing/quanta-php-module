# quanta-php-module
The new quanta PHP module

% cd <quanta_mon_source_directory>/extension/
% phpize
% ./configure --with-php-config=<path to php-config>
% make
% make install
php.ini file: You can update your php.ini file to automatically load your extension. Add the following to your php.ini file.

[quanta_mon]
extension=quanta_mon.so
;
; directory used by default implementation of the iQuantaMonRuns
; interface (namely, the QuantaMonRuns_Default class) for storing
; QuantaMon runs.
;
quanta_mon.output_dir=<directory_for_storing_quanta_monitoring_runs>

-- Output --
/tmp/quanta-TSSEC.TSUSEC-PID.txt
TSSEC = number of seconds from 1 Jan 1970
TSUSEC = microseconds part of the timestamp
PID = PID of apache

quanta1: flag=0 cpufreq=1999.927430 name0=Mage::run beg0=403239367634239 stp0=403240361902987 name1=Mage_Core_Controller_Varien_Action::preDispatch beg1=403239746633329 stp1=403240361915863 name2=Mage_Core_Controller_Varien_Action::loadLayoutUpdates beg2=403239856295515 stp2=403239861128599 name3=Mage_Core_Controller_Varien_Action::renderLayout beg3=403240113622273 stp3=403240296907129 name4=Mage_Core_Controller_Varien_Action::postDispatch beg4=403240296957859 stp4=403240354583875 name5=Mage_Core_Controller_Response_Http::sendResponse beg5=403240356690919 stp5=403240361246659

flag = debug, should be 0
cpufreq = cpu frequency of the current core, in Mhz
nameX = name of the traced function
begX = Time Stamp Counter (64 bit value) recorded at the beginning of the call
stpX = Time Stamp Counter recorded at the return of the call

-- Convert TSC to uSec --
Substract the two trace values (for Mage::run total runtime : stp0-beg0) and divide by cpufreq.
 (403240361902987 - 403239367634239) / 1999.927430 = 497152.4 us

