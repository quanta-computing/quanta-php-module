#!/bin/sh

package=$1
basedir=/php-module
packagerdir=/php-module/packager/debian8
outdir=${packagerdir}/${package}/pkg

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

echo "Delivering packages"
rm -Rf ${outdir}
mkdir -p ${outdir}
cp -v ${packagerdir}/${package}/php* ${outdir}

echo "Testing package..."
dpkg -i ${outdir}/*.deb
php5enmod quanta_mon || echo "No php5enmod"
php -i | grep quanta
