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
tar -rf ../axe_${version}.orig.tar --owner=0 --group=0 src/libqes/
tar --delete -vf ../axe_${version}.orig.tar .gitmodules src/libqes/.gitmodules src/libqes/.git
xz ../axe_${version}.orig.tar
