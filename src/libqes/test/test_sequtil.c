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
 *       Filename:  test_sequtil.c
 *
 *    Description:  Tests for the sequtil module
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "tests.h"
#include <qes_sequtil.h>


static void
test_qes_sequtil_translate_codon (void *ptr)
{
    size_t iii;
    size_t jjj;
    char *cdn = NULL;
    char aa = 0;

    (void) ptr;
    for (iii = 0; iii < n_codons; iii++) {
        aa = qes_sequtil_translate_codon(codon_list[iii]);
        tt_assert_op_type(aa, ==, aa_list[iii], char, "%c");
    }
    tt_int_op(qes_sequtil_translate_codon("XACACA"), ==, -1);
    tt_int_op(qes_sequtil_translate_codon("A"), ==, -1);
    tt_int_op(qes_sequtil_translate_codon(NULL), ==, -1);
    /* Try with mutations */
    for (iii = 0; iii < n_codons; iii++) {
        for (jjj = 0; jjj < 3; jjj++) {
            cdn = strdup(codon_list[iii]);
            cdn[jjj] = 'N';
            aa = qes_sequtil_translate_codon(cdn);
            tt_assert_op_type(aa, ==, 'X', char, "%c");
            free(cdn);
            cdn = NULL;
        }
    }
end:
    if (cdn != NULL) free(cdn);
}

struct testcase_t qes_sequtil_tests[] = {
    { "qes_sequtil_translate_codon", test_qes_sequtil_translate_codon, 0, NULL, NULL},
    END_OF_TESTCASES
};
