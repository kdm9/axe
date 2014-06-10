
# coding: utf-8

# In[1]:

from itertools import (
    izip,
    combinations,
    product,
)
from collections import Counter
import itertools
import datrie


# In[2]:

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


# In[3]:

def make_bcd_tries(sf, K=1):
    print "In bcd tries"
    tries = []
    sf = list(sf)
    for k in range(K+1):
        print "Processing k=", k
        trie = datrie.Trie("ACGT")
        for hdr, seq in sf:
            print seq
            for mut in set(hamm_mutate(seq, k)):
                umut = unicode(mut)
                if umut in trie:
                    print "Error, already in tree, so there's an ambiguous barcode", mut
                trie[umut] = open("out_{}.fq".format(hdr), "w")
        print "Made trie with k={}, had length {}".format(k, len(trie))
        #tries.append(trie)
    return tries


# In[4]:

def lookup_read(tries, read):
    for trie in tries:
        try:
            return trie.longest_prefix(read)
        except KeyError:
            pass
    return None


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
    for seq in seqfile:
        bcd = lookup_read(tries, unicode(seq1[1]))
        counter[bcd] += 1
    return counter


# In[5]:

bcd_sf = iter_fasta1("/home/kevin/tmp/brachyGBS/barcodes.fa")
print len(list(bcd_sf))
bcd_sf = iter_fasta1("/home/kevin/tmp/brachyGBS/barcodes.fa")
seqfile1 = iter_fastq("/home/kevin/tmp/brachyGBS/BrachyPlate1_1M_R1.fq")
seqfile2 = iter_fastq("/home/kevin/tmp/brachyGBS/BrachyPlate1_1M_R2.fq")


# In[ ]:

tries = make_bcd_tries(bcd_sf, K=1)


# In[ ]:

ctr_se = count_barcodes_se(tries, seqfile1)

