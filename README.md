AXE
===

> De-multiplex NGS reads using trie data structures. It's fast, and made of tries!



[![DOI](https://zenodo.org/badge/6357/kdmurray91/axe.svg)](https://zenodo.org/badge/latestdoi/6357/kdmurray91/axe)
[![Documentation Status](https://readthedocs.org/projects/axe-demultiplexer/badge/?version=latest)](https://readthedocs.org/projects/axe-demultiplexer/?badge=latest)
[![Join the chat at https://gitter.im/axe-demultipexer/Lobby](https://badges.gitter.im/axe-demultipexer/Lobby.svg)](https://gitter.im/axe-demultipexer/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)


AXE very rapidly selects the optimal index present in a sequence read, even
in the presence of sequencing errors. The algorithm is able to handle
combinatorial indexing, indexes of differing length, and several mismatches
per index. Early results indicate far improved accuracy and speed over
existing de-multiplexers. Unscientific trials show AXE processes more than
500,000 reads per second.


Quick Start
-----------

### `conda`

![](https://img.shields.io/badge/Anaconda.org-0.3.3-blue.svg?style=flat-square)

```
mamba install -c kdm801 axe-demultiplexer
```


### From source

To install on UNIX-like systems (Mac, Linux), get the dependencies (zlib and cmake, see below), and:

    git clone --recursive https://github.com/kdmurray91/axe.git axe
    cd axe
    cmake .
    sudo make install

A tutorial on the usage of Axe is available at <https://axe-demultiplexer.readthedocs.org/en/latest/>, along with full documentation and a basic description of the algorithm.

Important Note
--------------

**For arcane reasons, the name of the ``axe`` binary changed to ``axe-demux``
with version 0.3.0. Apologies for the inconvenience, this was required to
make ``axe`` installable in Debian and its derivatives. Command-line usage
did not change.**


Installation:
-------------

Currently, only recent GNU/Linux systems are officially supported. All code and
the build system is portable, so compilation and use on other systems should be
possible, I just don't have machines available to test. Please report any
installation issues on any system as GitHub bugs and I'll do my best to sort
them out.


To install to a prefix, as you would with `./configure --prefix` with the
autotools build system, please use the following cmake command in place of the
one above:

    cmake -DCMAKE_INSTALL_PREFIX=/path/to/your/prefix ..

e.g.:

    cmake -DCMAKE_INSTALL_PREFIX=$HOME ..

For me, using `~/` as the prefix will install `axe` under `/home/kevin/bin` on
GNU/Linux, and (if I had one) `/Users/kevin/bin` on Mac OSX.It's also wise to
use `make install` not `sudo make install` when installing to a home directory.

### Dependencies:

- cmake. This is installable via `sudo apt-get install cmake` on Debian based
  systems, or `brew install cmake` using homebrew on OS X.
- zlib version >= 1.2.5. On Debian, use the package `zlib1g-dev`.
- libqes, tinytest, libgsl and libdatrie (bundled in source, if you used
  `git clone --recursive` or an installation tarball. Otherwise, run
  `git submodule update --init`).

You'll possibly need to install zlib to your chosen prefix (e.g. `~/`) on
supercomputers, which often have very old versions of zlib. To do so:

    wget http://zlib.net/zlib-1.2.8.tar.gz
    tar xvf zlib-1.2.8.tar.gz
    cd zlib-1.2.8
    ./configure --prefix=<your_prefix> # e.g. --prefix=$HOME
    make && make install

And then, use the following cmake command, assuming your prefix is `~/`:

    cmake -DCMAKE_INSTALL_PREFIX=$HOME -DZLIB_ROOT=$HOME ..


Publication
-----------

We have a paper describing AXE published in Bioinformatics https://academic.oup.com/bioinformatics/article/34/22/3924/5026649
A [PDF is available](https://raw.githubusercontent.com/kdm9/axe-paper/master/oxford-final/axe-formatted.pdf?token=GHSAT0AAAAAABVH27O27FSFX47CB5FAUFZ6YV5NNQA)

Versioning
----------

We use Semantic Versioning. See [semver.org](http://semver.org)

LICENSE
-------

The source of axe itself, namely `src/axe*.[ch]` and `tests/*.[ch]`, is
Copyright 2014-2015 Kevin Murray. All axe source code is licensed under the GNU
GPL version 3 or greater, a copy of which is included with this source as
`LICENCE.txt`

The source of `tinytest`, located in `tests/tinytest`, is Copyright 2009-2012
Nick Matthewson; `tinytest` is distributed under the 3-clause BSD license.
`tinytest` is hosted at [Nick's github page](https://github.com/nmathewson/tinytest).

The source of `libgsl`, located in `src/gsl`, is Copyright (C) 1996, 1997,
1998, 1999, 2000, 2007 Gerard Jungman and Brian Gough. It is licensed under the
GNU General Public License, vesion 3 or greater.

The source of `libdatrie`, located in `src/datrie`, is Copyright 2006 Theppitak
Karoonboonyanan, and is licensed under the GNU LGPL version 2.1 per
`src/datrie/COPYING`. `libdatrie` is hosted at Theppitak Karoonboonyanan's
website, [here](http://linux.thai.net/~thep/datrie/datrie.html).
