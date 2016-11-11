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
 *       Filename:  helpers.h
 *
 *    Description:  Helpers for tests
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#ifndef HELPERS_H
#define HELPERS_H

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#ifndef _WIN32
# include <unistd.h>
#endif

#include <qes_libgnu.h>


extern char *data_prefix;
char *find_data_file(const char * filepath);
char *get_writable_file(void);
void clean_writable_file(char *filepath);
char *crc32_file(const char *filepath);
int filecmp(const char *file1, const char *file2);

#endif /* HELPERS_H */
