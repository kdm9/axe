AXE
===

> De-multiplex NGS reads using trie data structures. It's fast, and made of tries!

[![DOI](https://zenodo.org/badge/6357/kdmurray91/axe.png)](https://zenodo.org/record/12278)
[![Documentation Status](https://readthedocs.org/projects/axe-demultiplexer/badge/?version=latest)](https://readthedocs.org/projects/axe-demultiplexer/?badge=latest)


AXE very rapidly selects the optimal barcode present in a sequence read, even
in the presence of sequencing errors. The algorithm is able to handle
combinatorial barcoding, barcodes of differing length, and several mismatches
per barcode.  Early result indicate far improved accuracy and speed over
existing de-multiplexers. Unscientific trials show AXE processes more than
500,000 reads per second.

**Warning**: Axe has not yet been comprehensively tested. However, in the
spirit of "release early and often", here it is.

###Tests:

| Jenkins GNU/Linux | [![Build Status](http://biojenkins.anu.edu.au/job/axe/badge/icon)](http://biojenkins.anu.edu.au/job/axe/) |
| Jenkins MinGW     | [![Build Status](http://biojenkins.anu.edu.au/job/axe-mingw/badge/icon)](http://biojenkins.anu.edu.au/job/axe-mingw/) |
| TravisCI          | [![Build Status](https://travis-ci.org/kdmurray91/axe.svg?branch=dev)](https://travis-ci.org/kdmurray91/axe) |
| Test Coverage     | [![Coverage Status](https://img.shields.io/coveralls/kdmurray91/axe.svg)](https://coveralls.io/r/kdmurray91/axe?branch=master) |
| Coverity Scans    | [![Coverity Scan Build Status](https://scan.coverity.com/projects/2666/badge.svg)](https://scan.coverity.com/projects/2666) |


Installation:
-------------

Currently, only recent GNU/Linux systems are officially supported. All code and
the build system is portable, so compilation and use on other systems should be
possible, I just don't have machines available to test. Please report any
installation issues on any system as GitHub bugs and I'll do my best to sort
them out.

In short, on Linux, get the dependencies (see below), and:

    git clone --recursive https://github.com/kdmurray91/axe.git axe
    cd axe
    mkdir -p build && cd build
    cmake ..
    make
    sudo make install

To install to a prefix, as you would with `./configure --prefix` with the
autotools build system, please use the following cmake command in place of the
one above:

    cmake -DCMAKE_INSTALL_PREFIX=/path/to/your/prefix ..

e.g.:

    cmake -DCMAKE_INSTALL_PREFIX=~/ ..

For me, using `~/` as the prefix will install `axe` under `/home/kevin/bin` on
GNU/Linux, and (if I had one) `/Users/kevin/bin` on Mac OSX.It's also wise to
use `make install` not `sudo make install` when installing to a home directory.

###Dependencies:

- libgsl: (The GNU scientific library). Install with `sudo apt-get install
  libgsl0-dev` or `brew install gsl`. This dependency will be removed soon, as
  the relevant functions will be bundled with `axe`. If this dependency stops
  you installing `axe`, please pester me and I will make these changes
  immediately, removing the dependency.
- cmake. This is installable via `sudo apt-get install cmake` on Debian based
  systems, or `brew install cmake` using homebrew on OS X.
- zlib version >= 1.2.5. On Debian, use the package `zlib1g-dev`.
- libqes, tinytest and libdatrie (bundled in source, if you used
  `git clone --recursive` or an installation tarball. Otherwise, run
  `git submodule update --init`).

You'll possibly need to install zlib to your chosen prefix (e.g. `~/`) on
supercomputers, which often have very old versions of zlib. To do so:

    wget http://zlib.net/zlib-1.2.8.tar.gz
    tar xvf zlib-1.2.8.tar.gz
    cd zlib-1.2.8
    ./configure --prefix=<your_prefix> # e.g. --prefix=~/
    make && make install

And then, use the following cmake command, assuming your prefix is `~/`:

    cmake -DCMAKE_INSTALL_PREFIX=~/ -DZLIB_ROOT=~/ ..


Usage:
------

Full documentation, including a basic description of the algorithm, is hosted
at https://axe-demultiplexer.readthedocs.org/en/latest/ .


Implementation Progress:
------------------------

 - [x] Single ended read de-multiplexing
 - [x] Interleaved/Paired input and output with single-ended de-multiplexing
 - [x] Combinatorial de-multiplexing
 - [x] CLI integration tests
 - [ ] Comprehensive `libaxe` tests
 - [ ] Comprehensive CLI tests

See also TODO.md


Publication
-----------

A publication is coming soon, if the reviewer gods decide to smile upon us.

Versioning
----------

We use Semantic Versioning. See [semver.org](http://semver.org)

LICENSE
-------

The source of axe itself, namely `src/axe*.[ch]` and `tests/*.[ch]`, is
Copyright 2014 Kevin Murray. All axe source code is licensed under the GNU
GPL version 3, a copy of which is included with this source as `LICENCE.txt`

The source of `tinytest`, located in `tests/tinytest`, is Copyright 2009-2012
Nick Matthewson; `tinytest` is distributed under the 3-clause BSD license.
`tinytest` is hosted at [Nick's github page](https://github.com/nmathewson/tinytest).

The source of `libdatrie`, located in `src/datrie`, is Copyright 2006 Theppitak
Karoonboonyanan, and is licensed under the GNU LGPL version 2.1 per
`src/datrie/COPYING`. `libdatrie` is hosted at Theppitak Karoonboonyanan's
website, [here](http://linux.thai.net/~thep/datrie/datrie.html).
