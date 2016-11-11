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
 *       Filename:  qes_seqfile.c
 *
 *    Description:  qes_seqfile -- read sequences in FASTA or FASTQ format.
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "qes_seqfile.h"


static inline ssize_t
read_fastq_seqfile(struct qes_seqfile *seqfile, struct qes_seq *seq)
{
    /* Convenience macro, this happens a lot */
#define CHECK_AND_TRIM(subrec) if (len < 1) { \
            goto error;     \
        } else {        \
            subrec.str[--len] = '\0'; \
            subrec.len = len; \
        }
    ssize_t len = 0;
    int next = '\0';
    int errcode = -1;

    /* Fast-forward past the delimiter '@', ensuring it exists */
    next = qes_file_getc(seqfile->qf);
    if (next == EOF) {
        return EOF;
    } else if (next != FASTQ_DELIM) {
        /* This ain't a fastq! WTF! */
        errcode = -3;
        goto error;
    }
    len = qes_file_readline_str(seqfile->qf, &seqfile->scratch);
    if (len < 1) {
        /* Weird truncated file */
        errcode = -3;
        goto error;
    }
    qes_seq_fill_header(seq, seqfile->scratch.str, seqfile->scratch.len);
    /* Fill the actual sequence directly */
    len = qes_file_readline_str(seqfile->qf, &seq->seq);
    errcode = -4;
    CHECK_AND_TRIM(seq->seq)
    /* read the qual header, but don't store it. */
    errcode = -5;
    next = qes_file_getc(seqfile->qf);
    if (next != FASTQ_QUAL_DELIM) {
        goto error;
    }
    while ((next = qes_file_getc(seqfile->qf)) != '\n') {
        if (next == EOF) {
            goto error;
        }
    }
    if (next != '\n') goto error;
    /* Fill the qual score string directly */
    len = qes_file_readline_str(seqfile->qf, &seq->qual);
    errcode = -6;
    CHECK_AND_TRIM(seq->qual)
    if ((size_t)len != seq->seq.len) {
        /* Error out on different len qual/seq entries */
        errcode = -7;
        goto error;
    }
    /* return seq/qual len */
    seqfile->n_records++;
    return seq->seq.len;
error:
    qes_str_nullify(&seq->name);
    qes_str_nullify(&seq->comment);
    qes_str_nullify(&seq->seq);
    qes_str_nullify(&seq->qual);
    return errcode;
#undef CHECK_AND_TRIM
}

static inline ssize_t
read_fasta_seqfile(struct qes_seqfile *seqfile, struct qes_seq *seq)
{
    /* Convenience macro, this happens a lot */
#define CHECK_AND_TRIM(subrec) if (len < 1) { \
            goto error;     \
        } else {        \
            subrec.str[--len] = '\0'; \
            subrec.len = len; \
        }
    ssize_t len = 0;
    int next = '\0';

    /* This bit is basically a copy-paste from above */
    /* Fast-forward past the delimiter '>', ensuring it exists */
    next = qes_file_getc(seqfile->qf);
    if (next == EOF) {
        return EOF;
    } else if (next != FASTA_DELIM) {
        /* This ain't a fasta! WTF! */
        goto error;
    }
    len = qes_file_readline_str(seqfile->qf, &seqfile->scratch);
    if (len < 1) {
        goto error;
    }
    qes_seq_fill_header(seq, seqfile->scratch.str, seqfile->scratch.len);
    /* we need to nullify seq, as we rely on seq.len being 0 as we enter this
     *  while loop */
    qes_str_nullify(&seq->seq);
    qes_str_nullify(&seqfile->scratch);
    /* While the next char is not a '>', i.e. until next header line */
    while ((next = qes_file_peek(seqfile->qf)) != EOF && next != FASTA_DELIM) {
        len = qes_file_readline_str(seqfile->qf, &seqfile->scratch);
        CHECK_AND_TRIM(seqfile->scratch)
        if (len < 0) {
            goto error;
        }
        qes_str_cat(&seq->seq, &seqfile->scratch);
    }
    seq->seq.str[seq->seq.len] = '\0';
    /* return seq len */
    seqfile->n_records++;
    qes_str_nullify(&seq->qual);
    return seq->seq.len;
error:
    qes_str_nullify(&seq->name);
    qes_str_nullify(&seq->comment);
    qes_str_nullify(&seq->seq);
    qes_str_nullify(&seq->qual);
    return -2;
#undef CHECK_AND_TRIM
}

ssize_t
qes_seqfile_read (struct qes_seqfile *seqfile, struct qes_seq *seq)
{
    if (!qes_seqfile_ok(seqfile) || !qes_seq_ok(seq)) {
        return -2;
    }
    if (seqfile->qf->eof) {
        return EOF;
    }
    if (seqfile->format == FASTQ_FMT) {
        return read_fastq_seqfile(seqfile, seq);
    } else if (seqfile->format == FASTA_FMT) {
        return read_fasta_seqfile(seqfile, seq);
    }
    /* If we reach here, bail out with an error */
    qes_str_nullify(&seq->name);
    qes_str_nullify(&seq->comment);
    qes_str_nullify(&seq->seq);
    qes_str_nullify(&seq->qual);
    return -2;
}

