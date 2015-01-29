#!/bin/bash

set -e
set -x

test -d .git

PKG_DIR=axe-deb
SRC_DIRS="src/ docs/ tests/ CMakeLists.txt debian/ version"

# grab version from changelog
VERSION="$(head -n 1 debian/changelog  |perl -pe 's/.*\((.+)-.*/$1/')"

echo $VERSION >version
rm -rf $PKG_DIR/* axe_*.orig.tar.gz
mkdir -p $PKG_DIR
cp -r $SRC_DIRS $PKG_DIR


tar czf axe_${VERSION}.orig.tar.gz $SRC_DIRS
