/*
 * ============================================================================
 *
 *       Filename:  test_libtbd.c
 *
 *    Description:  Tests of core functionality
 *
 *        Version:  1.0
 *        Created:  22/06/14 13:23:46
 *       Revision:  none
 *        License:  GPLv3+
 *       Compiler:  gcc, clang
 *
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "tests.h"

void
test_combinations (void *ptr)
{

    const uint64_t len = 4;
    const uint64_t elem = 2;
    uintptr_t choices[] = {0,0};
    /* Truth from python's itertools */
    uintptr_t truth[][2] = { {0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}, };
    int ret = 0;
    int count = 0;

    while ((ret = combinations(len, elem, choices)) == 1) {
        size_t iii = 0;

        for (iii = 0; iii < elem; iii++) {
            tt_int_op(choices[iii], ==, truth[count][iii]);
        }
        count++;
    }
    tt_int_op(ret, ==, 0);
    tt_int_op(count, ==, 6);
end:
    ;
}


void
test_product (void *ptr)
{

    const uint64_t len = 4;
    const uint64_t elem = 2;
    uintptr_t choices[] = {0,0};
    /* Truth from python's itertools.product */
    uintptr_t truth[][2] = {
        {0, 0}, {0, 1}, {0, 2}, {0, 3},
        {1, 0}, {1, 1}, {1, 2}, {1, 3},
        {2, 0}, {2, 1}, {2, 2}, {2, 3},
        {3, 0}, {3, 1}, {3, 2}, {3, 3}, };
    int ret = 0;
    int count = 0;

    while ((ret = product(len, elem, choices, !ret)) == 1) {
        size_t iii = 0;

        for (iii = 0; iii < elem; iii++) {
            tt_int_op(choices[iii], ==, truth[count][iii]);
        }
        count++;
    }
    tt_int_op(ret, ==, 0);
    tt_int_op(count, ==, 16);
end:
    ;
}

struct testcase_t core_tests[] = {
    { "combinations", test_combinations, 0, NULL, NULL},
    { "product", test_product, 0, NULL, NULL},
    END_OF_TESTCASES
};
