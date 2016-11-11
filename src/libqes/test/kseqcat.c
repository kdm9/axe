#include "qes_config.h"
#include <stdio.h>
#include <stdlib.h>

#include <zlib.h>

#include "kseq.h"

#ifdef ZLIB_FOUND
#  include <zlib.h>
   KSEQ_INIT(gzFile, gzread)
#else
#  include <sys/stat.h>
#  include <fcntl.h>
   KSEQ_INIT(int, read)
#endif

int main(int argc, char *argv[])
{
    if (argc < 2) return 1;
    const char *fname = argv[1];
    gzFile fp = gzopen(fname, "r");
    kseq_t *kseq = kseq_init(fp);
    ssize_t res = 0;
    while((res = kseq_read(kseq)) > 0) {
        if (kseq->qual.l < 1) {
            // fasta
            printf(">%s %s\n", kseq->name.s, kseq->comment.s);
            puts(kseq->seq.s);
        } else {
            // fastq
            printf("@%s %s\n", kseq->name.s, kseq->comment.s);
            puts(kseq->seq.s);
            puts("+");
            puts(kseq->qual.s);
        }
    }
    kseq_destroy(kseq);
    gzclose(fp);
    return 0;
}
