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
 *       Filename:  qes_sequtil.c
 *
 *    Description:  Sequence utility functions
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "qes_sequtil.h"


/*
 * ===  FUNCTION  =============================================================
 *         Name:  qes_sequtil_translate_codon
 *  Description:  translate a codon to an amino acid.
 * ============================================================================
 */

inline int
qes_sequtil_translate_codon (const char *codon)
{
    if (codon == NULL || strnlen(codon, 4) != 3) {
        return -1;
    }
    if (codon[0] == 'A') {
        if (codon[1] == 'A') {
            if (codon[2] == 'A') return 'K';
            else if (codon[2] == 'C') return 'N';
            else if (codon[2] == 'G') return 'K';
            else if (codon[2] == 'T') return 'N';
            else if (codon[2] == 'U') return 'N';
         }
        else if (codon[1] == 'C') {
            if (codon[2] == 'A') return 'T';
            else if (codon[2] == 'C') return 'T';
            else if (codon[2] == 'G') return 'T';
            else if (codon[2] == 'T') return 'T';
            else if (codon[2] == 'U') return 'T';
         }
        else if (codon[1] == 'G') {
            if (codon[2] == 'A') return 'R';
            else if (codon[2] == 'C') return 'S';
            else if (codon[2] == 'G') return 'R';
            else if (codon[2] == 'T') return 'S';
            else if (codon[2] == 'U') return 'S';
         }
        else if (codon[1] == 'T') {
            if (codon[2] == 'A') return 'I';
            else if (codon[2] == 'C') return 'I';
            else if (codon[2] == 'G') return 'M';
            else if (codon[2] == 'T') return 'I';
            else if (codon[2] == 'U') return 'I';
         }
        else if (codon[1] == 'U') {
            if (codon[2] == 'A') return 'I';
            else if (codon[2] == 'C') return 'I';
            else if (codon[2] == 'G') return 'M';
            else if (codon[2] == 'T') return 'I';
            else if (codon[2] == 'U') return 'I';
         }
    }
    else if (codon[0] == 'C') {
        if (codon[1] == 'A') {
            if (codon[2] == 'A') return 'Q';
            else if (codon[2] == 'C') return 'H';
            else if (codon[2] == 'G') return 'Q';
            else if (codon[2] == 'T') return 'H';
            else if (codon[2] == 'U') return 'H';
         }
        else if (codon[1] == 'C') {
            if (codon[2] == 'A') return 'P';
            else if (codon[2] == 'C') return 'P';
            else if (codon[2] == 'G') return 'P';
            else if (codon[2] == 'T') return 'P';
            else if (codon[2] == 'U') return 'P';
         }
        else if (codon[1] == 'G') {
            if (codon[2] == 'A') return 'R';
            else if (codon[2] == 'C') return 'R';
            else if (codon[2] == 'G') return 'R';
            else if (codon[2] == 'T') return 'R';
            else if (codon[2] == 'U') return 'R';
         }
        else if (codon[1] == 'T') {
            if (codon[2] == 'A') return 'L';
            else if (codon[2] == 'C') return 'L';
            else if (codon[2] == 'G') return 'L';
            else if (codon[2] == 'T') return 'L';
            else if (codon[2] == 'U') return 'L';
         }
        else if (codon[1] == 'U') {
            if (codon[2] == 'A') return 'L';
            else if (codon[2] == 'C') return 'L';
            else if (codon[2] == 'G') return 'L';
            else if (codon[2] == 'T') return 'L';
            else if (codon[2] == 'U') return 'L';
         }
    }
    else if (codon[0] == 'G') {
        if (codon[1] == 'A') {
            if (codon[2] == 'A') return 'E';
            else if (codon[2] == 'C') return 'D';
            else if (codon[2] == 'G') return 'E';
            else if (codon[2] == 'T') return 'D';
            else if (codon[2] == 'U') return 'D';
         }
        else if (codon[1] == 'C') {
            if (codon[2] == 'A') return 'A';
            else if (codon[2] == 'C') return 'A';
            else if (codon[2] == 'G') return 'A';
            else if (codon[2] == 'T') return 'A';
            else if (codon[2] == 'U') return 'A';
         }
        else if (codon[1] == 'G') {
            if (codon[2] == 'A') return 'G';
            else if (codon[2] == 'C') return 'G';
            else if (codon[2] == 'G') return 'G';
            else if (codon[2] == 'T') return 'G';
            else if (codon[2] == 'U') return 'G';
         }
        else if (codon[1] == 'T') {
            if (codon[2] == 'A') return 'V';
            else if (codon[2] == 'C') return 'V';
            else if (codon[2] == 'G') return 'V';
            else if (codon[2] == 'T') return 'V';
            else if (codon[2] == 'U') return 'V';
         }
        else if (codon[1] == 'U') {
            if (codon[2] == 'A') return 'V';
            else if (codon[2] == 'C') return 'V';
            else if (codon[2] == 'G') return 'V';
            else if (codon[2] == 'T') return 'V';
            else if (codon[2] == 'U') return 'V';
         }
    }
    else if (codon[0] == 'T') {
        if (codon[1] == 'A') {
            if (codon[2] == 'A') return '*';
            else if (codon[2] == 'C') return 'Y';
            else if (codon[2] == 'G') return '*';
            else if (codon[2] == 'T') return 'Y';
            else if (codon[2] == 'U') return 'Y';
         }
        else if (codon[1] == 'C') {
            if (codon[2] == 'A') return 'S';
            else if (codon[2] == 'C') return 'S';
            else if (codon[2] == 'G') return 'S';
            else if (codon[2] == 'T') return 'S';
            else if (codon[2] == 'U') return 'S';
         }
        else if (codon[1] == 'G') {
            if (codon[2] == 'A') return '*';
            else if (codon[2] == 'C') return 'C';
            else if (codon[2] == 'G') return 'W';
            else if (codon[2] == 'T') return 'C';
            else if (codon[2] == 'U') return 'C';
         }
        else if (codon[1] == 'T') {
            if (codon[2] == 'A') return 'L';
            else if (codon[2] == 'C') return 'F';
            else if (codon[2] == 'G') return 'L';
            else if (codon[2] == 'T') return 'F';
            else if (codon[2] == 'U') return 'F';
         }
        else if (codon[1] == 'U') {
            if (codon[2] == 'A') return 'L';
            else if (codon[2] == 'C') return 'F';
            else if (codon[2] == 'G') return 'L';
            else if (codon[2] == 'T') return 'F';
            else if (codon[2] == 'U') return 'F';
         }
    }
    else if (codon[0] == 'U') {
        if (codon[1] == 'A') {
            if (codon[2] == 'A') return '*';
            else if (codon[2] == 'C') return 'Y';
            else if (codon[2] == 'G') return '*';
            else if (codon[2] == 'T') return 'Y';
            else if (codon[2] == 'U') return 'Y';
         }
        else if (codon[1] == 'C') {
            if (codon[2] == 'A') return 'S';
            else if (codon[2] == 'C') return 'S';
            else if (codon[2] == 'G') return 'S';
            else if (codon[2] == 'T') return 'S';
            else if (codon[2] == 'U') return 'S';
         }
        else if (codon[1] == 'G') {
            if (codon[2] == 'A') return '*';
            else if (codon[2] == 'C') return 'C';
            else if (codon[2] == 'G') return 'W';
            else if (codon[2] == 'T') return 'C';
            else if (codon[2] == 'U') return 'C';
         }
        else if (codon[1] == 'T') {
            if (codon[2] == 'A') return 'L';
            else if (codon[2] == 'C') return 'F';
            else if (codon[2] == 'G') return 'L';
            else if (codon[2] == 'T') return 'F';
            else if (codon[2] == 'U') return 'F';
         }
        else if (codon[1] == 'U') {
            if (codon[2] == 'A') return 'L';
            else if (codon[2] == 'C') return 'F';
            else if (codon[2] == 'G') return 'L';
            else if (codon[2] == 'T') return 'F';
            else if (codon[2] == 'U') return 'F';
         }
    }
    return 'X';
}


