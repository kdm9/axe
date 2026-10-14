/*
 * ============================================================================
 *
 *       Filename:  test_libaxe.c
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

static void
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
    size_t iii = 0;

    (void)ptr;
    while ((ret = product(len, elem, choices, !ret)) == 1) {

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

/* Number of positions at which two equal-length strings differ */
static size_t
hamming(const char *a, const char *b)
{
    size_t d = 0;

    for (; *a != '\0' && *b != '\0'; a++, b++) {
        d += (*a != *b);
    }
    return d;
}

static void
test_hamming_mutate (void *ptr)
{
    char **mutated = NULL;
    size_t count = 0;
    size_t iii = 0;
    size_t jjj = 0;
    const char *str = "ACGT";

    (void) ptr;
    /* dist 2 of a 4-mer: C(4,2) * 3^2 = 54 strings, plus the original */
    mutated  = hamming_mutate_dna(&count, str, strlen(str), 2, 1);
    tt_ptr_op(mutated, !=, NULL);
    tt_int_op(count, ==, 55);
    tt_str_op(mutated[0], ==, str);
    for (iii = 1; iii < count; iii++) {
        tt_ptr_op(mutated[iii], !=, NULL);
        /* Exactly dist mismatches, and no duplicates */
        tt_int_op(hamming(mutated[iii], str), ==, 2);
        for (jjj = 0; jjj < iii; jjj++) {
            tt_str_op(mutated[iii], !=, mutated[jjj]);
        }
    }
    for (iii = 0; iii < count; iii++) {
        free(mutated[iii]);
    }
    free(mutated);

    /* Without the original, and dist 1: 4 * 3 */
    mutated  = hamming_mutate_dna(&count, str, strlen(str), 1, 0);
    tt_ptr_op(mutated, !=, NULL);
    tt_int_op(count, ==, 12);
    for (iii = 0; iii < count; iii++) {
        tt_int_op(hamming(mutated[iii], str), ==, 1);
    }
    for (iii = 0; iii < count; iii++) {
        free(mutated[iii]);
    }
    free(mutated);
    mutated = NULL;

    /* Can't have more mismatches than bases */
    tt_ptr_op(hamming_mutate_dna(&count, "AC", 2, 3, 0), ==, NULL);

end:
    ;
}

static void
test_revcomp (void *ptr)
{
    char odd[] = "AACGT";
    char even[] = "AACG";
    char junk[] = "ACxT\n";
    char *rc = NULL;

    (void) ptr;
    qes_sequtil_revcomp_inplace(odd, strlen(odd));
    tt_str_op(odd, ==, "ACGTT");
    qes_sequtil_revcomp_inplace(even, strlen(even));
    tt_str_op(even, ==, "CGTT");
    /* trailing whitespace is trimmed, other bases become N */
    qes_sequtil_revcomp_inplace(junk, strlen(junk));
    tt_str_op(junk, ==, "ANGT");
    /* len larger than the string must not run off its end */
    rc = qes_sequtil_revcomp("AACGT", 100);
    tt_ptr_op(rc, !=, NULL);
    tt_str_op(rc, ==, "ACGTT");
    free(rc);
    /* len smaller than the string uses only the first len bases */
    rc = qes_sequtil_revcomp("AACGT", 3);
    tt_ptr_op(rc, !=, NULL);
    tt_str_op(rc, ==, "GTT");
    free(rc);
    rc = qes_sequtil_revcomp("", 5);
    tt_ptr_op(rc, !=, NULL);
    tt_str_op(rc, ==, "");
end:
    free(rc);
}

struct testcase_t core_tests[] = {
    { "product", test_product, 0, NULL, NULL},
    { "hamming_mutate", test_hamming_mutate, 0, NULL, NULL},
    { "revcomp", test_revcomp, 0, NULL, NULL},
    END_OF_TESTCASES
};
