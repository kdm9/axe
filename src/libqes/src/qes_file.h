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
 *       Filename:  qes_file.h
 *
 *    Description:  Compressed file IO
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#ifndef QES_FILE_H
#define QES_FILE_H

#include <qes_util.h>
#include <qes_str.h>

enum qes_file_mode {
    QES_FILE_MODE_UNKNOWN,
    QES_FILE_MODE_READ,
    QES_FILE_MODE_WRITE
};

struct qes_file {
    QES_ZTYPE fp;
    char *path;
    char *buffer;
    char *bufiter;
    char *bufend;
    off_t filepos;
    enum qes_file_mode mode;
    /* Is the fp at EOF, AND do we have nothing left to copy from the buffer */
    int eof;
    /* Is the fp at EOF */
    int feof;
};

/* qes_file_open:
    Create a `struct qes_file` and open `path` with mode `mode` and
    errorhandler `onerr`
 */
struct qes_file *qes_file_open_(const char             *path,
                                const char             *mode,
                                qes_errhandler_func     onerr,
                                const char             *file,
                                int                     line);
#define qes_file_open(pth, mod)                                             \
    qes_file_open_(pth, mod, QES_DEFAULT_ERR_FN, __FILE__, __LINE__)
#define qes_file_open_errnil(pth, mod)                                      \
    qes_file_open_(pth, mod, errnil, __FILE__, __LINE__)
#define qes_file_open_errprint(pth, mod)                                    \
    qes_file_open_(pth, mod, errprint, __FILE__, __LINE__)
#define qes_file_open_errprintexit(pth, mod)                                \
    qes_file_open_(pth, mod, errprintexit, __FILE__, __LINE__)


/*===  FUNCTION  ============================================================*
Name:           qes_file_close
Parameters:     struct qes_file *file: file to close.
Description:    Closes the file pointer in ``file``, frees dynamically
                allocated members of ``file`` and sets ``file`` to NULL.
Returns:        void
 *===========================================================================*/
void qes_file_close_           (struct qes_file        *file);
#define qes_file_close(file) do {                                           \
            qes_file_close_ (file);                                         \
            file = NULL;                                                    \
        } while(0)

int qes_file_readable          (struct qes_file        *file);
int qes_file_writable          (struct qes_file        *file);
enum qes_file_mode qes_file_guess_mode
                               (const char             *mode);

void qes_file_rewind           (struct qes_file        *file);
int qes_file_peek              (struct qes_file        *file);

int qes_file_putstr            (struct qes_file        *stream,
                                const struct qes_str   *str);
int qes_file_puts              (struct qes_file        *file,
                                const char             *str);
int qes_file_putc              (struct qes_file        *stream,
                                const int               chr);
int qes_file_getc              (struct qes_file        *file);


/*===  FUNCTION  ============================================================*
Name:           qes_file_readline
Parameters:     struct qes_file *file: File to read
                char *dest: Destination buffer
                size_t maxlen: size of destination buffer
Description:    Reads at most ``maxlen - 1`` bytes of the next '\n' delimited
                line into ``dest``, and null- terminates ``dest``. The '\n' is
                copied, and therefore counts towards ``strlen()`` of ``dest``.
Returns:        ssize_t: EOF, -2 (error), or length of bytes copied, i.e.
                ``strlen(dest);``
 *===========================================================================*/
ssize_t qes_file_readline      (struct qes_file        *file,
                                char                   *dest,
                                size_t                  maxlen);

/*===  FUNCTION  ============================================================*
Name:           qes_file_readline_str
Parameters:     struct qes_file *file: File to read.
                struct qes_str *str: struct qes_str object to read into.
Description:    Convenience wrapper around qes_file_readline_realloc, which
                reads a line into a struct qes_str object, passing str->str to
                and str->capacity to qes_file_readline_realloc.
Returns:        ssize_t set to either the length of the line copied to the
                struct qes_str, or one of -1 (EOF) or -2 (error).
* ===========================================================================*/
ssize_t qes_file_readline_str  (struct qes_file        *file,
                                struct qes_str         *str);

/*===  FUNCTION  ============================================================*
Name:           qes_file_getuntil
Parameters:     struct qes_file *file: File to read
                const int delim: Delimiter char.
Description:    Reads ``file`` into ``dest`` until ``delim`` is found or
                ``maxlen - `` bytes have been read. ``delim`` is copied into
                ``dest``! ``delim`` can be EOF for "read until EOF", or any
                other thing that fits in a ``char``.
Returns:        ssize_t: EOF, -2 (error) or size of data read.
 *===========================================================================*/
