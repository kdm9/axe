from itertools import (
    izip,
    combinations,
    product,
    islice,
)
from collections import Counter
import itertools
import datrie
import docopt

CLI_DOC = """
USAGE:
    trieBCD.py [-m MISMATCH] -i IN_1 -I IN_2 -o OUT_PREFIX -b BCD_FASTA

OPTIONS:
    -i IN_1         Input R1 fastq file.
    -I IN_2         Input R2 fastq file.
    -o OUT_PREFIX   Output prefix. output files will be:
                    <OUT_PREFIX>_<barcode sequence>_R<read num>.fq
    -b BCD_FASTA    Fasta containing the barcodes. The headers will be used to
                        name demultiplexed fastqs, so make the headers useful.
    -m MISMATCH     Allow a hamming distance of up to MISMATCH. [Default: 1]
"""


def iter_fastq(fn):
    with open(fn) as fh:
        for h, s, _, q in izip(fh, fh, fh, fh):
            yield (h[1:].strip(), s.strip(), q.strip())

def iter_fasta1(fn):
    with open(fn) as fh:
        for h, s in izip(fh, fh):
            yield (h[1:].strip(), s.strip())

def hamm_mutate(seq, k):
    if k == 0:
        yield seq
    # For every group of length k sites in seq
    for mut_idxs in combinations(range(len(seq)), k):
        # For every k-mer of alphabet ACGT
        for replacements in product("ACGT", repeat=k):
            # Do a mutation
            mutated = list(seq)
            for index, replacement in izip(mut_idxs, replacements):
                mutated[index] = replacement
            yield "".join(mutated)

def make_bcd_tries(sf, K=1, basename="out"):
    tries = []
    files = {}
    sf = list(sf)
    for k in range(K+1):
        trie = datrie.Trie("ACGT")
        for hdr, seq in sf:
            files[seq] = (open("{}_{}_R1.fq".format(basename, seq), "w"),
                    open("{}_{}_R2.fq".format(basename, seq), "w"))
            for mut in set(hamm_mutate(seq, k)):
                umut = unicode(mut)
                if umut in trie:
                    print "Error, already in tree, so there's an ambiguous barcode", mut
                trie[umut] = seq
        print "Made trie with k={}, had length {}".format(k, len(trie))
        tries.append(trie)
    files[None] = (open("{}_NOBCD_R1.fq".format(basename), "w"),
                   open("{}_NOBCD_R2.fq".format(basename), "w"))
    return (tries, files)

def lookup_read(tries, read):
    for trie in tries:
        try:
            ret = trie.longest_prefix(read)
            # there's no gurantee that the
            while ret not in trie:
                ret = ret[:-1]
            if len(ret) < 1:
                raise KeyError
            return (ret, trie[ret])
        except KeyError:
            pass
    return (None, None)

def count_barcodes_pe(tries, seqfile1, seqfile2):
    counter = Counter()
    for seq1, seq2 in izip(seqfile1, seqfile2):
        try:
            bcd = trie.longest_prefix(unicode(seq1[1]))
        except KeyError:
            bcd = None
        counter[bcd] += 1
    return counter

def count_barcodes_se(tries, seqfile):
    counter = Counter()
    re_counter = Counter()
    for seq in seqfile:
        read = seq[1]
        bcd, true_bcd = lookup_read(tries, unicode(read))
        bcd_ln = 0
        if bcd is not None:
            bcd_ln = len(bcd)
            re_counter[read[bcd_ln:bcd_ln+5]] += 1
        counter[bcd] += 1
    for re, c in re_counter.most_common(25):
        print re, c
    return counter

def main(opts):
    bcd_sf = iter_fasta1(opts['-b'])
    tries, filedict = make_bcd_tries(bcd_sf, K=int(opts['-m']),
                                     basename=opts['-o'])
    seqfile1 = iter_fastq(opts['-i'])
    seqfile2 = iter_fastq(opts['-I'])
    ctr = Counter()
    for seq1, seq2 in izip(seqfile1, seqfile2):
        fwd = seq1[1]
        bcd, true_bcd = lookup_read(tries, unicode(fwd))
        bcd_ln = 0
        if bcd is not None:
            bcd_ln = len(bcd)
        # trim away bcd
        oseq1 = (seq1[0], seq1[1][bcd_ln:], seq1[2][bcd_ln:])
        o1, o2 = filedict[true_bcd]
        o1.write("{}\n{}\n+\n{}".format(*oseq1))
        o2.write("{}\n{}\n+\n{}".format(*seq2))
        ctr[true_bcd] += 1
    for k, v in ctr.most_common():
        print k, v


def counters():
    """old shit from ipynb"""
    ctr_se = count_barcodes_se(tries, seqfile1)
    for k, v in ctr_se.most_common():
        print str(k).ljust(10), v

if __name__ == "__main__":
    opts = docopt.docopt(CLI_DOC)
    main(opts)
