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
 *       Filename:  test_file.c
 *
 *    Description:  Test qes_file
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "tests.h"
#include <qes_file.h>


static void
test_qes_file_open (void *ptr)
{
    struct qes_file *file = NULL;
    struct qes_file *badfile = NULL;
    char *fname = NULL;

    (void) ptr;
    /* Test file opening for reading */
    fname = find_data_file("loremipsum.txt");
    tt_assert(fname != NULL);
    file = qes_file_open(fname, "r");
    tt_ptr_op(file, !=, NULL);
    tt_int_op(file->mode, ==, QES_FILE_MODE_READ);
    qes_file_close(file);
    free(fname);
    /* test zipped file opening for reading */
    fname = find_data_file("loremipsum.txt.gz");
    tt_assert(fname != NULL);
    file = qes_file_open(fname, "r");
    tt_ptr_op(file, !=, NULL);
    tt_int_op(file->mode, ==, QES_FILE_MODE_READ);
    qes_file_close(file);
    free(fname);
    /* read with non-existant file */
    fname = get_writable_file();
    tt_assert(fname != NULL);
    file = qes_file_open(fname, "r");
    tt_ptr_op(file, ==, NULL);
    clean_writable_file(fname);
    fname = NULL;
    /* writing with gziped file */
    fname = get_writable_file();
    tt_assert(fname != NULL);
    file = qes_file_open(fname, "w");
    tt_ptr_op(file, !=, NULL);
    tt_int_op(file->mode, ==, QES_FILE_MODE_WRITE);
    clean_writable_file(fname);
    fname = NULL;
    /* With non-existant file path */
    badfile = qes_file_open("non/existent.file", "w");
    tt_ptr_op(badfile, ==, NULL);

end:
    qes_file_close(file);
    qes_file_close(badfile);
    if (fname != NULL) free(fname);
}

static void
test_qes_file_close (void *ptr)
{
    struct qes_file *file = NULL;
    struct qes_file *nullfile = NULL;
    char *fname = NULL;

    (void) ptr;
    /* Open file */
    fname = find_data_file("loremipsum.txt");
    tt_assert(fname != NULL);
    file = qes_file_open(fname, "r");
    tt_assert(file);
    qes_file_close(file);
    tt_ptr_op(file, ==, NULL);
    /* check with null poitner, ensure no problems are caused. */
    qes_file_close(nullfile);
    tt_ptr_op(nullfile, ==, NULL);
end:
    qes_file_close(file);
    free(fname);
}


static void
test_qes_file_rewind (void *ptr)
{
    struct qes_file *file = NULL;
    size_t bufsize = 1<<10;
    char buffer[bufsize];
    ssize_t res = 0;
    char *fname = NULL;

    (void) ptr;
    /* Open file */
    fname = find_data_file("loremipsum.txt");
    tt_assert(fname != NULL);
    file = qes_file_open(fname, "r");
    tt_assert(file);
    while (res != EOF) {
        res = qes_file_readline(file, buffer, bufsize);
    }
    tt_int_op(file->filepos, ==, loremipsum_fsize);
    tt_int_op(QES_ZTELL(file->fp), ==, loremipsum_fsize);
    tt_assert(file->eof);
    tt_assert(file->feof);
    qes_file_rewind(file);
    tt_int_op(file->filepos, ==, 0);
    tt_assert(!file->eof);
    tt_assert(!file->feof);
    tt_int_op(QES_ZTELL(file->fp), ==, 0);
end:
    qes_file_close(file);
    free(fname);
}

