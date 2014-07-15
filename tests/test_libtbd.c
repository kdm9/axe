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

    while ((ret = combinations(len, elem, choices, !ret)) == 1) {
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

void
test_hamming_mutate (void *ptr)
{
    char **mutated = NULL;
    size_t count = 0;
    size_t iii = 0;
    const char *str = "AAAA";
    const char *truth[] = {
        "AAAA", "ACAA", "AGAA", "ATAA", "CAAA", "CCAA", "CGAA", "CTAA", "GAAA",
        "GCAA", "GGAA", "GTAA", "TAAA", "TCAA", "TGAA", "TTAA", "AAAA", "AACA",
        "AAGA", "AATA", "CAAA", "CACA", "CAGA", "CATA", "GAAA", "GACA", "GAGA",
        "GATA", "TAAA", "TACA", "TAGA", "TATA", "AAAA", "AAAC", "AAAG", "AAAT",
        "CAAA", "CAAC", "CAAG", "CAAT", "GAAA", "GAAC", "GAAG", "GAAT", "TAAA",
        "TAAC", "TAAG", "TAAT", "AAAA", "AACA", "AAGA", "AATA", "ACAA", "ACCA",
        "ACGA", "ACTA", "AGAA", "AGCA", "AGGA", "AGTA", "ATAA", "ATCA", "ATGA",
        "ATTA", "AAAA", "AAAC", "AAAG", "AAAT", "ACAA", "ACAC", "ACAG", "ACAT",
        "AGAA", "AGAC", "AGAG", "AGAT", "ATAA", "ATAC", "ATAG", "ATAT", "AAAA",
        "AAAC", "AAAG", "AAAT", "AACA", "AACC", "AACG", "AACT", "AAGA", "AAGC",
        "AAGG", "AAGT", "AATA", "AATC", "AATG", "AATT", };

    mutated  = hamming_mutate_dna(&count, str, strlen(str), 2, 1);
    tt_ptr_op(mutated, !=, NULL);
    tt_int_op(count, ==, 96);
    for (iii = 0; iii < count; iii++) {
        tt_ptr_op(mutated[iii], !=, NULL);
        tt_str_op(mutated[iii], ==, truth[iii]);
    }

end:
    if (mutated != NULL) {
        for (iii = 0; iii < count; iii++) {
            if (mutated[iii] != NULL) {
                free(mutated[iii]);
            }
        }
        free(mutated);

    }
}

struct testcase_t core_tests[] = {
    { "combinations", test_combinations, 0, NULL, NULL},
    { "product", test_product, 0, NULL, NULL},
    { "hamming_mutate", test_hamming_mutate, 0, NULL, NULL},
    END_OF_TESTCASES
};
