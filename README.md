AXE
===

> Demultiplex NGS reads using fast data structures. It's fast, and made of tries!

As a virtue of it's algorithms and data structures, AXE automatically, and very
quickly, selects the longest optimal barcode present in a sequence read, even
in the presence of mutation.

Early result indicate far improved accuracy and speed over existing
demultiplexers. Unscientific trials show AXE processes approximately 1 million
reads per second.

[![Build Status](https://travis-ci.org/kdmurray91/axe.svg?branch=dev)](https://travis-ci.org/kdmurray91/axe)

**Warning**: Axe has not even been fully implemented yet, let alone
comprehensively tested. However, in the spirit of "release early and often",
here it is.

Implementation Progress:
------------------------

 - [x] Single ended read trimming
 - [x] Interleaved/Paired input and output, but R1 only barcoding.
 - [x] Combinatorial barcoding.
 - [ ] Comprehensive `libaxe` tests.
 - [ ] Comprehensive CLI tests.

Installation:
-------------

Currently, only recent GNU/Linux systems are offically supported. All code and
the build system is portable, so compilation and use on other systems should be
possible, I just don't have machines available to test. Please report any
installation issues on any system as github bugs and I'll do my best to sort
them out.

In short, on Linux, get the dependencies (see below), and:

    git clone --recursive https://github.com/kdmurray91/axe.git axe
    cd axe
    mkdir -p build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make
    sudo make install

To install to a prefix, as you would with `./configure --prefix` with the
autotools build system, please use the following cmake command:

    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/path/to/your/prefix ..

e.g.:

    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/home/kevin ..

###Dependencies:

- zlib version >= 1.2.5. On debian, use the pacakge `zlib1g-dev`.
- kmlib, tinytest and libdatrie (bundled in source, if you used
  `git clone --recursive` or an installation tarball)

Usage:
------

Full documentation will appear here, or on readthedocs.org, once the CLI has
been stabilised.

Publication
-----------

A publication is coming soon, if the reviewer gods decide to smile upon us.

LICENSE
-------

The source of axe itself, namely `src/axe*.[ch]` and `tests/*.[ch]`, is
Copyright 2014 Kevin Murray. All axe source code is licenced under the GNU
GPL version 3, a copy of which is included with this source as `LICENCE.txt`

The source of `tinytest`, located in `tests/tinytest`, is Copyright 2009-2012
Nick Matthewson; `tinytest` is distributed under the 3-clause BSD license.
`tinytest` is hosted at [Nick's github page](https://github.com/nmathewson/tinytest).

The source of `libdatrie`, located in `src/datrie`, is Copyright 2006 Theppitak
Karoonboonyanan, and is licenced under the GNU LGPL version 2.1 per
`src/datrie/COPYING`. `libdatrie` is hosted at Theppitak Karoonboonyanan's
website, [here](http://linux.thai.net/~thep/datrie/datrie.html).
