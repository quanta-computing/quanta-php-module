#!/bin/sh

package=$1
basedir=/php-module
packagerdir=/php-module/packager/debian7
builddir=${packagerdir}/${package}/build
outdir=${packagerdir}/${package}/pkg

if [ -z "$package" ]; then
  echo "Usage: build_package.sh <package>"
  exit 1
fi

echo "Creating build directory..."
rm -Rf ${builddir}
mkdir -p /packager
cp -r ${basedir}/extension ${builddir}
cp -r ${packagerdir}/${package}/debian ${builddir}
cd ${builddir}


echo "Building package..."
debuild -us -uc

echo "Delivering packages"
rm -Rf ${outdir}
mkdir -p ${outdir}
cp -v /packager/php* ${outdir}

echo "Testing package..."
dpkg -i ${outdir}/*.deb
php5enmod quanta_mon || echo "No php5enmod"
php -i | grep quanta