static void
test_qes_file_readline (void *ptr)
{
    struct qes_file *file = NULL;
    size_t bufsize = 1<<10;
    char buffer[bufsize];
    ssize_t res_len = 0;
    off_t orig_filepos = 0;
    size_t iii;
    char *fname = NULL;

    (void) ptr;
    /* Open file */
    fname = find_data_file("loremipsum.txt");
    tt_assert(fname != NULL);
    file = qes_file_open(fname, "r");
    /* Check each line is of the right length, that the length is returned,
     * that the string is as expected, and that file->filepos is updated.
     */
    for (iii = 0; iii < n_loremipsum_lines; iii++) {
        orig_filepos = file->filepos;
        res_len = qes_file_readline(file, buffer, bufsize);
        tt_int_op(res_len, ==, strlen(buffer));
        tt_int_op(res_len, ==, loremipsum_line_lens[iii]);
        tt_str_op(buffer, ==, loremipsum_lines[iii]);
        tt_int_op(file->filepos - orig_filepos, ==, loremipsum_line_lens[iii]);
    }
    /* Check that a file at EOF returns EOF. */
    tt_int_op(file->filepos, ==, loremipsum_fsize);
    tt_int_op(qes_file_readline(file, buffer, bufsize), ==, EOF);
    tt_assert(file->eof)
    /* Test with bad parameters */
    tt_int_op(qes_file_readline(NULL, buffer, bufsize), ==, -2);
    tt_int_op(qes_file_readline(file, NULL, bufsize), ==, -2);
    tt_int_op(qes_file_readline(file, buffer, 0), ==, -2);

end:
    qes_file_close(file);
    free(fname);
}

static void
test_qes_file_getuntil (void *ptr)
{
    struct qes_file *file = NULL;
    const size_t bufsize = 1<<10;
    char buffer[bufsize];
    ssize_t res_len = 0;
    size_t expt_len = 0;
    off_t orig_filepos = 0;
    off_t our_filepos = 0;
    size_t iii;
    const size_t n_delims = 5;
    const int delims[] = {' ', ',', '.', '\n', '\n'};
    const char *delim_words[] = {
        "Lorem ",
        "ipsum dolor sit amet,",
        " consectetur adipiscing elit.",
        " Donec ornare tortor et\n",
        "rhoncus iaculis. Sed suscipit, arcu nec elementum vestibulum, tortor tortor\n",
    };
    char *fname = NULL;

    (void) ptr;
    /* Open file */
    fname = find_data_file("loremipsum.txt");
    tt_assert(fname != NULL);
    file = qes_file_open(fname, "r");
    /* Check each token is of the right length, that the length is returned,
     * that the string is as expected, and that file->filepos is updated.
     */
    for (iii = 0; iii < n_delims; iii++) {
        orig_filepos = file->filepos;
        res_len = qes_file_getuntil(file, delims[iii], buffer, bufsize);
        our_filepos += res_len;
        expt_len = strnlen(delim_words[iii], bufsize);
        tt_int_op(res_len, ==, strnlen(buffer, bufsize));
        tt_int_op(res_len, ==, expt_len);
        tt_str_op(buffer, ==, delim_words[iii]);
        tt_int_op(file->filepos - orig_filepos, ==, expt_len);
        tt_int_op(file->filepos, ==, our_filepos);
    }
    /* Check we can give EOF as the char and make it give us the remainder of
       the file */
    orig_filepos = file->filepos;
    res_len = qes_file_getuntil(file, EOF, buffer, bufsize);
    expt_len = loremipsum_fsize - our_filepos;
    our_filepos += res_len;
    tt_int_op(res_len, ==, strnlen(buffer, bufsize));
    tt_int_op(res_len, ==, expt_len);
    tt_int_op(file->filepos - orig_filepos, ==, expt_len);
    tt_int_op(file->filepos, ==, our_filepos);
    tt_assert(file->eof)
    tt_int_op(file->filepos, ==, loremipsum_fsize);
    /* Check that a file at EOF returns EOF. */
    tt_int_op(file->filepos, ==, loremipsum_fsize);
    tt_int_op(qes_file_getuntil(file, '\n', buffer, bufsize), ==, EOF);
    tt_assert(file->eof)
    /* Test with bad parameters */
    QES_ZREWIND(file->fp);
    file->eof = 0;
    file->filepos = 0;
    tt_int_op(qes_file_getuntil(NULL, '\n', buffer, bufsize), ==, -2);
    tt_int_op(qes_file_getuntil(file, 256, buffer, bufsize), ==, -2);
    tt_int_op(qes_file_getuntil(file, '\n', NULL, bufsize), ==, -2);
    tt_int_op(qes_file_getuntil(file, '\n', buffer, 0), ==, -2);
end:
    qes_file_close(file);
    if (fname != NULL) free(fname);
}

