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
 *       Filename:  qes_match.h
 *
 *    Description:  Sequence matching and finding functions used in
 *                  bioinformatic tasks
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#ifndef QES_MATCH_H
#define QES_MATCH_H

#include <qes_util.h>


/*===  FUNCTION  ============================================================*
Name:           qes_match_hamming
Parameters:     const char *seq1, *seq2: Two strings to compare.
                size_t len: Compare ``len`` chars. If 0, guess length with
                strlen (may be unsafe).
Description:    Find the hamming distance between two strings. The strings are
                matched until the length of the smallest string.
Returns:        The hamming distance between ``seq1`` and ``seq2``, or -1 on
                error.
 *===========================================================================*/
extern int_fast32_t qes_match_hamming(const char *seq1, const char *seq2, size_t len);


/*===  FUNCTION  ============================================================*
Name:           qes_match_hamming_max
Parameters:     const char *seq1, *seq2: Two strings to compare.
                size_t len: Compare ``len`` chars. If 0, guess length with
                strlen (may be unsafe).
                int_fast32_t max: Stop counting at ``max``, return ``max + 1``.
Description:    Find the hamming distance between two strings. The strings are
                matched until the length of the smallest string, or ``len``
                charachers, or until the maximum hamming distance (``max``) is
                reached.
Returns:        The hamming distance between ``seq1`` and ``seq2``, or
                ``max + 1`` if the hamming distance exceeds ``max``, or -1 on
                error.
 *===========================================================================*/
extern int_fast32_t qes_match_hamming_max(const char *seq1, const char *seq2, size_t len,
        int_fast32_t max);

#endif /* QES_MATCH_H */
