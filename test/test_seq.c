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
 *       Filename:  test_seq.c
 *
 *    Description:  Test qes_seq.c
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "tests.h"
#include <qes_seq.h>


static void
test_qes_seq_create (void *ptr)
{
    struct qes_seq *seq = NULL;
    (void) ptr;
    tt_ptr_op(seq, ==, NULL);
    seq = qes_seq_create();
    tt_ptr_op(seq, !=, NULL);
    tt_ptr_op(seq->name.str, !=, NULL);
    tt_int_op(seq->name.capacity, >, 0);
    tt_int_op(seq->name.len, ==, 0);
    tt_ptr_op(seq->comment.str, !=, NULL);
    tt_int_op(seq->comment.capacity, >, 0);
    tt_int_op(seq->comment.len, ==, 0);
    tt_ptr_op(seq->seq.str, !=, NULL);
    tt_int_op(seq->seq.capacity, >, 0);
    tt_int_op(seq->seq.len, ==, 0);
    tt_ptr_op(seq->qual.str, !=, NULL);
    tt_int_op(seq->qual.capacity, >, 0);
    tt_int_op(seq->qual.len, ==, 0);
    tt_assert(qes_str_ok(&seq->name));
    tt_assert(qes_str_ok(&seq->comment));
    tt_assert(qes_str_ok(&seq->seq));
    tt_assert(qes_str_ok(&seq->qual));
end:
    qes_seq_destroy(seq);
}

static void
test_qes_seq_create_no_qual (void *ptr)
{
    struct qes_seq *seq = NULL;
    (void) ptr;
    tt_ptr_op(seq, ==, NULL);
    seq = qes_seq_create_no_qual();
    tt_ptr_op(seq, !=, NULL);
    tt_ptr_op(seq->name.str, !=, NULL);
    tt_int_op(seq->name.capacity, >, 0);
    tt_int_op(seq->name.len, ==, 0);
    tt_ptr_op(seq->comment.str, !=, NULL);
    tt_int_op(seq->comment.capacity, >, 0);
    tt_int_op(seq->comment.len, ==, 0);
    tt_ptr_op(seq->seq.str, !=, NULL);
    tt_int_op(seq->seq.capacity, >, 0);
    tt_int_op(seq->seq.len, ==, 0);
    tt_ptr_op(seq->qual.str, ==, NULL);
    tt_int_op(seq->qual.capacity, ==, 0);
    tt_int_op(seq->qual.len, ==, 0);
    tt_assert(qes_str_ok(&seq->name));
    tt_assert(qes_str_ok(&seq->comment));
    tt_assert(qes_str_ok(&seq->seq));
    tt_assert(!qes_str_ok(&seq->qual));
end:
    qes_seq_destroy(seq);
}

static void
test_qes_seq_create_no_qual_or_comment (void *ptr)
{
    struct qes_seq *seq = NULL;
    (void) ptr;
    tt_ptr_op(seq, ==, NULL);
    seq = qes_seq_create_no_qual_or_comment();
    tt_ptr_op(seq, !=, NULL);
    tt_ptr_op(seq->name.str, !=, NULL);
    tt_int_op(seq->name.capacity, >, 0);
    tt_int_op(seq->name.len, ==, 0);
    tt_ptr_op(seq->comment.str, ==, NULL);
    tt_int_op(seq->comment.capacity, ==, 0);
    tt_int_op(seq->comment.len, ==, 0);
    tt_ptr_op(seq->seq.str, !=, NULL);
    tt_int_op(seq->seq.capacity, >, 0);
    tt_int_op(seq->seq.len, ==, 0);
    tt_ptr_op(seq->qual.str, ==, NULL);
    tt_int_op(seq->qual.capacity, ==, 0);
    tt_int_op(seq->qual.len, ==, 0);
    tt_assert(qes_str_ok(&seq->name));
    tt_assert(!qes_str_ok(&seq->comment));
    tt_assert(qes_str_ok(&seq->seq));
    tt_assert(!qes_str_ok(&seq->qual));
end:
    qes_seq_destroy(seq);
}

