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
 *       Filename:  testdata.c
 *
 *    Description:  Data for tests
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "testdata.h"


const size_t n_loremipsum_lines = 11;
const size_t loremipsum_fsize = 80+76+80+75+80+79+77+75+69+1+20;
const size_t loremipsum_line_lens[] = {
    80, 76, 80, 75, 80, 79, 77, 75, 69, 1, 20
};
const char *loremipsum_lines[] = {
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec ornare tortor et\n",
    "rhoncus iaculis. Sed suscipit, arcu nec elementum vestibulum, tortor tortor\n",
    "dictum dui, eu sodales magna orci eu libero. Cras commodo, ligula tempor auctor\n",
    "vulputate, eros urna gravida eros, eget congue leo quam quis mi. Curabitur\n",
    "luctus augue nibh, eget vehicula augue commodo eget. Donec condimentum molestie\n",
    "adipiscing. In non purus lacus. Nam nec mollis mauris. Donec rhoncus, diam sit\n",
    "amet rhoncus viverra, lectus risus tincidunt ipsum, in dignissim justo purus\n",
    "eget enim. Fusce congue nulla egestas est auctor faucibus. Integer feugiat\n",
    "molestie leo, a interdum neque pretium nec. Etiam sit amet nibh leo.\n",
    "\n",
    "End of lorem ipsum.\n",
};
const char *first_fastq_read[] = {
    "HWI-ST960:105:D10GVACXX:2:1101:1151:2158",
    "1:N:0: bcd:RPI9 seq:CACGATCAGATC",
    "CACGATCAGATCAANGACATTGAATCTATATGT",
    "JJJJJJJIJHIJCC#4ADFFHHHGHJJJJIJJJ",
};
const size_t first_fastq_len = 33;


const size_t n_codons = 125;

const char *codon_list[] = {
     "AAA",  "AAC",  "AAG",  "AAT",  "AAU",
     "ACA",  "ACC",  "ACG",  "ACT",  "ACU",
     "AGA",  "AGC",  "AGG",  "AGT",  "AGU",
     "ATA",  "ATC",  "ATG",  "ATT",  "ATU",
     "AUA",  "AUC",  "AUG",  "AUT",  "AUU",
     "CAA",  "CAC",  "CAG",  "CAT",  "CAU",
     "CCA",  "CCC",  "CCG",  "CCT",  "CCU",
     "CGA",  "CGC",  "CGG",  "CGT",  "CGU",
     "CTA",  "CTC",  "CTG",  "CTT",  "CTU",
     "CUA",  "CUC",  "CUG",  "CUT",  "CUU",
     "GAA",  "GAC",  "GAG",  "GAT",  "GAU",
     "GCA",  "GCC",  "GCG",  "GCT",  "GCU",
     "GGA",  "GGC",  "GGG",  "GGT",  "GGU",
     "GTA",  "GTC",  "GTG",  "GTT",  "GTU",
     "GUA",  "GUC",  "GUG",  "GUT",  "GUU",
     "TAA",  "TAC",  "TAG",  "TAT",  "TAU",
     "TCA",  "TCC",  "TCG",  "TCT",  "TCU",
     "TGA",  "TGC",  "TGG",  "TGT",  "TGU",
     "TTA",  "TTC",  "TTG",  "TTT",  "TTU",
     "TUA",  "TUC",  "TUG",  "TUT",  "TUU",
     "UAA",  "UAC",  "UAG",  "UAT",  "UAU",
     "UCA",  "UCC",  "UCG",  "UCT",  "UCU",
     "UGA",  "UGC",  "UGG",  "UGT",  "UGU",
     "UTA",  "UTC",  "UTG",  "UTT",  "UTU",
     "UUA",  "UUC",  "UUG",  "UUT",  "UUU",
};

const char aa_list[] = {
     'K',  'N',  'K',  'N',  'N',
     'T',  'T',  'T',  'T',  'T',
     'R',  'S',  'R',  'S',  'S',
     'I',  'I',  'M',  'I',  'I',
     'I',  'I',  'M',  'I',  'I',
     'Q',  'H',  'Q',  'H',  'H',
     'P',  'P',  'P',  'P',  'P',
     'R',  'R',  'R',  'R',  'R',
     'L',  'L',  'L',  'L',  'L',
     'L',  'L',  'L',  'L',  'L',
     'E',  'D',  'E',  'D',  'D',
     'A',  'A',  'A',  'A',  'A',
     'G',  'G',  'G',  'G',  'G',
     'V',  'V',  'V',  'V',  'V',
     'V',  'V',  'V',  'V',  'V',
     '*',  'Y',  '*',  'Y',  'Y',
     'S',  'S',  'S',  'S',  'S',
     '*',  'C',  'W',  'C',  'C',
     'L',  'F',  'L',  'F',  'F',
     'L',  'F',  'L',  'F',  'F',
     '*',  'Y',  '*',  'Y',  'Y',
     'S',  'S',  'S',  'S',  'S',
     '*',  'C',  'W',  'C',  'C',
     'L',  'F',  'L',  'F',  'F',
     'L',  'F',  'L',  'F',  'F',
};

void
test_data_files (void *ptr)
{
    char *fname = NULL;
    char *crc_res = NULL;

    (void) ptr;
    fname = find_data_file("loremipsum.txt");
    crc_res = crc32_file(fname);
    tt_str_op(crc_res, ==, "9f20f7ec");
    free(fname);
    free(crc_res);
    fname = find_data_file("loremipsum.txt.gz");
    crc_res = crc32_file(fname);
    tt_str_op(crc_res, ==, "4e42dcb2");
    free(fname);
    free(crc_res);
    fname = find_data_file("test.fastq");
    crc_res = crc32_file(fname);
    tt_str_op(crc_res, ==, "c32cc3c0");
    free(fname);
    free(crc_res);
    fname = find_data_file("test.fasta");
    crc_res = crc32_file(fname);
    tt_str_op(crc_res, ==, "3de06bb6");
    free(fname);
    free(crc_res);
    fname = find_data_file("test.fastq.gz");
    crc_res = crc32_file(fname);
    tt_str_op(crc_res, ==, "ba1206ee");
    free(fname);
    free(crc_res);
    fname = find_data_file("test.fastq.bz2");
    crc_res = crc32_file(fname);
    tt_str_op(crc_res, ==, "c8b66d33");
end:
    free(fname);
    free(crc_res);
}

struct testcase_t data_tests[] = {
    { "data_files", test_data_files, 0, NULL, NULL},
    END_OF_TESTCASES
};
