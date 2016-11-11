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
 *       Filename:  test.c
 *
 *    Description:  Tests for libqes
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "tests.h"


struct testgroup_t libqes_tests[] = {
    {"qes/util/", qes_util_tests},
    {"qes/match/", qes_match_tests},
    {"qes/file/", qes_file_tests},
    {"qes/seqfile/", qes_seqfile_tests},
    {"qes/seq/", qes_seq_tests},
    {"qes/log/", qes_log_tests},
    {"qes/sequtil/", qes_sequtil_tests},
    {"testdata/", data_tests},
    {"testhelpers/", helper_tests},
    END_OF_GROUPS
};


/*
 * ===  FUNCTION  =============================================================
 *         Name:  main
 *  Description:  Run all tests
 * ============================================================================
 */

int
main (int argc, const char *argv[])
{
    int res;
    int our_argc = argc;
    const char **our_argv = argv;

    data_prefix = NULL;
    if (argc>1) {
       data_prefix = strdup(argv[1]);
       our_argc -= 1;
       our_argv += 1;
    }
    if (data_prefix == NULL) {
        data_prefix = strdup(".");
        assert(data_prefix != NULL);
    }
    if (access(data_prefix, W_OK | X_OK | R_OK) != 0) {
        fprintf(stderr, "ERROR: Could not access data prefix dir '%s'\n",
                data_prefix);
        fprintf(stderr, "Usage: test_libqes <DATA_DIR> [<test>]\n");
        free(data_prefix);
        exit(EXIT_FAILURE);
    }
    res = tinytest_main(our_argc, our_argv, libqes_tests);
    free(data_prefix);
    return res;
}