struct qes_seqfile *
qes_seqfile_create (const char *path, const char *mode)
{
    struct qes_seqfile *sf = NULL;
    if (path == NULL || mode == NULL) return NULL;
    sf = qes_calloc(1, sizeof(*sf));
    sf->qf = qes_file_open(path, mode);
    if (sf->qf == NULL) {
        qes_free(sf->qf);
        qes_free(sf);
        return NULL;
    }
    qes_str_init(&sf->scratch, __INIT_LINE_LEN);
    sf->n_records = 0;
    qes_seqfile_guess_format(sf);
    return sf;
}

enum qes_seqfile_format
qes_seqfile_guess_format (struct qes_seqfile *seqfile)
{
    int first_char = '\0';
    if (!qes_seqfile_ok(seqfile)) return UNKNOWN_FMT;
    if (!qes_file_readable(seqfile->qf)) return UNKNOWN_FMT;
    first_char = qes_file_peek(seqfile->qf);
    switch (first_char) {
        case FASTQ_DELIM:
            seqfile->format = FASTQ_FMT;
            return FASTQ_FMT;
            break;
        case FASTA_DELIM:
            seqfile->format = FASTA_FMT;
            return FASTA_FMT;
            break;
        default:
            seqfile->format = UNKNOWN_FMT;
            return UNKNOWN_FMT;
    }
}

void
qes_seqfile_set_format (struct qes_seqfile *seqfile,
                        enum qes_seqfile_format format)
{
    if (!qes_seqfile_ok(seqfile)) return;
    seqfile->format = format;
}

void
qes_seqfile_destroy_(struct qes_seqfile *seqfile)
{
    if (seqfile != NULL) {
        qes_file_close(seqfile->qf);
        qes_str_destroy_cp(&seqfile->scratch);
        qes_free(seqfile);
    }
}

size_t
qes_seqfile_format_seq(const struct qes_seq *seq, enum qes_seqfile_format fmt,
                       char *buffer, size_t maxlen)
{
    size_t len = 0;
    if (buffer == NULL || maxlen < 1) {
        return 0;
    }
    switch (fmt) {
        case FASTQ_FMT:
            if (!qes_seq_ok(seq)) {
                buffer[0] = '\0';
                return 0;
            }
            len = snprintf(buffer, maxlen, "%c%s %s\n%s\n%c\n%s\n",
                    FASTQ_DELIM, seq->name.str, seq->comment.str,
                    seq->seq.str,
                    FASTQ_QUAL_DELIM,
                    seq->qual.str);
            return len;
            break;
        case FASTA_FMT:
            if (!qes_seq_ok_no_qual(seq)) {
                buffer[0] = '\0';
                return 0;
            }
            len = snprintf(buffer, maxlen, "%c%s %s\n%s\n",
                    FASTA_DELIM, seq->name.str, seq->comment.str,
                    seq->seq.str);
            return len;
            break;
        case UNKNOWN_FMT:
        default:
            return 0;
    }
}


ssize_t
qes_seqfile_write (struct qes_seqfile *seqfile, struct qes_seq *seq)
{
#define sf_putc_check(c) ret = QES_ZFPUTC(seqfile->qf->fp, c);              \
    if (ret != c) {return -2;}                                              \
    else {res_len += 1;}                                                    \
    ret = 0
#define sf_puts_check(s) ret = QES_ZFPUTS(seqfile->qf->fp, s.str);          \
    if (ret < 0) {return -2;}                                               \
    else {res_len += s.len;}                                                \
    ret = 0

    int ret = 0;
    ssize_t res_len = 0;

    if (!qes_seqfile_ok(seqfile) || !qes_seq_ok(seq)) {
        return -2;
    }
    switch (seqfile->format) {
        case FASTA_FMT:
            sf_putc_check(FASTA_DELIM);
            sf_puts_check(seq->name);
            if (qes_seq_has_comment(seq)) {
                sf_putc_check(' ');
                sf_puts_check(seq->comment);
            }
            sf_putc_check('\n');
            sf_puts_check(seq->seq);
            sf_putc_check('\n');
            break;
        case FASTQ_FMT:
            sf_putc_check(FASTQ_DELIM);
            sf_puts_check(seq->name);
            if (qes_seq_has_comment(seq)) {
                sf_putc_check(' ');
                sf_puts_check(seq->comment);
            }
            sf_putc_check('\n');
            sf_puts_check(seq->seq);
            sf_putc_check('\n');
            if (qes_seq_has_qual(seq)) {
                sf_putc_check('+');
                sf_putc_check('\n');
                sf_puts_check(seq->qual);
                sf_putc_check('\n');

            }
            break;
        case UNKNOWN_FMT:
        default:
            return -2;
            break;
    }
    return res_len;
#undef sf_putc_check
#undef sf_puts_check
}
