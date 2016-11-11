/*
 * Copyright 2015 Kevin Murray <spam@kdmurray.id.au>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * ============================================================================
 *
 *       Filename:  qes_libgnu.h
 *    Description:  Functions required from gnulib
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#ifndef QES_LIBGNU_H
#define QES_LIBGNU_H

#include "qes_config.h"

/* This file and qes_libgnu.c are designed to allow us to keep the sources of
 * the gnulib functions intact and in their original separate form. */

#ifndef ZLIB_FOUND
# include "crc.h"
#else
# include <zlib.h>
/* Cast is to avoid a difference in signed-ness in the two implementations. */
# define crc32_update(c, b, l) crc32(c, (const unsigned char *)b, l)
#endif

#endif /* QES_LIBGNU_H */
