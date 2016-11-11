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
 *       Filename:  test_seqfile.c
 *
 *    Description:  Tests for the qes_seqfile module
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "tests.h"
#include <qes_seqfile.h>
#include "kseq.h"


#ifdef ZLIB_FOUND
#  include <zlib.h>
   KSEQ_INIT(gzFile, gzread)
#else
#  include <sys/stat.h>
#  include <fcntl.h>
   KSEQ_INIT(int, read)
#endif



static void
test_qes_seqfile_create(void *ptr)
{
    struct qes_seqfile *sf = NULL;
    char *fname = NULL;

    (void) ptr;
    /* Test file opening for reading */
    /* test opening a valid, unziped FASTQ */
    fname = find_data_file("test.fastq");
    tt_assert(fname != NULL);
    sf = qes_seqfile_create(fname, "r");
    tt_ptr_op(sf, !=, NULL);
    tt_ptr_op(sf->qf, !=, NULL);
    tt_int_op(sf->qf->mode, ==, QES_FILE_MODE_READ);
    tt_int_op(sf->n_records, ==, 0);
    tt_int_op(sf->format, ==, FASTQ_FMT);
    qes_seqfile_destroy(sf);
    free(fname);
    /* test opening a valid, unziped FASTA */
    fname = find_data_file("test.fasta");
    tt_assert(fname != NULL);
    sf = qes_seqfile_create(fname, "r");
    tt_ptr_op(sf, !=, NULL);
    tt_ptr_op(sf->qf, !=, NULL);
    tt_int_op(sf->qf->mode, ==, QES_FILE_MODE_READ);
    tt_int_op(sf->n_records, ==, 0);
    tt_int_op(sf->format, ==, FASTA_FMT);
    qes_seqfile_destroy(sf);
    free(fname);
    /* Test opening a file in transparent write mode */
    fname = get_writable_file();
    tt_assert(fname != NULL);
    sf = qes_seqfile_create(fname, "wT");
    QES_ZWRITE(sf->qf->fp, "ABCD", 4);
    tt_ptr_op(sf, !=, NULL);
    tt_ptr_op(sf->qf, !=, NULL);
    tt_int_op(sf->qf->mode, ==, QES_FILE_MODE_WRITE);
    tt_int_op(sf->n_records, ==, 0);
    tt_int_op(access(fname, F_OK), ==, 0);
    qes_seqfile_destroy(sf);
    clean_writable_file(fname);
    fname = NULL;
    /* Test opening a file in write mode , acually zipping output */
    fname = get_writable_file();
    tt_assert(fname != NULL);
    sf = qes_seqfile_create(fname, "w9");
    tt_ptr_op(sf, !=, NULL);
    tt_ptr_op(sf->qf, !=, NULL);
    tt_int_op(sf->qf->mode, ==, QES_FILE_MODE_WRITE);
    tt_int_op(sf->n_records, ==, 0);
    tt_int_op(access(fname, F_OK), ==, 0);
    qes_seqfile_destroy(sf);
    clean_writable_file(fname);
    fname = NULL;
end:
    qes_seqfile_destroy(sf);
    if (fname != NULL) {
        free(fname);
    }
}


static void
test_qes_seqfile_guess_format (void *ptr)
{
    struct qes_seqfile *sf = NULL;
    int res = -1;
    char *fname = NULL;

    (void) ptr;
    /* Test file opening for reading */
    fname = find_data_file("test.fastq");
    tt_assert(fname != NULL);
    sf = qes_seqfile_create(fname, "r");
    /* test with a FASTQ file */
    res = qes_seqfile_guess_format(sf);
    tt_int_op(res, ==, FASTQ_FMT);
    tt_int_op(sf->format, ==, FASTQ_FMT);
    qes_seqfile_destroy(sf);
    free(fname);
    /* test with a FASTA file */
    fname = find_data_file("test.fasta");
    tt_assert(fname != NULL);
    sf = qes_seqfile_create(fname, "r");
    res = qes_seqfile_guess_format(sf);
    tt_int_op(res, ==, FASTA_FMT);
    tt_int_op(sf->format, ==, FASTA_FMT);
    qes_seqfile_destroy(sf);
    free(fname);
    fname = NULL;
#ifdef ZLIB_FOUND
    /* test with a gziped FASTQ file */
    fname = find_data_file("test.fastq.gz");
    tt_assert(fname != NULL);
    sf = qes_seqfile_create(fname, "r");
    res = qes_seqfile_guess_format(sf);
    tt_int_op(res, ==, FASTQ_FMT);
    tt_int_op(sf->format, ==, FASTQ_FMT);
    qes_seqfile_destroy(sf);
#endif
end:
    qes_seqfile_destroy(sf);
    if (fname != NULL) free(fname);
}


