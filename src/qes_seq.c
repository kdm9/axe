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
 *       Filename:  qes_seq.c
 *
 *    Description:  Sequence structures
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "qes_seq.h"


void
qes_seq_init (struct qes_seq *seq)
{
    qes_str_init(&seq->name, __INIT_LINE_LEN);
    qes_str_init(&seq->comment, __INIT_LINE_LEN);
    qes_str_init(&seq->seq, __INIT_LINE_LEN);
    qes_str_init(&seq->qual, __INIT_LINE_LEN);
}

struct qes_seq *
qes_seq_create (void)
{
    struct qes_seq *seq = qes_malloc(sizeof(*seq));

    qes_str_init(&seq->name, __INIT_LINE_LEN);
    qes_str_init(&seq->comment, __INIT_LINE_LEN);
    qes_str_init(&seq->seq, __INIT_LINE_LEN);
    qes_str_init(&seq->qual, __INIT_LINE_LEN);
    return seq;
}

struct qes_seq *
qes_seq_create_no_qual (void)
{
    struct qes_seq *seq = qes_malloc(sizeof(*seq));

    qes_str_init(&seq->name, __INIT_LINE_LEN);
    qes_str_init(&seq->comment, __INIT_LINE_LEN);
    qes_str_init(&seq->seq, __INIT_LINE_LEN);
    seq->qual.capacity = 0;
    seq->qual.len = 0;
    seq->qual.str = NULL;
    return seq;
}

struct qes_seq *
qes_seq_create_no_qual_or_comment (void)
{
    struct qes_seq *seq = qes_malloc(sizeof(*seq));
    qes_str_init(&seq->name, __INIT_LINE_LEN);
    qes_str_init(&seq->seq, __INIT_LINE_LEN);
    seq->qual.capacity = 0;
    seq->qual.len = 0;
    seq->qual.str = NULL;
    seq->comment.capacity = 0;
    seq->comment.len = 0;
    seq->comment.str = NULL;
    return seq;
}


inline int
qes_seq_fill_name (struct qes_seq *seqobj, const char *name, size_t len)
{
    if (seqobj == NULL || name == NULL || len < 1) {
        return 1;
    }
    qes_str_fill_charptr(&seqobj->name, name, len);
    return 0;
}

inline int
qes_seq_fill_comment (struct qes_seq *seqobj, const char *comment, size_t len)
{
    if (seqobj == NULL || comment == NULL || len < 1) {
        return 1;
    }
    qes_str_fill_charptr(&seqobj->comment, comment, len);
    return 0;
}

inline int
qes_seq_fill_seq (struct qes_seq *seqobj, const char *seq, size_t len)
{
    if (seqobj == NULL || seq == NULL || len < 1) {
        return 1;
    }
    qes_str_fill_charptr(&seqobj->seq, seq, len);
    return 0;
}

inline int
qes_seq_fill_qual (struct qes_seq *seqobj, const char *qual, size_t len)
{
    if (seqobj == NULL || qual == NULL || len < 1) {
        return 1;
    }
    qes_str_fill_charptr(&seqobj->qual, qual, len);
    return 0;
}

inline int
qes_seq_fill_header (struct qes_seq *seqobj, char *header, size_t len)
{
    char *tmp = NULL;
    size_t startfrom = 0;

    if (seqobj == NULL || header == NULL) {
        return 1;
    }
    if (len < 1) {
        len = strlen(header);
    }
    while (isspace(header[len-1])) {
        header[--len] = '\0';
    }
    tmp = memchr(header, ' ', len);
    startfrom = header[0] == '@' || header[0] == '>' ? 1 : 0;
    if (tmp != NULL) {
        qes_str_fill_charptr(&seqobj->name, header + startfrom,
                tmp - header - startfrom);
        qes_str_fill_charptr(&seqobj->comment, tmp + 1, 0);
    } else {
        qes_str_fill_charptr(&seqobj->name, header + startfrom, len - startfrom);
        qes_str_nullify(&seqobj->comment);
    }
    return 0;
}

inline int
qes_seq_fill(struct qes_seq *seqobj, const char *name, const char *comment,
             const char *seq, const char *qual)
{
    if (!qes_seq_ok(seqobj) || name == NULL || comment == NULL || seq == NULL \
            || qual == NULL) {
        return 1;
    }
    qes_str_nullify(&seqobj->name);
    qes_str_nullify(&seqobj->comment);
    qes_str_nullify(&seqobj->seq);
    qes_str_nullify(&seqobj->qual);
    if (qes_seq_fill_name(seqobj, name, strlen(name)) != 0) return 1;
    if (qes_seq_fill_comment(seqobj, comment, strlen(comment)) != 0) return 1;
    if (qes_seq_fill_seq(seqobj, seq, strlen(seq)) != 0) return 1;
    if (strlen(qual) > 0 &&
        qes_seq_fill_qual(seqobj, qual, strlen(qual)) != 0) return 1;
    return 0;
}

/*===  FUNCTION  ============================================================*
Name:           qes_seq_destroy
Parameters:     struct qes_seq *: seq to destroy.
Description:    Deallocate and set to NULL a struct qes_seq on the heap.
Returns:        void.
 *===========================================================================*/
void
qes_seq_destroy_(struct qes_seq *seq)
{
    if (seq != NULL) {
        qes_str_destroy_cp(&seq->name);
        qes_str_destroy_cp(&seq->comment);
        qes_str_destroy_cp(&seq->seq);
        qes_str_destroy_cp(&seq->qual);
        qes_free(seq);
    }
}

inline void
_printstr_linewrap(const struct qes_str *str, size_t linelen, FILE *stream)
{

    for (size_t i = 0; i < str->len; i += linelen) {
        const size_t remaining = str->len - i;
        const size_t towrite = remaining < linelen ? remaining : linelen;
        fwrite(str->str + i, towrite, 1,  stream);
        fputc('\n', stream);
    }
}

int
qes_seq_print(const struct qes_seq *seq, FILE *stream, bool fasta, int tag)
{
    if (!qes_seq_ok(seq)) return 1;
    if (stream == NULL) return 1;
    size_t linelen = fasta ? 79 : SIZE_MAX - 1;

    if (fasta) {
        fputc('>', stream);
    } else {
        fputc('@', stream);
    }
    fputs(seq->name.str, stream);
    if (tag > 0) {
        // Add tag only if read is not already tagged.
        if (seq->name.len > 2 && seq->name.str[seq->name.len - 2] != '/') {
            fprintf(stream, "/%d", tag);
        }
    }
    if (seq->comment.str) {
        fputc(' ', stream);
        fputs(seq->comment.str, stream);
    }
    fputc('\n', stream);
    _printstr_linewrap(&seq->seq, linelen, stream);
    if (!fasta) {
        fputs("+\n", stream);
        _printstr_linewrap(&seq->qual, linelen, stream);
    }
    fflush(stream);
    return 0;
}
