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
 *       Filename:  qes_util.c
 *
 *    Description:  Wrappers around std library functions
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "qes_util.h"


/* Pull LIBQES_VERSION in from qes_config.h */
const char *libqes_version = LIBQES_VERSION;

/* Valid non-function to pass to libqes functions */
void
errnil (QES_ERRFN_ARGS)
{
    (void) (msg);
    (void) (file);
    (void) (line);
}

/* Function to pass to libqes functions which prints out errors to stderr */
void
errprint (QES_ERRFN_ARGS)
{
    char msg_fmt[1<<8] = "";
    va_list args;
    if (msg == NULL) {
        msg = "GENERIC ERROR WITH NO MESSAGE";
    }
    va_start (args, line);
    vsnprintf(msg_fmt, 1<<8, msg, args);
    va_end (args);
    fprintf(stderr, "[%s: %d]: %s\n", file, line, msg_fmt);
}

/* Function to pass to libqes functions which prints out errors to stderr and
   calls `exit(EXIT_FAILURE)` */
void
errprintexit (QES_ERRFN_ARGS)
{
    char msg_fmt[1<<8] = "";
    va_list args;
    if (msg == NULL) {
        msg = "GENERIC ERROR WITH NO MESSAGE";
    }
    va_start (args, line);
    vsnprintf(msg_fmt, 1<<8, msg, args);
    va_end (args);
    fprintf(stderr, "[%s: %d]: %s\n", file, line, msg_fmt);
    QES_EXIT_FN(EXIT_FAILURE);
}