static void
test_qes_seqfile_destroy (void *ptr)
{
    struct qes_seqfile *sf = NULL;
    char *fname = NULL;

    (void) ptr;
    /* Test file opening for reading */
    fname = find_data_file("test.fastq");
    tt_assert(fname != NULL);
    sf = qes_seqfile_create(fname, "r");
    tt_ptr_op(sf, !=, NULL);
    tt_assert(qes_seqfile_ok(sf));
    qes_seqfile_destroy(sf);
    tt_ptr_op(sf, ==, NULL);
    tt_assert(!qes_seqfile_ok(sf));
end:
    qes_seqfile_destroy(sf);
    if (fname != NULL) free(fname);
}


/*===  FUNCTION  ============================================================*
Name:           test_qes_seqfile_read
Description:    Tests the qes_seqfile_read function from qes_seqfile.c
 *===========================================================================*/
static void
test_qes_seqfile_read (void *ptr)
{
    struct qes_seq *seq = qes_seq_create();
    ssize_t res = 0;
    struct qes_seqfile *sf = NULL;
    char *fname = NULL;
    /* Check a seq is empty */
#define CHECK_SEQ_EMPTY                                                     \
    tt_str_op(seq->name.str, ==, "");                                       \
    tt_str_op(seq->comment.str, ==, "");                                    \
    tt_str_op(seq->seq.str, ==, "");                                        \
    tt_str_op(seq->qual.str, ==, "")
    /* Check seq against the first known read */
#define CHECK_SEQ_FIRST                                                     \
    tt_str_op(seq->name.str, ==, first_fastq_read[0]);                      \
    tt_str_op(seq->comment.str, ==, first_fastq_read[1]);                   \
    tt_str_op(seq->seq.str, ==, first_fastq_read[2]);                       \
    tt_str_op(seq->qual.str, ==, first_fastq_read[3])
    /* Open, read, check & close a seqfile */
#define CHECK_SEQFILE_READ(fn, expt_res, check_seq)                         \
    fname = find_data_file(fn);                                             \
    tt_assert(fname != NULL);                                               \
    sf = qes_seqfile_create(fname, "r");                                    \
    res = qes_seqfile_read(sf, seq);                                        \
    tt_int_op(res, ==, expt_res);                                           \
    check_seq;                                                              \
    qes_seqfile_destroy(sf);                                                \
    free(fname);                                                            \
    fname = NULL
    /* Open, read, check & close a seqfile, forcing its filetype to FASTQ */
#define CHECK_SEQFILE_READ_FORCE(fn, expt_res, check_seq)                   \
    fname = find_data_file(fn);                                             \
    tt_assert(fname != NULL);                                               \
    sf = qes_seqfile_create(fname, "r");                                    \
    qes_seqfile_set_format(sf, FASTQ_FMT);                                  \
    res = qes_seqfile_read(sf, seq);                                        \
    tt_int_op(res, ==, expt_res);                                           \
    check_seq;                                                              \
    qes_seqfile_destroy(sf);                                                \
    free(fname);                                                            \
    fname = NULL

    (void) ptr;
    /* Test file opening for reading */
    CHECK_SEQFILE_READ("test.fastq", first_fastq_len, CHECK_SEQ_FIRST);
    CHECK_SEQFILE_READ("test.fasta", 33,
        tt_str_op(seq->name.str, ==, "HWI-ST960:105:D10GVACXX:2:1101:1122:2186");
        tt_str_op(seq->comment.str, ==, "1:N:0: bcd:RPI8 seq:CACACTTGAATC");
        tt_str_op(seq->seq.str, ==, "CACACTTGAATCCAGTTTAAAGTTAACTCATTG");
        tt_int_op(seq->qual.len, ==, 0);
        tt_str_op(seq->qual.str, ==, "")
        );
    CHECK_SEQFILE_READ("nocomment.fasta", 33,
        tt_str_op(seq->name.str, ==, "HWI-ST960:105:D10GVACXX:2:1101:1122:2186");
        tt_int_op(seq->comment.len, ==, 0);
        tt_str_op(seq->comment.str, ==, "");
        tt_str_op(seq->seq.str, ==, "CACACTTGAATCCAGTTTAAAGTTAACTCATTG");
        tt_int_op(seq->qual.len, ==, 0);
        tt_str_op(seq->qual.str, ==, "")
        );
    /* Test with bad fastqs, ensure all code paths are taken */
    CHECK_SEQFILE_READ("loremipsum.txt", -2, CHECK_SEQ_EMPTY);
    CHECK_SEQFILE_READ("bad_nohdr.fastq", -2, CHECK_SEQ_EMPTY);
    CHECK_SEQFILE_READ("loremipsum.txt", -2, CHECK_SEQ_EMPTY);
    CHECK_SEQFILE_READ_FORCE("loremipsum.txt", -3, CHECK_SEQ_EMPTY);
    CHECK_SEQFILE_READ("empty.fastq", -3, CHECK_SEQ_EMPTY);
    CHECK_SEQFILE_READ("bad_noqual.fastq", -6, CHECK_SEQ_EMPTY);
    CHECK_SEQFILE_READ("bad_noqualhdrchr.fastq", -5, CHECK_SEQ_EMPTY);
    CHECK_SEQFILE_READ("bad_noqualhdreol.fastq", -5, CHECK_SEQ_EMPTY);
    CHECK_SEQFILE_READ("bad_diff_lens.fastq", -7, CHECK_SEQ_EMPTY);
    /* Check with bad params that it returns -2 */
    res = qes_seqfile_read(NULL, seq);
    tt_int_op(res, ==, -2);
    res = qes_seqfile_read(sf, NULL);
    tt_int_op(res, ==, -2);
end:
    qes_seqfile_destroy(sf);
    qes_seq_destroy(seq);
    if (fname != NULL) {
        free(fname);
    }
#undef CHECK_SEQ_EMPTY
#undef CHECK_SEQFILE_READ
}


