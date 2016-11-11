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
 *       Filename:  test_util.c
 *
 *    Description:  Test qes_util.c
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "tests.h"
#include <qes_util.h>


/* Actual tests */
static void
test_qes_calloc(void *ptr)
{
    void *res = NULL;
    const size_t bufsize = 1<<10;
    unsigned char *zeros[1<<10];

    (void) ptr;
    memset(zeros, 0, bufsize);
    /* This should work, and the buffer should be zero throughout */
    res = qes_calloc(1, 1);
    tt_ptr_op(res, !=, NULL);
    tt_int_op(memcmp(res, zeros, 1), ==, 0);
    free(res);
    res = NULL;
end:
    if (res != NULL) free(res);
}

static void
test_qes_malloc(void *ptr)
{
    void *res = NULL;

    (void) ptr;
    res = qes_malloc(1);
    tt_ptr_op(res, !=, NULL);
    free(res);
    res = NULL;
end:
    if (res != NULL) free(res);
}

static void
test_qes_realloc(void *ptr)
{
    char *res = NULL;
    const char *str = "test";
    char *dat = strdup(str);

    (void) ptr;
    /* Test resizing buffer */
    res = qes_realloc(dat, 10);
    dat = NULL;
    tt_ptr_op(res, !=, NULL);
    tt_int_op(memcmp(res, str, 5), ==, 0);
    free(res);
    res = NULL;
end:
    if (res != NULL) free(res);
    if (dat != NULL) free(dat);
}

static void
test_qes_free(void *ptr)
{
    char *dat = strdup("test");

    (void) ptr;
    /* Test freeing buffer */
    tt_ptr_op(dat, !=, NULL);
    qes_free(dat);
    tt_ptr_op(dat, ==, NULL);
    /* This free(NULL) should not fail */
    qes_free(dat);
    tt_ptr_op(dat, ==, NULL);
end:
    if (dat != NULL) free(dat);
}

static void
test_qes_roundup32 (void *ptr)
{
    int32_t val = 3;
    uint32_t uval = (1u<<31) - 1;

    (void) ptr;
    /* Signed */
    tt_int_op(qes_roundup32(val), ==, 4);
    val++;
    tt_int_op(qes_roundup32(val), ==, 8);
    val = 8;
    tt_int_op(qes_roundup32(val), ==, 16);
    val = 262143;
    tt_int_op(qes_roundup32(val), ==, 262144);
    /* Unsigned */
    tt_int_op(qes_roundup32(uval), ==, 1u<<31);
    uval++;
    tt_int_op(qes_roundup32(uval), ==, 0);
end:
    ;
}

static void
test_qes_roundup64 (void *ptr)
{
    int64_t val = 3;
    uint64_t uval = (1llu<<63) - 1;

    (void) ptr;
    /* Signed */
    tt_int_op(qes_roundup64(val), ==, 4);
    val = 4;
    tt_int_op(qes_roundup64(val), ==, 8);
    val = 8;
    tt_int_op(qes_roundup64(val), ==, 16);
    val = 262143llu;
    tt_int_op(qes_roundup64(val), ==, 262144);
    /* Unsigned */
    tt_assert(qes_roundup64(uval) == 1llu<<63);
    uval = 1llu<<62;
    tt_assert(qes_roundup64(uval) == 1llu<<63);
    uval = 63llu;
    tt_assert(qes_roundup64(uval) - 2 == 62llu);
end:
    ;
}


struct testcase_t qes_util_tests[] = {
    { "qes_calloc", test_qes_calloc, 0, NULL, NULL},
    { "qes_malloc", test_qes_malloc, 0, NULL, NULL},
    { "qes_realloc", test_qes_realloc, 0, NULL, NULL},
    { "qes_free", test_qes_free, 0, NULL, NULL},
    { "qes_roundup32", test_qes_roundup32, 0, NULL, NULL},
    { "qes_roundup64", test_qes_roundup64, 0, NULL, NULL},
    END_OF_TESTCASES
};
