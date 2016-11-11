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
 *       Filename:  qes_seqfile.h
 *
 *    Description:  qes_seqfile -- read sequences in FASTA or FASTQ format.
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#ifndef QES_SEQFILE_H
#define QES_SEQFILE_H

#include <qes_util.h>
#include <qes_seq.h>
#include <qes_file.h>


/*--------------------------------------------------------------------------
  |       Seqfile -- seamless reading & writing of FASTA & FASTQ           |
  ---------------------------------------------------------------------------*/

#define    FASTA_DELIM '>'
#define    FASTQ_DELIM '@'
#define    FASTQ_QUAL_DELIM '+'

enum qes_seqfile_format {
    UNKNOWN_FMT = 0,
    FASTA_FMT = 1,
    FASTQ_FMT = 2,
};

struct qes_seqfile {
    struct qes_file *qf;
    size_t n_records;
    enum qes_seqfile_format format;
    /* A buffer to store misc shit in while reading.
       One per file to keep it re-entrant */
    struct qes_str scratch;
};


/*===  FUNCTION  ============================================================*
Name:           qes_seqfile_create
Parameters:     const char *path: Path to open.
                const char *mode: Mode to pass to the fopen equivalent used.
Description:    Allocates structures, initialises values and opens the internal
                file handle.
Returns:        A fully usable ``struct qes_seqfile *`` or NULL.
 *===========================================================================*/
struct qes_seqfile *qes_seqfile_create (const char *path, const char *mode);


/*===  FUNCTION  ============================================================*
Name:           qes_seqfile_ok
Parameters:     const struct qes_seqfile *file: File reference to check.
Description:    Check a file reference ``file``, ensuring that it may be
                dereferenced and used. This checks if a buffer has been
                allocated and buffer pointers set appropriately IFF the file is
                opened for reading. No guarantees are given about being able to
                read from the file.
Returns:        An int evaluating to true if ``file`` is OK, otherwise false.
 *===========================================================================*/
static inline int
qes_seqfile_ok(const struct qes_seqfile *file)
{
    return (file != NULL && qes_file_ok(file->qf));
}



/*===  FUNCTION  ============================================================*
Name:           qes_seqfile_guess_format
Parameters:     struct qes_seqfile *file: File whose format we guess.
Description:    Guess, using qes_file_peek, the format of ``file``. This only inspects
                the first character, so it will be confused if the first
                character of the file is incorrect. Only use this at the start
                of a file (this is checked).
Returns:        The file's format as a enum qes_seqfile_format, or UNKNOWN_FMT if the
                file is not readable, or not a FASTA/FASTQ file.
 *===========================================================================*/
enum qes_seqfile_format qes_seqfile_guess_format(struct qes_seqfile *file);

void qes_seqfile_set_format (struct qes_seqfile *file,
                             enum qes_seqfile_format format);

ssize_t qes_seqfile_read (struct qes_seqfile *file, struct qes_seq *seq);

ssize_t qes_seqfile_write (struct qes_seqfile *file, struct qes_seq *seq);

size_t qes_seqfile_format_seq(const struct qes_seq *seq, enum qes_seqfile_format fmt,
        char *buffer, size_t maxlen);

void qes_seqfile_destroy_(struct qes_seqfile *seqfile);
#define qes_seqfile_destroy(seqfile) do {                                   \
            qes_seqfile_destroy_(seqfile);                                  \
            seqfile = NULL;                                                 \
        } while(0)

#ifdef OPENMP_FOUND
#define QES_SEQFILE_ITER_PARALLEL_SINGLE_BEGIN(fle, sq, ln, opts)           \
    _Pragma(STRINGIFY(omp parallel shared(fle) opts default(none)))         \
    {                                                                       \
        struct qes_seq *sq = qes_seq_create();                              \
        ssize_t ln = 0;                                                     \
        while(1) {                                                          \
            _Pragma(STRINGIFY(omp critical))                                \
            {                                                               \
                ln = qes_seqfile_read(fle, sq);                             \
            }                                                               \
            if (ln < 0) {                                                   \
                break;                                                      \
            }

#define QES_SEQFILE_ITER_PARALLEL_SINGLE_END(sq)                            \
        }                                                                   \
        qes_seq_destroy(sq);                                                \
    }

