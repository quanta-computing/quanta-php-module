#!/bin/sh

set -e

package=$1
basedir=/php-module
packagerdir=/php-module/packager/centos7

if [ -z "$package" ]; then
  echo "Usage: build_package.sh <package>"
  exit 1
fi

echo "Creating build directory..."
rm -Rf ${packagerdir}/${package}/build
mkdir ${packagerdir}/${package}/build
cp -r ${basedir}/extension ${packagerdir}/${package}/build/${package}
cd ${packagerdir}/${package}/build
cp ${packagerdir}/${package}/${package}.spec /root/rpmbuild/SPECS/${package}.spec
tar czf ${package}.tgz ${package}
mv ${package}.tgz /root/rpmbuild/SOURCES/${package}.tgz

echo "Cleaning up output dir..."
rm -Rf /root/rpmbuild/RPMS/x86_64/*

echo "Building package..."
rpmbuild -ba /root/rpmbuild/SPECS/${package}.spec
cp /root/rpmbuild/RPMS/x86_64/* ${packagerdir}/${package}/build