static void
test_qes_seqfile_read_vs_kseq (void *ptr)
{
    struct qes_seq *seq = qes_seq_create();
#ifdef ZLIB_FOUND
    char *fname = find_data_file("test.fastq.gz");
    gzFile fp = gzopen(fname, "r");
#else
    char *fname = find_data_file("test.fastq");
    int fp = open(fname, O_RDONLY);
#endif
    struct qes_seqfile *sf = qes_seqfile_create(fname, "r");
    kseq_t *kseq = kseq_init(fp);
    ssize_t kseq_res = 0;
    ssize_t my_res = 0;

    (void) ptr;
    tt_assert(fname != NULL);
    while (1) {
        my_res = qes_seqfile_read(sf, seq);
        kseq_res = kseq_read(kseq);
        tt_int_op(my_res, ==, kseq_res);
        if (my_res < 1 || kseq_res < 1) {
            /* EOF or error */
            break;
        }
        tt_str_op(seq->name.str, ==, kseq->name.s);
        tt_str_op(seq->comment.str, ==, kseq->comment.s);
        tt_str_op(seq->seq.str, ==, kseq->seq.s);
        tt_str_op(seq->qual.str, ==, kseq->qual.s);
    }
    my_res = qes_seqfile_read(sf, seq);
    kseq_res = kseq_read(kseq);
    tt_int_op(my_res, ==, kseq_res);
    tt_int_op(my_res, ==, EOF);
    qes_seqfile_destroy(sf);
    qes_seq_destroy(seq);
    kseq_destroy(kseq);
    kseq = NULL;
#ifdef ZLIB_FOUND
    gzclose(fp);
#else
    close(fp);
#endif
    free(fname);
    /* Try again, with fasta */
    seq = qes_seq_create();
    fname = find_data_file("test_large.fasta.gz");
    tt_assert(fname != NULL);
    sf = qes_seqfile_create(fname, "r");
#ifdef ZLIB_FOUND
    fp = gzopen(fname, "r");
#else
    fp = open(fname, O_RDONLY);
#endif
    kseq = kseq_init(fp);
    while (1) {
        my_res = qes_seqfile_read(sf, seq);
        kseq_res = kseq_read(kseq);
        tt_int_op(my_res, ==, kseq_res);
        if (my_res < 1 || kseq_res < 1) {
            /* EOF or error */
            break;
        }
        tt_str_op(seq->name.str, ==, kseq->name.s);
        tt_str_op(seq->comment.str, ==, kseq->comment.s);
        tt_str_op(seq->seq.str, ==, kseq->seq.s);
    }
end:
    qes_seqfile_destroy(sf);
    qes_seq_destroy(seq);
    if (kseq != NULL) kseq_destroy(kseq);
    if (fname != NULL) free(fname);
#ifdef ZLIB_FOUND
    gzclose(fp);
#else
    close(fp);
#endif
}


