from Bio.Data import CodonTable

nts = "ACGTU"
codons = []
aas = []
codon_tab = CodonTable.generic_by_name["Standard"].forward_table
for a in nts:
    for b in nts:
        for c in nts:
            codon = "".join((a,b,c))
            try:
                aa = codon_tab[codon.replace("U", "T")]
            except KeyError:
                aa = "*"
            codons.append(codon)
            aas.append(aa)
n = 0
print "const size_t n_codons = %s;" % len(codons)
print
print "const char *codon_list[] = {" ,
for cdn in codons:
    if n % 5 == 0:
        print "\n    ",
    print '"%s", ' % cdn,
    n += 1
print
print "};"
n = 0
print
print "const char aa_list[] = {",
for aa in aas:
    if n % 5 == 0:
        print "\n    ",
    print "'%s', " % aa,
    n += 1
print
print "};"
print