static inline char
qes_sequtil_complement (char base)
{
    switch (base) {
        case 'a': case 'A': return 'T';
        case 'c': case 'C': return 'G';
        case 'g': case 'G': return 'C';
        case 't': case 'T': return 'A';
        default: return 'N';
    }
}

/* Reverse complement of (at most) the first ``len`` bases of ``seq``, in a
 * newly malloc'd string. Trailing whitespace is trimmed. */
char *
qes_sequtil_revcomp (const char *seq, size_t len)
{
    size_t seqlen = 0;
    char *outseq = NULL;

    if (seq == NULL) {
        return NULL;
    }
    seqlen = strlen(seq);
    if (len < seqlen) {
        seqlen = len;
    }
    outseq = qes_malloc(seqlen + 1);
    memcpy(outseq, seq, seqlen);
    outseq[seqlen] = '\0';
    qes_sequtil_revcomp_inplace(outseq, seqlen);
    return outseq;
}

/* Reverse complement the first ``len`` bytes of ``seq`` in place. Trailing
 * whitespace is trimmed (and replaced by NULs). Non-ACGT bases become N. */
void
qes_sequtil_revcomp_inplace (char *seq, size_t len)
{
    size_t iii;

    if (seq == NULL) {
        return;
    }
    /* Trim trailing whitespace */
    while (len > 0 && isspace((unsigned char)seq[len - 1])) {
        seq[--len] = '\0';
    }
    for (iii = 0; iii < len / 2; iii++) {
        const size_t endpos = len - iii - 1;
        const char front = qes_sequtil_complement(seq[iii]);
        seq[iii] = qes_sequtil_complement(seq[endpos]);
        seq[endpos] = front;
    }
    if (len % 2 == 1) {
        seq[len / 2] = qes_sequtil_complement(seq[len / 2]);
    }
}
