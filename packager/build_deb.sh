#!/bin/sh

localdir=`git rev-parse --show-toplevel`
packagerdir=${localdir}/packager

echo "BUILDING PACKAGES"
for dir in `find ${packagerdir} -type d -name 'debian?'`; do
  for subdir in `find $dir -type d -name 'php*' -depth 1`; do
    package=`echo $subdir | sed -E 's/.*packager\/(debian.)\/(php.*)-quanta-mon/\1;\2/'`
    os=`echo $package | cut -d ';' -f 1`
    ver=`echo $package | cut -d ';' -f 2`
    ${packagerdir}/build_package.sh $os $ver
  done
done


mkdir -p ${packagerdir}/pkg_debian

echo "COLLECT .so"
for file in `find ${packagerdir}/debian? -type f -wholename '*/usr/lib/php*/*/quanta_mon.so'`; do
  echo $file
  dst=`echo $file | sed -E 's/.*packager\/(debian.)\/.*\/([0-9]+)\/quanta_mon.so$/\1-\2.so/'`
  cp -v $file ${packagerdir}/pkg_debian/$dst
done
