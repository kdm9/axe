#!/bin/bash


version=$1

git tag -v "$version"
if [ $? -ne 0 ]
then
	echo "Invalid tag: $version"
	exit 1
fi

set -xe

rm -f ../axe_${version}.orig.tar*
git archive -o ../axe_${version}.orig.tar $version
tar -rvf ../axe_${version}.orig.tar --owner=0 --group=0 src/libqes/
xz ../axe_${version}.orig.tar
