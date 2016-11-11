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
        int fasta = seq->qual.len < 1;
        qes_seq_print(seq, stdout, fasta, 0);
    }
    qes_seqfile_destroy(sf);
    qes_seq_destroy(seq);
    return 0;
}