static void
test_qes_seq_ok (void *ptr)
{
    struct qes_seq *seq = NULL;
    (void) ptr;
    /* Test null seq */
    tt_assert(!qes_seq_ok(seq));
    /* Make valid seq */
    seq = qes_seq_create();
    tt_assert(qes_seq_ok(seq));
    /* invalidate name, should fail */
    qes_str_destroy_cp(&seq->name);
    tt_assert(!qes_seq_ok(seq));
    qes_seq_destroy(seq);
    /* remake */
    seq = qes_seq_create();
    tt_assert(qes_seq_ok(seq));
    /* invalidate comment, should fail */
    qes_str_destroy_cp(&seq->comment);
    tt_assert(!qes_seq_ok(seq));
    qes_seq_destroy(seq);
    /* remake */
    seq = qes_seq_create();
    tt_assert(qes_seq_ok(seq));
    /* invalidate seq, should fail */
    qes_str_destroy_cp(&seq->seq);
    tt_assert(!qes_seq_ok(seq));
    qes_seq_destroy(seq);
    /* remake */
    seq = qes_seq_create();
    tt_assert(qes_seq_ok(seq));
    /* invalidate qual, should fail */
    qes_str_destroy_cp(&seq->qual);
    tt_assert(!qes_seq_ok(seq));
    qes_seq_destroy(seq);
    /* Destroy seq, invalidating it */
    qes_seq_destroy(seq);
    tt_assert(!qes_seq_ok(seq));
end:
    qes_seq_destroy(seq);
}

static void
test_qes_seq_ok_no_comment (void *ptr)
{
    struct qes_seq *seq = NULL;

    (void) ptr;
    /* Test null seq */
    tt_assert(!qes_seq_ok_no_comment(seq));
    /* Make valid seq */
    seq = qes_seq_create();
    tt_assert(qes_seq_ok_no_comment(seq));
    /* invalidate name, should fail */
    qes_str_destroy_cp(&seq->name);
    tt_assert(!qes_seq_ok_no_comment(seq));
    qes_seq_destroy(seq);
    /* remake */
    seq = qes_seq_create();
    tt_assert(qes_seq_ok_no_comment(seq));
    /* invalidate comment, should still pass */
    qes_str_destroy_cp(&seq->comment);
    tt_assert(qes_seq_ok_no_comment(seq));
    qes_seq_destroy(seq);
    /* remake */
    seq = qes_seq_create();
    tt_assert(qes_seq_ok_no_comment(seq));
    /* invalidate seq, should fail */
    qes_str_destroy_cp(&seq->seq);
    tt_assert(!qes_seq_ok_no_comment(seq));
    qes_seq_destroy(seq);
    /* remake */
    seq = qes_seq_create();
    tt_assert(qes_seq_ok_no_comment(seq));
    /* invalidate qual, should fail */
    qes_str_destroy_cp(&seq->qual);
    tt_assert(!qes_seq_ok_no_comment(seq));
    qes_seq_destroy(seq);
    /* Destroy seq, invalidating it */
    qes_seq_destroy(seq);
    tt_assert(!qes_seq_ok_no_comment(seq));
end:
    qes_seq_destroy(seq);
}

static void
test_qes_seq_ok_no_qual (void *ptr)
{
    struct qes_seq *seq = NULL;
    (void) ptr;
    /* Test null seq */
    tt_assert(!qes_seq_ok_no_qual(seq));
    /* Make valid seq */
    seq = qes_seq_create();
    tt_assert(qes_seq_ok_no_qual(seq));
    /* invalidate name, should fail */
    qes_str_destroy_cp(&seq->name);
    tt_assert(!qes_seq_ok_no_qual(seq));
    qes_seq_destroy(seq);
    /* remake */
    seq = qes_seq_create();
    tt_assert(qes_seq_ok_no_qual(seq));
    /* invalidate comment, should fail */
    qes_str_destroy_cp(&seq->comment);
    tt_assert(!qes_seq_ok_no_qual(seq));
    qes_seq_destroy(seq);
    /* remake */
    seq = qes_seq_create();
    tt_assert(qes_seq_ok_no_qual(seq));
    /* invalidate seq, should fail */
    qes_str_destroy_cp(&seq->seq);
    tt_assert(!qes_seq_ok_no_qual(seq));
    qes_seq_destroy(seq);
    /* remake */
    seq = qes_seq_create();
    tt_assert(qes_seq_ok_no_qual(seq));
    /* invalidate qual, should PASS */
    qes_str_destroy_cp(&seq->qual);
    tt_assert(qes_seq_ok_no_qual(seq));
    qes_seq_destroy(seq);
    /* Destroy seq, invalidating it */
    qes_seq_destroy(seq);
    tt_assert(!qes_seq_ok_no_qual(seq));
end:
    qes_seq_destroy(seq);
}