static void
test_qes_file_peek (void *ptr)
{
    int res = 0;
    struct qes_file *file = NULL;
    off_t orig_filepos = 0;
    char *fname = NULL;

    (void) ptr;
    /* Open file and save pos */
    fname = find_data_file("loremipsum.txt");
    tt_assert(fname != NULL);
    file = qes_file_open(fname, "r");
    orig_filepos = file->filepos;
    /* Peek a char */
    res = qes_file_peek(file);
    /* Check it's the right char */
    tt_int_op(res, ==, loremipsum_lines[0][0]);
    /* And that the filepos hasn't changed */
    tt_int_op(file->filepos , ==, orig_filepos);
    /* And that the same char is returned again */
    res = qes_file_peek(file);
    tt_int_op(res, ==, loremipsum_lines[0][0]);
    /* And that it returns an error on being given a null pointer  */
    tt_int_op(qes_file_peek(NULL), ==, -2);
end:
    qes_file_close(file);
    if (fname != NULL) free(fname);
}

static void
test_qes_file_guess_mode (void *ptr)
{
    const char *modes[] = {
        "r", "rb", "rb8", "rT", "rbT",
        "w", "wb", "wb8", "wT", "wbT",
        "a", "ab", "ab8", "aT", "abT",
        "+", "+b", "+b8", "+T", "+bT",
    };
    const int mode_results[] = {
        QES_FILE_MODE_READ, QES_FILE_MODE_READ, QES_FILE_MODE_READ,
        QES_FILE_MODE_READ, QES_FILE_MODE_READ, QES_FILE_MODE_WRITE,
        QES_FILE_MODE_WRITE, QES_FILE_MODE_WRITE, QES_FILE_MODE_WRITE,
        QES_FILE_MODE_WRITE, QES_FILE_MODE_WRITE, QES_FILE_MODE_WRITE,
        QES_FILE_MODE_WRITE, QES_FILE_MODE_WRITE, QES_FILE_MODE_WRITE,
        QES_FILE_MODE_UNKNOWN, QES_FILE_MODE_UNKNOWN, QES_FILE_MODE_UNKNOWN,
        QES_FILE_MODE_UNKNOWN, QES_FILE_MODE_UNKNOWN,
    };
    const size_t num_modes = 20;
    size_t iii;
    (void) ptr;
    for (iii = 0; iii < num_modes; iii++) {
        tt_int_op(qes_file_guess_mode(modes[iii]), ==, mode_results[iii]);
    }
end:
    ;
}

