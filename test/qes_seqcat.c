#include "qes_config.h"
#include <stdio.h>
#include <stdlib.h>

#include <qes_seqfile.h>

int main(int argc, char *argv[])
{
    if (argc < 2) return 1;
    const char *fname = argv[1];
    struct qes_seq *seq = qes_seq_create();
    struct qes_seqfile *sf = qes_seqfile_create(fname, "r");
    ssize_t res = 0;
    while((res = qes_seqfile_read(sf, seq)) > 0) {
        if (seq->qual.len < 1) {
            // fasta
            printf(">%s %s\n", seq->name.str, seq->comment.str);
            puts(seq->seq.str);
        } else {
            // fastq
            printf("@%s %s\n", seq->name.str, seq->comment.str);
            puts(seq->seq.str);
            puts("+");
            puts(seq->qual.str);
        }
    }
    qes_seqfile_destroy(sf);
    qes_seq_destroy(seq);
    return 0;
}
