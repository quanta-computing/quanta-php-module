#!/bin/sh

package=$1
basedir=/php-module
packagerdir=/php-module/packager/debian7
builddir=/tmp/debian-build
outdir=${packagedir}/${package}/pkg

if [ -z "$package" ]; then
  echo "Usage: build_package.sh <package>"
  exit 1
fi

echo "Creating build directory..."
rm -Rf ${builddir}
cp -r ${basedir}/extension ${builddir}
cp -r ${packagerdir}/${package}/debian ${builddir}
cd ${packagerdir}/${package}/build


echo "Building package..."
debuild -us -uc

echo "Delivering packages"
mkdir -p ${outdir}
cp ${builddir}/*.* ${outdir}