static void
test_qes_file_readline_realloc (void *ptr)
{
    char *buf = NULL;
    char *smallbuf = NULL;
    const size_t smallbuf_len = 4;
    const size_t buf_len = 1<<10; /* 1024b */
    struct qes_file *file = NULL;
    ssize_t ret = 0;
    off_t fpos = 0;
    size_t line_num;
    char *nulcp = NULL;
    size_t tmpsz = buf_len;
    char *fname = NULL;

    (void) ptr;
    /* Open file and save pos */
    fname = find_data_file("loremipsum.txt");
    /* This should always work, so long as you run it from the right dir */
    file = qes_file_open(fname, "r");
    buf = calloc(buf_len, sizeof(*buf));
    smallbuf = calloc(smallbuf_len, sizeof(*smallbuf));
    tt_assert(file && buf && smallbuf);
    /* Check each line is of the right length, that the length is returned,
     * that the string is as expected, and that file->filepos is updated.
     */
    for (line_num = 0; line_num < n_loremipsum_lines; line_num++) {
        ret = qes_file_readline_realloc(file, &buf, &tmpsz);
        fpos += ret;
        tt_int_op(fpos, ==, file->filepos);
        tt_str_op(buf, ==, loremipsum_lines[line_num]);
        tt_int_op(strlen(buf), ==, loremipsum_line_lens[line_num]);
        tt_int_op(ret, ==, loremipsum_line_lens[line_num]);
        tt_int_op(buf[ret], ==, '\0');
        tt_int_op(tmpsz, ==, buf_len);
        tt_int_op(tmpsz, >=, qes_roundupz(loremipsum_line_lens[line_num]));
    }
    tt_int_op(file->filepos, ==, loremipsum_fsize);
    ret = qes_file_readline_realloc(file, &buf, &tmpsz);
    tt_assert(file->eof);
    tt_int_op(ret, ==, EOF);
    qes_file_close(file);
    /*
     *                  Test w/ small buffer
     */
    /* Do the same checks, but with a buffer that needs resizing */
    file = qes_file_open(fname, "r");
    tmpsz = smallbuf_len;
    fpos = 0;
    for (line_num = 0; line_num < n_loremipsum_lines; line_num++) {
        ret = qes_file_readline_realloc(file, &smallbuf, &tmpsz);
        fpos += ret;
        tt_int_op(fpos, ==, file->filepos);
        tt_str_op(smallbuf, ==, loremipsum_lines[line_num]);
        tt_int_op(strlen(smallbuf), ==, loremipsum_line_lens[line_num]);
        tt_int_op(ret, ==, loremipsum_line_lens[line_num]);
        tt_int_op(tmpsz, !=, smallbuf_len);
        tt_int_op(tmpsz, >=, qes_roundupz(loremipsum_line_lens[line_num]));
        tt_int_op(smallbuf[ret], ==, '\0');
    }
    tt_int_op(file->filepos, ==, loremipsum_fsize);
    /* Test with EOF file */
    tmpsz = buf_len;
    ret = qes_file_readline_realloc(file, &buf, &tmpsz);
    tt_int_op(ret, ==, EOF);
    tt_str_op(buf, ==,  "");
    tt_int_op(strlen(buf), ==, 0);
    tt_int_op(tmpsz, ==, buf_len);
    tt_assert(file->eof)
    qes_file_close(file);
    /*
     *                     Test bad things
     */
    /* Null buf. Should alloc a buffer and fill it */
    file = qes_file_open(fname, "r");
    line_num = 0;
    ret = qes_file_readline_realloc(file, &nulcp, &tmpsz);
    tt_int_op(ret, ==, loremipsum_line_lens[line_num]);
    tt_str_op(nulcp, ==, loremipsum_lines[line_num]);
    tt_int_op(strlen(nulcp), ==, loremipsum_line_lens[line_num]);
    tt_int_op(tmpsz, ==, __INIT_LINE_LEN);
    /* Null file */
    ret = qes_file_readline_realloc(NULL, &buf, &tmpsz);
    tt_int_op(ret, ==, -2);
    /* This shouldn't change and is set in the prev. test */
    tt_int_op(tmpsz, ==, __INIT_LINE_LEN);
    /* Both buf & file null */
    ret = qes_file_readline_realloc(NULL, &nulcp, &tmpsz);
    tt_int_op(ret, ==, -2);
    tt_int_op(tmpsz, ==, __INIT_LINE_LEN);
end:
    if (buf != NULL) free(buf);
    if (smallbuf != NULL) free(smallbuf);
    if (nulcp != NULL) free(nulcp);
    if (file != NULL) qes_file_close(file);
    free(fname);
}

static void
test_qes_file_ok (void *ptr)
{
    struct qes_file *file;
    char *writeable = NULL;
    char *readable = NULL;

    (void) ptr;
    readable = find_data_file("loremipsum.txt");
    writeable = get_writable_file();
    /* Should result in an OK file */
    file = qes_file_open(readable, "r");
    tt_assert(qes_file_ok(file));
    tt_assert(qes_file_readable(file));
    qes_file_close(file);
    file = qes_file_open("nosuchfile", "r");
    tt_assert(!qes_file_ok(file));
    tt_assert(!qes_file_readable(file));
    qes_file_close(file);
    file = qes_file_open(writeable, "w");
    tt_assert(qes_file_ok(file));
    tt_assert(!qes_file_readable(file));
    qes_file_close(file);
end:
    if (file != NULL) qes_file_close(file);
    clean_writable_file(writeable);
    free(readable);

}

struct testcase_t qes_file_tests[] = {
    { "qes_file_open", test_qes_file_open, 0, NULL, NULL},
    { "qes_file_peek", test_qes_file_peek, 0, NULL, NULL},
    { "qes_file_readline", test_qes_file_readline, 0, NULL, NULL},
    { "qes_file_readline_realloc", test_qes_file_readline_realloc, 0, NULL, NULL},
    { "qes_file_guess_mode", test_qes_file_guess_mode, 0, NULL, NULL},
    { "qes_file_close", test_qes_file_close, 0, NULL, NULL},
    { "qes_file_rewind", test_qes_file_rewind, 0, NULL, NULL},
    { "qes_file_getuntil", test_qes_file_getuntil, 0, NULL, NULL},
    { "qes_file_ok", test_qes_file_ok, 0, NULL, NULL},
    END_OF_TESTCASES
};
