# quanta-php-module
The new quanta PHP module

## Compile
```
% cd <quanta_mon_source_directory>/extension/
% phpize
% ./configure
% make
% make install

php.ini file: You can update your php.ini file to automatically load your extension. Add the following to your php.ini file.
or create a file like /etc/php5/mods-available/quanta_mon.ini
and create a link : cd /etc/php5/apache2/conf.d && ln -s ../../mods-available/quanta_mon.ini 20-quanta_mon.ini
``` 

``` 
[quanta_mon]
extension=quanta_mon.so
;
; directory used by default implementation of the iQuantaMonRuns
; interface (namely, the QuantaMonRuns_Default class) for storing
; QuantaMon runs.
;
quanta_mon.output_dir=<directory_for_storing_quanta_monitoring_runs>
```

## Output
* renderize: time took between beforeToHtml and afterToHtml, may also return
  * -1.0 if the block is not renderized at all (neither 'before'/'after' was called)
  * -2.0 if the block was found in cache ('after' was called but not 'before')
  * -3.0 is returned when 'before' is called but not 'after'


