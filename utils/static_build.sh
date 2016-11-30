#!/bin/bash
set -e

version=$(git describe --always --match '[[:digit:]]*')

srcdir=$PWD
wkdir=$(mktemp -d)

#trap "rm -rf $wkdir" EXIT

set -x

git archive -o $wkdir/axe_${version}.tar HEAD

cd $wkdir

docker run                                           \
    -v $wkdir:/src                                   \
    -v $srcdir/utils/hbb_script.sh:/hbb_script.sh:ro \
    -e AXE_VERSION=${version}                        \
    kdmurray91/kdm-hbb-64                            \
    bash /hbb_script.sh


tar cvzf $srcdir/axe_${version}_amd64.tar.gz  axe_${version}_amd64
