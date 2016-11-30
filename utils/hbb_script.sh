#!/bin/bash
set -e
source /hbb_exe/activate
set -x

prefix=/io/axe_${AXE_VERSION}_amd64
rm -rf $prefix
mkdir -p $prefix
trap "chown $HBBUID:$HBBUID -R $prefix" EXIT

# Clone to builddir
builddir=$(mktemp -d)
cd $builddir
tar xvf /io/axe_${AXE_VERSION}.tar --strip-components=1

rm -rf build
mkdir build
cd build

cmake ..                         \
    -DAXE_VERSION=${AXE_VERSION} \
    -DCMAKE_INSTALL_PREFIX=$prefix
make -j4 VERBOSE=1
make test
make install
