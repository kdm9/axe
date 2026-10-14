#!/usr/bin/env python3
"""Simulate 1M (pairs of) reads per scenario and compare axe's output to
the demultiplexing a nearest-barcode oracle expects. Needs numpy."""
import os
import subprocess as sp
import sys
import tempfile
import unittest

import numpy as np

BINDIR = sys.argv.pop(1) if len(sys.argv) > 1 else "."
AXE = os.path.join(BINDIR, "bin", "axe-demux")
N = 1000000
GENOMIC = 50
PSTI = b"TGCAG"
ACGT = np.frombuffer(b"ACGT", np.uint8)
CODE = np.zeros(256, np.uint8)
CODE[ACGT] = range(4)


def make_barcodes(rng, n, lengths):
    seen = set()
    while len(seen) < n:
        seen.add(ACGT[rng.integers(0, 4, rng.choice(lengths))].tobytes())
    return sorted(seen)


def simulate(rng, bcs, who, mm, site):
    """Reads of samples `who`, with 0..mm+1 barcode errors, 2% pure junk."""
    n = len(who)
    pad = np.zeros((len(bcs), 8), np.uint8)
    for i, bc in enumerate(bcs):
        pad[i, :len(bc)] = list(bc)
    lens = np.array([len(bc) for bc in bcs])[who]
    inside = np.arange(8) < lens[:, None]
    bar = pad[who]
    score = np.where(inside, rng.random((n, 8)), 2)
    err = np.argsort(np.argsort(score, 1), 1) < rng.integers(0, mm + 2, n)[:, None]
    mut = ACGT[(CODE[bar] + rng.integers(1, 4, (n, 8))) % 4]
    bar = np.where(err & inside, mut, bar)
    junk = rng.random(n) < 0.02
    bar[junk] = ACGT[rng.integers(0, 4, (junk.sum(), 8))]
    tail = np.hstack([np.tile(np.frombuffer(site, np.uint8), (n, 1)),
                      ACGT[rng.integers(0, 4, (n, GENOMIC))]])
    return [bar[i, :lens[i]].tobytes() + tail[i].tobytes() for i in range(n)]


def assign(reads, bcs, mm):
    """Index of the unique nearest barcode within mm (longest barcodes
    first, as the prefix trie does), else -1."""
    first = np.frombuffer(b"".join(r[:8] for r in reads), np.uint8).reshape(-1, 8)
    out = np.full(len(reads), -1)
    for length in sorted({len(bc) for bc in bcs}, reverse=True):
        ids = np.array([i for i, bc in enumerate(bcs) if len(bc) == length])
        bar = np.array([list(bcs[i]) for i in ids], np.uint8)
        for s in range(0, len(reads), 50000):
            dist = (first[s:s + 50000, None, :length] != bar).sum(2)
            best = dist.min(1)
            hit = ((best <= mm) & ((dist == best[:, None]).sum(1) == 1)
                   & (out[s:s + 50000] < 0))
            out[s:s + 50000][hit] = ids[dist.argmin(1)][hit]
    return out


def record(i, seq, trim=0):
    return b"@r%d\n%s\n+\n%s\n" % (i, seq[trim:], b"I" * (len(seq) - trim))


class Simulation(unittest.TestCase):
    def setUp(self):
        self.rng = np.random.default_rng(42)
        self.tmp = tempfile.TemporaryDirectory()

    def tearDown(self):
        self.tmp.cleanup()

    def path(self, name):
        return os.path.join(self.tmp.name, name)

    def write(self, name, chunks):
        with open(self.path(name), "wb") as fh:
            fh.writelines(chunks)
        return self.path(name)

    def run_axe(self, mm, *args):
        cmd = [AXE, "-q", "-p", "-m", str(mm), "-F", self.path("o")] + list(args)
        proc = sp.run(cmd, stdout=sp.PIPE, stderr=sp.STDOUT)
        self.assertEqual(proc.returncode, 0, proc.stdout.decode())

    def compare(self, expected):
        names = {n for n in os.listdir(self.tmp.name) if n.startswith("o_")}
        self.assertEqual(names, {"o_%s_%s.fastq" % k for k in expected})
        for (sample, mate), recs in expected.items():
            with open(self.path("o_%s_%s.fastq" % (sample, mate)), "rb") as fh:
                self.assertEqual(fh.read(), b"".join(recs), (sample, mate))

    def single_end(self, bcs, mm):
        who = self.rng.integers(0, len(bcs), N)
        reads = simulate(self.rng, bcs, who, mm, b"")
        found = assign(reads, bcs, mm)
        self.assertGreater((found == who).mean(), 0.2)
        names = ["s%02d" % i for i in range(len(bcs))]
        bcfile = self.write("bc.tsv", [b"%s\t%s\n" % (b, n.encode())
                                       for b, n in zip(bcs, names)])
        fq = self.write("in.fq", [record(i, r) for i, r in enumerate(reads)])
        self.run_axe(mm, "-f", fq, "-b", bcfile)
        expected = {(n, "R1"): [] for n in names + ["unknown"]}
        for i, (r, a) in enumerate(zip(reads, found)):
            if a < 0:
                expected[("unknown", "R1")].append(record(i, r))
            else:
                expected[(names[a], "R1")].append(record(i, r, len(bcs[a])))
        self.compare(expected)

    def combinatorial(self, n1, n2, lengths, mm, site):
        bc1 = make_barcodes(self.rng, n1, lengths)
        bc2 = make_barcodes(self.rng, n2, lengths)
        w1 = self.rng.integers(0, n1, N)
        w2 = self.rng.integers(0, n2, N)
        r1 = simulate(self.rng, bc1, w1, mm, site)
        r2 = simulate(self.rng, bc2, w2, mm, site)
        f1, f2 = assign(r1, bc1, mm), assign(r2, bc2, mm)
        self.assertGreater(((f1 == w1) & (f2 == w2)).mean(), 0.2)
        bcfile = self.write("bc.tsv", [b"%s\t%s\ts%d_%d\n" % (a, b, i, j)
                                       for i, a in enumerate(bc1)
                                       for j, b in enumerate(bc2)])
        fq1 = self.write("r1.fq", [record(i, r) for i, r in enumerate(r1)])
        fq2 = self.write("r2.fq", [record(i, r) for i, r in enumerate(r2)])
        self.run_axe(mm, "-c", "-f", fq1, "-r", fq2, "-R", self.path("o"),
                     "-b", bcfile)
        expected = {("unknown", m): [] for m in ("R1", "R2")}
        expected.update({("s%d_%d" % (i, j), m): [] for i in range(n1)
                         for j in range(n2) for m in ("R1", "R2")})
        for i in range(N):
            a, b = f1[i], f2[i]
            if a < 0 or b < 0:
                expected[("unknown", "R1")].append(record(i, r1[i]))
                expected[("unknown", "R2")].append(record(i, r2[i]))
            else:
                sample = "s%d_%d" % (a, b)
                expected[(sample, "R1")].append(record(i, r1[i], len(bc1[a])))
                expected[(sample, "R2")].append(record(i, r2[i], len(bc2[b])))
        self.compare(expected)

    def test_48_single_end_6bp_2mm(self):
        self.single_end(make_barcodes(self.rng, 48, [6]), 2)

    def test_96_combinatorial_8bp_4mm(self):
        self.combinatorial(12, 8, [8], 4, b"")

    def test_96_combinatorial_gbs_4_8bp_2mm_psti(self):
        self.combinatorial(12, 8, [4, 5, 6, 7, 8], 2, PSTI)


if __name__ == "__main__":
    unittest.main()
