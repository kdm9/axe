libqes
======

A C library for various bioinformatics-y tasks. Proper docs will come in time.
For now, we have reasonable test coverage under `./test/` that demonstrate the
API.

###Tests:

| Jenkins GNU/Linux | [![Build Status](http://biojenkins.anu.edu.au/job/libqes/badge/icon)](http://biojenkins.anu.edu.au/job/libqes/) |
| ----------------- | --- |
| Jenkins MinGW     | [![Build Status](http://biojenkins.anu.edu.au/job/libqes-mingw/badge/icon)](http://biojenkins.anu.edu.au/job/libqes-mingw/) |
| TravisCI          | [![Build Status](https://travis-ci.org/kdmurray91/libqes.svg?branch=dev)](https://travis-ci.org/kdmurray91/libqes) |
| Test Coverage     | [![Coverage Status](https://img.shields.io/coveralls/kdmurray91/libqes.svg)](https://coveralls.io/r/kdmurray91/libqes?branch=master) |


License
=======

![GPL logo](http://www.gnu.org/graphics/gplv3-127x51.png)

All libqes source code is licensed under the GNU Public License version 3, or a
later version at your preference.  For license text, see `./gpl-3.0.txt` or
[the GNU website here](http://www.gnu.org/licenses/gpl-3.0.html).

The source of `tinytest`, located in `tests/tinytest`, is Copyright 2009-2012
Nick Matthewson; `tinytest` is distributed under the 3-clause BSD license.
`tinytest` is hosted at
[Nick's github page](https://github.com/nmathewson/tinytest).

`src/crc.[ch]` are from gnulib, and are licensed under the LGPL.