static void
test_qes_seq_print(void *ptr)
{
    struct qes_seq *seq = NULL;
    FILE *fp = NULL;
    char *outfile = NULL;
    char *truthfile = NULL;
    int res = 1;

    (void) ptr;
    /* Create output file */
    outfile = get_writable_file();
    fp = fopen(outfile, "w");
    tt_ptr_op(fp, !=, NULL);
    /* create sequence, fill it */
    seq = qes_seq_create();
    res = qes_seq_fill(seq, "TEST", "Comment 1", "AGCT", "IIII");
    tt_int_op(res, ==, 0);
    /* Print the seq to the output file */
    res = qes_seq_print(seq, fp, false, 0);
    tt_int_op(res, ==, 0);
    /* Check printing */
    truthfile = find_data_file("truth/qes_seq_print.fq");
    tt_ptr_op(truthfile, !=, NULL);
    tt_int_op(filecmp(outfile, truthfile), ==, 0);
    /* Clean up */
    free(truthfile);
    truthfile = NULL;
    fclose(fp);
    fp = NULL;
    qes_seq_destroy(seq);

    /* recreate, just without quality */
    seq = qes_seq_create();
    fp = fopen(outfile, "w");
    tt_ptr_op(fp, !=, NULL);
    res = qes_seq_fill(seq, "TEST", "Comment 1", "AGCT", "");
    tt_int_op(res, ==, 0);
    /* Print the seq to the output file */
    res = qes_seq_print(seq, fp, true, 0);
    tt_int_op(res, ==, 0);
    /* Check printing */
    truthfile = find_data_file("truth/qes_seq_print.fa");
    tt_ptr_op(truthfile, !=, NULL);
    tt_int_op(filecmp(outfile, truthfile), ==, 0);
    free(truthfile);
    truthfile = NULL;
    fclose(fp);
    fp = NULL;

    /* error cases  */
    tt_int_op(qes_seq_print(NULL, stdout, false, 0), ==, 1);
    tt_int_op(qes_seq_print(seq, NULL, false, 0), ==, 1);

end:
    qes_seq_destroy(seq);
    if (fp != NULL) fclose(fp);
    if (outfile != NULL) free(outfile);
    if (truthfile != NULL) free(truthfile);
}

static void
test_qes_seq_ok_no_comment_or_qual (void *ptr)
{
    struct qes_seq *seq = NULL;

    (void) ptr;
    /* Test null seq */
    tt_assert(!qes_seq_ok_no_comment_or_qual(seq));
    /* Make valid seq */
    seq = qes_seq_create();
    tt_assert(qes_seq_ok_no_comment_or_qual(seq));
    /* invalidate name, should fail */
    qes_str_destroy_cp(&seq->name);
    tt_assert(!qes_seq_ok_no_comment_or_qual(seq));
    qes_seq_destroy(seq);
    /* remake */
    seq = qes_seq_create();
    tt_assert(qes_seq_ok_no_comment_or_qual(seq));
    /* invalidate comment, should still pass */
    qes_str_destroy_cp(&seq->comment);
    tt_assert(qes_seq_ok_no_comment_or_qual(seq));
    qes_seq_destroy(seq);
    /* remake */
    seq = qes_seq_create();
    tt_assert(qes_seq_ok_no_comment_or_qual(seq));
    /* invalidate seq, should fail */
    qes_str_destroy_cp(&seq->seq);
    tt_assert(!qes_seq_ok_no_comment_or_qual(seq));
    qes_seq_destroy(seq);
    /* remake */
    seq = qes_seq_create();
    tt_assert(qes_seq_ok_no_comment_or_qual(seq));
    /* invalidate qual, should fail */
    qes_str_destroy_cp(&seq->qual);
    tt_assert(qes_seq_ok_no_comment_or_qual(seq));
    qes_seq_destroy(seq);
    /* Destroy seq, invalidating it */
    qes_seq_destroy(seq);
    tt_assert(!qes_seq_ok_no_comment_or_qual(seq));
end:
    qes_seq_destroy(seq);
}