/*===  FUNCTION  ============================================================*
Name:           test_qes_seqfile_write
Description:    Tests the qes_seqfile_write function from qes_seqfile.c
 *===========================================================================*/
static void
test_qes_seqfile_write (void *ptr)
{
    struct qes_seq *seq = qes_seq_create();
    size_t expt_bytes = 0;
    ssize_t res = 0;
    struct qes_seqfile *sf = NULL;
    char *fname = NULL;
    char *crc = NULL;

    (void) ptr;
    /* Make a seq to write */
    qes_seq_fill_name(seq, "HWI-TEST", 8);
    qes_seq_fill_comment(seq, "testseq 1 2 3", 13);
    qes_seq_fill_seq(seq, "ACTCAATT", 8);
    qes_seq_fill_qual(seq, "IIIIIIII", 8);
    expt_bytes = 1 + 8 + 1 + 13 + 1 +   /* @ + name + ' ' + comment + '\n' */
                 8 + 1 + 2 + 8 + 1;     /* seq + '\n' + "+\n" + qual + '\n' */
    /* Test with a FASTQ seqfile */
    fname = get_writable_file();
    tt_assert(fname != NULL);
    sf = qes_seqfile_create(fname, "wT");
    qes_seqfile_set_format(sf, FASTQ_FMT);
    res = qes_seqfile_write(sf, seq);
    tt_int_op(res, ==, expt_bytes);
    qes_seqfile_destroy(sf); /* Has to happen here to flush it */
    crc = crc32_file(fname);
    tt_str_op(crc, ==, "d4665941");
    clean_writable_file(fname);
    free(crc);
    fname = NULL;
    crc = NULL;
    /* Test with a FASTA seqfile */
    expt_bytes = 1 + 8 + 1 + 13 + 1 +   /* @ + name + ' ' + comment + '\n' */
                 8 + 1;                 /* seq + '\n'*/
    fname = get_writable_file();
    tt_assert(fname != NULL);
    sf = qes_seqfile_create(fname, "wT");
    qes_seqfile_set_format(sf, FASTA_FMT);
    /* do the write */
    res = qes_seqfile_write(sf, seq);
    tt_int_op(res, ==, expt_bytes);
    qes_seqfile_destroy(sf); /* Flush it */
    crc = crc32_file(fname);
    tt_str_op(crc, ==, "0a295c77");
    clean_writable_file(fname);
    fname = NULL;
    /* Check with bad params that it returns -2 */
    res = qes_seqfile_write(NULL, seq);
    tt_int_op(res, ==, -2);
    res = qes_seqfile_write(sf, NULL);
    tt_int_op(res, ==, -2);
end:
    qes_seqfile_destroy(sf);
    qes_seq_destroy(seq);
    if (fname != NULL) free(fname);
    if (crc != NULL) free(crc);
}


struct testcase_t qes_seqfile_tests[] = {
    { "qes_seqfile_create", test_qes_seqfile_create, 0, NULL, NULL},
    { "qes_seqfile_guess_format", test_qes_seqfile_guess_format, 0, NULL, NULL},
    { "qes_seqfile_destroy", test_qes_seqfile_destroy, 0, NULL, NULL},
    { "qes_seqfile_read_vs_kseq", test_qes_seqfile_read_vs_kseq, 0, NULL, NULL},
    { "qes_seqfile_read", test_qes_seqfile_read, 0, NULL, NULL},
    { "qes_seqfile_write", test_qes_seqfile_write, 0, NULL, NULL},
    END_OF_TESTCASES
};
