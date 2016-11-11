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
 *       Filename:  qes_str.c
 *
 *    Description:  String handling functions
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "qes_str.h"


void
qes_str_print (const struct qes_str *str, FILE *stream)
{
    if (qes_str_ok(str)) {
        fwrite(str->str, 1, str->len, stream);
    }
}

void
qes_str_destroy_cp (struct qes_str *str)
{
    if (str != NULL) qes_free(str->str);
}

void
qes_str_destroy (struct qes_str *str)
{
    qes_str_destroy_cp(str);
    qes_free(str);
}

