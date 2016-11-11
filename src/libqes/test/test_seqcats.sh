#!/bin/bash

if [ $# -lt 1 ]
then
    files=("data/test.fasta")
else
    files=( "$@" )
fi

for file in "${files[@]}"
do
    kseq_m5=$(bin/kseqcat $file | md5sum | awk '{print $1}')
    qseq_m5=$(bin/qes_seqcat $file | md5sum | awk '{print $1}')
    kprint_m5=$(bin/kseqcat $file | seqtk seq -l 79 | md5sum | awk '{print $1}')
    qprint_m5=$(bin/qes_seqprint $file | md5sum | awk '{print $1}')

    if [ "$kseq_m5" != "$qseq_m5" ]
    then
        echo "TEST (seq) FAILED: $file"
        echo "  kseq: $kseq_m5"
        echo "  qes: $qseq_m5"
    else
        echo "TEST (seq) PASSED: $file"
    fi

    if [ "$kprint_m5" != "$qprint_m5" ]
    then
        echo "TEST (print) FAILED: $file"
        echo "  kseq: $kprint_m5"
        echo "  qes: $qprint_m5"
    else
        echo "TEST (print) PASSED: $file"
    fi
done