ssize_t qes_file_getuntil      (struct qes_file        *file,
                                const int               delim,
                                char                   *dest,
                                size_t                  maxlen);

/*===  FUNCTION  ============================================================*
Name:           qes_file_getuntil_realloc
Parameters:     qes_file *file: File to read.
                int delim: Delimiter char.
                char **bufref: reference to a `char *` containing the buffer.
                    Must not refer to a ``char[]`` that cannot be resized with
                    ``realloc``.
                size *sizeref: Reference to a variable tracking the allocated
                    size of the ``char *`` referred to by ``bufref``.
Description:    Read a string from `file` into a
                `char *` pointed to by
                `bufref` up to and inclding the character ``delim``. This
                function has the added benefit of `realloc`-ing `*bufref` to
                the next highest base-2 power, if we run out of space.  If it
                is realloced, `(*sizeref)` is updated to the new buffer size.
Returns:        ssize_t set to either the length of the line copied to
                `*bufref`, or one of -1 (EOF) or -2 (error).
*============================================================================*/
ssize_t qes_file_getuntil_realloc_
                               (struct qes_file        *file,
                                int                     delim,
                                char                  **bufref,
                                size_t                 *sizeref,
                                qes_errhandler_func     onerr,
                                const char             *src,
                                const int               line);

#define qes_file_getuntil_realloc(fp, dlm, buf, sz)                         \
    qes_file_getuntil_realloc_(fp, dlm, buf, sz, QES_DEFAULT_ERR_FN,        \
                               __FILE__, __LINE__)
#define qes_file_getuntil_realloc_errnil(fp, dlm, buf, sz)                  \
    qes_file_getuntil_realloc_(fp, dlm, buf, sz, errnil, __FILE__, __LINE__)
#define qes_file_getuntil_realloc_errprint(fp, dlm, buf, sz)                \
    qes_file_getuntil_realloc_(fp, dlm, buf, sz, errprint, __FILE__, __LINE__)
#define qes_file_getuntil_realloc_errprintexit(fp, dlm, buf, sz)            \
    qes_file_getuntil_realloc_(fp, dlm, buf, sz, errprintexit, __FILE__,    \
                               __LINE__)


/* ===  FUNCTION  =============================================================
          Name: qes_file_readline_realloc
   Description: Read a line from `file` into a `char *` pointed to by `buf`.
                This function has the added benefit of `realloc`-ing `buf`
                to the next highest base-2 power, if we run out of space.
                If it is realloced, `(*size)` is updated to the new buffer
                size. DON'T USE ON STACK BUFFERS.
       Returns: ssize_t set to either the length of the line copied to `*buf`,
                or one of -1 (EOF) or -2 (error).
 * ==========================================================================*/
ssize_t qes_file_readline_realloc_
                               (struct qes_file        *file,
                                char                  **buf,
                                size_t                 *size,
                                qes_errhandler_func     onerr,
                                const char             *src,
                                const int               line);
#define qes_file_readline_realloc(fp, buf, sz)                              \
    qes_file_readline_realloc_(fp, buf, sz, QES_DEFAULT_ERR_FN, __FILE__,   \
                               __LINE__)
#define qes_file_readline_realloc_errnil(fp, buf, sz)                       \
    qes_file_readline_realloc_(fp, buf, sz, errnil, __FILE__, __LINE__)
#define qes_file_readline_realloc_errprint(fp, buf, sz)                     \
    qes_file_readline_realloc_(fp, buf, sz, errprint, __FILE__, __LINE__)
#define qes_file_readline_realloc_errprintexit(fp, buf, sz)                 \
    qes_file_readline_realloc_(fp, buf, sz, errprintexit, __FILE__, __LINE__)


const char *qes_file_error     (struct qes_file        *file);

static inline int
qes_file_ok                    (const struct qes_file  *qf)
{
    /* qes_file_ok just check we won't dereference NULLs, so we check pointer
     * NULLness for all pointers we care about in current modes. Which, unless
     * we're Write-only, is all of them */
    return  qf != NULL && \
            qf->fp != NULL && \
            qf->bufiter != NULL && \
            qf->buffer != NULL;
}


#endif /* QES_FILE_H */
