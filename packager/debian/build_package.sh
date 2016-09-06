#!/bin/sh

package=$1
basedir=/php-module
packagerdir=/php-module/packager/debian

if [ -z "$package" ]; then
  echo "Usage: build_package.sh <package>"
  exit 1
fi

echo "Creating build directory..."
rm -Rf ${packagedir}/${package}/build
cp -r ${basedir}/extension ${packagerdir}/${package}/build
cp -r ${packagerdir}/${package}/debian ${packagerdir}/${package}/build
cd ${packagerdir}/${package}/build

echo "Building package..."
debuild -us -uc
