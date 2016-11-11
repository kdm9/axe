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
 *       Filename:  qes_compat.c
 *
 *    Description:  Compatibility helpers for cross-platformness
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "qes_compat.h"


#ifndef STRNDUP_FOUND
char *
strndup(const char *s, size_t n)
{
    char *dest = malloc(n+1);
    if (dest == NULL) return NULL;
    strncpy(dest, s, n);
    dest[n] = '\0';
    return dest;
}
#endif

#ifndef VASPRINTF_FOUND
int vasprintf(char **ret, const char *format, va_list args)
{
    va_list copy;
    int count;
    va_copy(copy, args);

    *ret = NULL;

    count = vsnprintf(NULL, 0, format, args);
    if (count >= 0) {
        char *buffer = malloc(count + 1);
        if (buffer == NULL) {
            count = -1;
        } else if ((count = vsnprintf(buffer, count + 1, format, copy)) < 0) {
            free(buffer);
        }
        else {
            *ret = buffer;
        }
    }
    va_end(copy);

    return count;
}
#endif

#ifndef ASPRINTF_FOUND
int asprintf(char **ret, const char *format, ...)
{
    va_list args;
    int count;

    va_start(args, format);
    count = vasprintf(ret, format, args);
    va_end(args);
    return(count);
}
#endif