static void
test_qes_seq_destroy (void *ptr)
{
    struct qes_seq *seq = NULL;

    (void) ptr;
    tt_ptr_op(seq, ==, NULL);
    seq = qes_seq_create();
    tt_ptr_op(seq, !=, NULL);
    qes_seq_destroy(seq);
    tt_ptr_op(seq, ==, NULL);
    seq = NULL; /* Best be sure */
    qes_seq_destroy(seq);
end:
    qes_seq_destroy(seq);
}

static void
test_qes_seq_copy(void *ptr)
{
    struct qes_seq *seq = NULL;
    struct qes_seq *copy = NULL;
    int res = 1;

    (void) ptr;
    seq = qes_seq_create();
    copy = qes_seq_create();
    res = qes_seq_fill(seq, "TEST", "Comment 1", "AGCT", "IIII");
    tt_int_op(res, ==, 0);
    tt_str_op(seq->name.str, ==, "TEST");
    tt_str_op(seq->comment.str, ==, "Comment 1");
    tt_str_op(seq->seq.str, ==, "AGCT");
    tt_str_op(seq->qual.str, ==, "IIII");
    res = qes_seq_copy(copy, seq);
    tt_int_op(res, ==, 0);
    tt_str_op(copy->name.str, ==, "TEST");
    tt_str_op(copy->comment.str, ==, "Comment 1");
    tt_str_op(copy->seq.str, ==, "AGCT");
    tt_str_op(copy->qual.str, ==, "IIII");
    tt_int_op(qes_seq_copy(NULL, seq), ==, 1);
    tt_int_op(qes_seq_copy(seq, NULL), ==, 1);
    tt_int_op(qes_seq_copy(seq, seq), ==, 1);

end:
    qes_seq_destroy(seq);
    qes_seq_destroy(copy);
}

