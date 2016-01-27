#!/bin/sh
rsync -ar /php-module/extension/ /extension/
phpize
./configure