#define QES_SEQFILE_ITER_PARALLEL_PAIRED_BEGIN(fle1, fle2, sq1, sq2, ln1, ln2, opts)\
    _Pragma(STRINGIFY(omp parallel shared(fle1, fle2) opts default(none)))  \
    {                                                                       \
        struct qes_seq *sq1 = qes_seq_create();                             \
        struct qes_seq *sq2 = qes_seq_create();                             \
        ssize_t ln1 = 0;                                                    \
        ssize_t ln2 = 0;                                                    \
        while(1) {                                                          \
            _Pragma(STRINGIFY(omp critical))                                \
            {                                                               \
                ln1 = qes_seqfile_read(fle1, sq1);                          \
                ln2 = qes_seqfile_read(fle2, sq2);                          \
            }                                                               \
            if (ln1 < 0 || ln2 < 0) {                                       \
                break;                                                      \
            }

#define QES_SEQFILE_ITER_PARALLEL_PAIRED_END(sq1, sq2)                      \
        }                                                                   \
        qes_seq_destroy(sq1);                                               \
        qes_seq_destroy(sq2);                                               \
    }

#define QES_SEQFILE_ITER_PARALLEL_INTERLEAVED_BEGIN(fle, sq1, sq2, ln1, ln2, opts)\
    _Pragma(STRINGIFY(omp parallel shared(fle) opts default(none)))         \
    {                                                                       \
        struct qes_seq *sq1 = qes_seq_create();                             \
        struct qes_seq *sq2 = qes_seq_create();                             \
        ssize_t ln1 = 0;                                                    \
        ssize_t ln2 = 0;                                                    \
        while(1) {                                                          \
            _Pragma(STRINGIFY(omp critical))                                \
            {                                                               \
                ln1 = qes_seqfile_read(fle, sq1);                           \
                ln2 = qes_seqfile_read(fle, sq2);                           \
            }                                                               \
            if (ln1 < 0 || ln2 < 0) {                                       \
                break;                                                      \
            }

#define QES_SEQFILE_ITER_PARALLEL_INTERLEAVED_END(sq1, sq2)                 \
        }                                                                   \
        qes_seq_destroy(sq1);                                               \
        qes_seq_destroy(sq2);                                               \
    }

#endif /* OPENMP_FOUND */

#define QES_SEQFILE_ITER_SINGLE_BEGIN(fle, sq, ln)                          \
    {                                                                       \
        struct qes_seq *sq = qes_seq_create();                              \
        /* TODO MUSTFIX check for null sq */                                \
        ssize_t ln = 0;                                                     \
        while(1) {                                                          \
            ln = qes_seqfile_read(fle, sq);                                 \
            if (ln < 0) {                                                   \
                break;                                                      \
            }

#define QES_SEQFILE_ITER_SINGLE_END(sq)                                     \
        }                                                                   \
        qes_seq_destroy(sq);                                                \
    }

#define QES_SEQFILE_ITER_PAIRED_BEGIN(fle1, fle2, sq1, sq2, ln1, ln2)       \
    {                                                                       \
        struct qes_seq *sq1 = qes_seq_create();                             \
        struct qes_seq *sq2 = qes_seq_create();                             \
        ssize_t ln1 = 0;                                                    \
        ssize_t ln2 = 0;                                                    \
        while(1) {                                                          \
            ln1 = qes_seqfile_read(fle1, sq1);                              \
            ln2 = qes_seqfile_read(fle2, sq2);                              \
            if (ln1 < 0 || ln2 < 0) {                                       \
                break;                                                      \
            }

#define QES_SEQFILE_ITER_PAIRED_END(sq1, sq2)                               \
        }                                                                   \
        qes_seq_destroy(sq1);                                               \
        qes_seq_destroy(sq2);                                               \
    }

#define QES_SEQFILE_ITER_INTERLEAVED_BEGIN(fle, sq1, sq2, ln1, ln2)         \
    {                                                                       \
        struct qes_seq *sq1 = qes_seq_create();                             \
        struct qes_seq *sq2 = qes_seq_create();                             \
        ssize_t ln1 = 0;                                                    \
        ssize_t ln2 = 0;                                                    \
        while(1) {                                                          \
            ln1 = qes_seqfile_read(fle, sq1);                               \
            ln2 = qes_seqfile_read(fle, sq2);                               \
            if (ln1 < 0 || ln2 < 0) {                                       \
                break;                                                      \
            }

#define QES_SEQFILE_ITER_INTERLEAVED_END(sq1, sq2)                          \
        }                                                                   \
        qes_seq_destroy(sq1);                                               \
        qes_seq_destroy(sq2);                                               \
    }

#endif /* QES_SEQFILE_H */