static void
test_qes_seq_fill_funcs(void *ptr)
{
#define CHECK_FILLING(submember, st, ln)                                     \
    seq = qes_seq_create();                                                  \
    tt_str_op(seq->submember.str, ==, "");                                   \
    res = qes_seq_fill_ ##submember (seq, st, ln);                           \
    tt_int_op(res, ==, 0);                                                   \
    tt_str_op(seq->submember.str, ==, st);                                   \
    tt_int_op(seq->submember.len, ==, ln);                                   \
    tt_int_op(seq->submember.capacity, >=, ln);                              \
    qes_seq_destroy(seq);
#define CHECK_FILLING_FAIL(submember, st, ln)                                \
    seq = qes_seq_create();                                                  \
    tt_str_op(seq->submember.str, ==, "");                                   \
    res = qes_seq_fill_ ##submember (seq, st, ln);                           \
    tt_int_op(res, ==, 1);                                                   \
    tt_str_op(seq->submember.str, ==, "");                                   \
    tt_int_op(seq->submember.len, ==, 0);                                    \
    qes_seq_destroy(seq);

    struct qes_seq *seq = NULL;
    int res = 0;
    char *tmp = NULL;

    (void) ptr;
    /* These should all work pretty well */
    CHECK_FILLING(name, "HWI_TEST", 8)
    CHECK_FILLING(comment, "abc 123 comment", 15)
    CHECK_FILLING(seq, "ACTG", 4)
    CHECK_FILLING(qual, "IIII", 4)
    /* These should all fail */
    CHECK_FILLING_FAIL(name, NULL, 1)
    CHECK_FILLING_FAIL(name, "BAD", 0)
    CHECK_FILLING_FAIL(comment, NULL, 1)
    CHECK_FILLING_FAIL(comment, "BAD", 0)
    CHECK_FILLING_FAIL(seq, NULL, 1)
    CHECK_FILLING_FAIL(seq, "BAD", 0)
    CHECK_FILLING_FAIL(qual, NULL, 1)
    CHECK_FILLING_FAIL(qual, "BAD", 0)
    tt_int_op(qes_seq_fill_name(NULL, "BAD", 3), ==, 1);
    tt_int_op(qes_seq_fill_comment(NULL, "BAD", 3), ==, 1);
    tt_int_op(qes_seq_fill_seq(NULL, "BAD", 3), ==, 1);
    tt_int_op(qes_seq_fill_qual(NULL, "BAD", 3), ==, 1);

    /* Fill header */
#define CHECK_FILL_HEADER(st, ln, nm, nmlen, com, comlen)                    \
    tmp = strdup(st);                                                        \
    seq = qes_seq_create();                                                  \
    tt_str_op(seq->name.str, ==, "");                                        \
    tt_str_op(seq->comment.str, ==, "");                                     \
    res = qes_seq_fill_header(seq, tmp, ln);                                 \
    tt_int_op(res, ==, 0);                                                   \
    tt_str_op(seq->name.str, ==, nm);                                        \
    tt_int_op(seq->name.len, ==, nmlen);                                     \
    tt_int_op(seq->name.capacity, >=, nmlen);                                \
    tt_str_op(seq->comment.str, ==, com);                                    \
    tt_int_op(seq->comment.len, ==, comlen);                                 \
    tt_int_op(seq->comment.capacity, >=, comlen);                            \
    qes_seq_destroy(seq);                                                    \
    free(tmp);                                                               \
    tmp = NULL;
    CHECK_FILL_HEADER("@HWI_TEST COMM\n", 15, "HWI_TEST", 8, "COMM", 4)
    CHECK_FILL_HEADER("@HWI_TEST COMM\n", 0, "HWI_TEST", 8, "COMM", 4)
    CHECK_FILL_HEADER("@HWI_TEST COMM \r\n", 17, "HWI_TEST", 8, "COMM", 4)
    CHECK_FILL_HEADER("@HWI_TEST COMM", 14, "HWI_TEST", 8, "COMM", 4)
    CHECK_FILL_HEADER(">HWI_TEST COMM", 14, "HWI_TEST", 8, "COMM", 4)
    CHECK_FILL_HEADER("HWI_TEST COMM", 13, "HWI_TEST", 8, "COMM", 4)
    CHECK_FILL_HEADER("@HWI_TEST", 9, "HWI_TEST", 8, "", 0)
    CHECK_FILL_HEADER(">HWI_TEST", 9, "HWI_TEST", 8, "", 0)
    CHECK_FILL_HEADER("HWI_TEST", 8, "HWI_TEST", 8, "", 0)
    /* Check bad values */
    seq = qes_seq_create();
    tmp = strdup("BAD");
    tt_int_op(qes_seq_fill_header(NULL, tmp, 3), ==, 1);
    tt_int_op(qes_seq_fill_header(seq, NULL, 3), ==, 1);
    qes_seq_destroy(seq);
end:
    if (tmp != NULL) {
        free(tmp);
    }
    qes_seq_destroy(seq);
#undef CHECK_FILLING
#undef CHECK_FILLING_FAIL
#undef CHECK_FILL_HEADER
}


struct testcase_t qes_seq_tests[] = {
    { "qes_seq_create", test_qes_seq_create, 0, NULL, NULL},
    { "qes_seq_create_no_qual", test_qes_seq_create_no_qual, 0, NULL, NULL},
    { "qes_seq_create_no_qual_or_comment",
        test_qes_seq_create_no_qual_or_comment, 0, NULL, NULL},
    { "qes_seq_ok", test_qes_seq_ok, 0, NULL, NULL},
    { "qes_seq_ok_no_comment", test_qes_seq_ok_no_comment, 0, NULL, NULL},
    { "qes_seq_ok_no_qual", test_qes_seq_ok_no_qual, 0, NULL, NULL},
    { "qes_seq_ok_no_comment_or_qual", test_qes_seq_ok_no_comment_or_qual, 0,
        NULL, NULL},
    { "qes_seq_destroy", test_qes_seq_destroy, 0, NULL, NULL},
    { "qes_seq_fill", test_qes_seq_fill_funcs, 0, NULL, NULL},
    { "qes_seq_copy", test_qes_seq_copy, 0, NULL, NULL},
    { "qes_seq_print", test_qes_seq_print, 0, NULL, NULL},
    END_OF_TESTCASES
};
