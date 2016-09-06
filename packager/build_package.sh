#!/bin/sh

ostype=$1
version=$2
localdir=`git rev-parse --show-toplevel`
dockerfile=${localdir}/packager/${ostype}-${version}.dockerfile

if [ -z "$ostype" ] || [ -z "$version" ]; then
  echo "Usage: ./build_package.sh <OS> <version>"
  exit 1
fi

echo "======>>> Started building package for ${ostype}/${version}..."
if [ ! -f $dockerfile ]; then
  echo "ERROR: Wrong os or version, ${dockerfile} does not exists"
  exit 1
fi

echo "======>>> Building Docker image..."
image=`cat ${dockerfile} | docker build -q - | cut -d ':' -f 2`
ret=$?
if [ ! $ret -eq 0 ] || [ -z "$image" ] ; then
  echo "ERROR: Failed to build docker image, exiting..."
  exit 1
fi

echo "======>>> Running the packaging script"
docker run -i -v ${localdir}:/php-module ${image} \
  /php-module/packager/${ostype}/build_package.sh ${version}-quanta-mon

echo "======>>> Sucessfully built package for ${ostype}/${version}!"
