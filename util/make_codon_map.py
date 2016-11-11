from Bio.Data import CodonTable

nts = "ACGTU"

codon_tab = CodonTable.generic_by_name["Standard"].forward_table
print "char\ntranslate_codon(char *codon)\n{"
for a in nts:
    print "    else if (codon[0] == '%s') {" % a
    for b in nts:
        print "        else if (codon[1] == '%s') {" % b
        for c in nts:
            print "            else if (codon[2] == '%s')" % c ,
            codon = "".join((a,b,c))
            try:
                aa = codon_tab[codon]
            except KeyError:
                aa = "*"
            print "return '%s'" % aa
        print "         }"
    print "    }"
print "}"
