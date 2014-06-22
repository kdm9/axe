trieBCD
=======

Demultiplex NGS reads using fast data structures.

Installation:
-------------

Currently, only recent GNU/Linux systems are offically supported. All code and
the build system is portable, so compilation and use on other systems should be
possible, I just don't have machines available to test. Please report any
installation issues on any system as github bugs and I'll do my best to sort
them out.

In short, on Linux, get the dependencies (see below), and:

    git clone --recursive https://github.com/kdmurray91/trieBCD.git trieBCD
    cd trieBCD
    mkdir -p build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make
    sudo make install

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

The source of trieBCD itself, namely `src/trieBCD*.[ch]` and `tests/*.[ch]`, is
Copyright 2014 Kevin Murray. All trieBCD source code is licenced under the GNU
GPL version 3, a copy of which is included with this source as `LICENCE.txt`

The source of `tinytest`, located in `tests/tinytest`, is Copyright 2009-2012
Nick Matthewson; `tinytest` is distributed under the 3-clause BSD license.

The source of `libdatrie`, located in `src/datrie`, is Copyright 2006 Theppitak
Karoonboonyanan, and is licenced under the GNU LGPL version 2.1 per
`src/datrie/COPYING`.