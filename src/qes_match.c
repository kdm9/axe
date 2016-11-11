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
 *       Filename:  qes_match.c
 *
 *    Description:  Sequence matching and finding functions used in
 *                  bioinformatic tasks
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "qes_match.h"


inline int_fast32_t
qes_match_hamming (const char *seq1, const char *seq2, size_t len)
{
    int_fast32_t mismatches = 0;
    size_t iii = 0;

    /* Error out on bad arguments */
    if (seq1 == NULL || seq2 == NULL) {
        return -1;
    }
    /* If we've been given a length of 0, we make it up ourselves */
    if (len == 0) {
        size_t len2 = strlen(seq2);
        len = strlen(seq1);
        /* Max of len & len2 */
        if (len > len2) {
            len = len2;
        }
    }
    /* Count mismatches. See comment on analogous loop in qes_match_hamming_max
     * for an explanation. */
    while(iii < len) {
        if (seq2[iii] != seq1[iii]) {
            mismatches++;
        }
        iii++;
    }
    return mismatches;
}


inline int_fast32_t
qes_match_hamming_max(const char *seq1, const char *seq2, size_t len,
                      int_fast32_t max)
{
    int_fast32_t mismatches = 0;
    size_t iii = 0;

    /* Error out on bad arguments */
    if (seq1 == NULL || seq2 == NULL || max < 0) {
        return -1;
    }
    /* If we've been given a length of 0, we make it up ourselves */
    if (len == 0) {
        size_t len2 = strlen(seq2);
        len = strlen(seq1);
        /* Max of len & len2 */
        if (len > len2) {
            len = len2;
        }
    }
    /* We obediently go until ``len``, assuming whoever gave us ``len`` knew
       WTF they were doing. This makes things a bit faster, since these
       functions are expected to be very much inner-loop. */
    while(iii < len) {
        /* Find mismatch count */
        if (seq2[iii] != seq1[iii]) {
            mismatches++;
        }
        iii++;
        if (mismatches > max) {
            /* Bail out if we're over max, always cap at max + 1 */
            return max + 1;
        }
    }
    return mismatches;
}
