#!/usr/bin/env python3
import hashlib
import logging
import os
from os import path
import re
import shutil
import subprocess as sp
import sys
import tempfile
import unittest


if len(sys.argv) < 2:
    print("USAGE: axe_cli_tests.py $CMAKE_BINARY_DIR")
    exit(-1)

CMAKE_BINARY_DIR = sys.argv.pop(1)


def md5sum(filename):
    h = hashlib.md5()
    with open(filename, 'rb') as fh:
        while True:
            hunk = fh.read(1024 ** 2)
            if not hunk:
                break
            h.update(hunk)
    return h.hexdigest()


class AxeTest(unittest.TestCase):
    maxDiff=  None

    def __init__(self, methodName='runTest'):
        super(AxeTest, self).__init__(methodName)
        self.data = path.join(CMAKE_BINARY_DIR, "data")
        self.out = path.join(CMAKE_BINARY_DIR, "out", "cli_tests")
        self.axe = path.join(CMAKE_BINARY_DIR, "bin", "axe-demux")
        self.log = logging.getLogger("AxeTest")
        if not path.exists(self.data) or not path.exists(self.axe):
            print("Please run axe_cli_tests.py after compiling axe")
            exit(-1)

    def setUp(self):
        if not path.exists(self.out):
            os.makedirs(self.out)

    def run_and_check_stdout(self, command):
        self.log.debug(" ".join(command))
        try:
            output = sp.check_output(command, stderr=sp.STDOUT)
        except sp.CalledProcessError as err:
            self.log.info(err.output)
            return False
        return True

    def get_md5_dict(self):
        dct = {}
        for root, dirs, files in os.walk(self.out):
            for fle in files:
                md5 = md5sum(path.join(root, fle))
                dct[path.basename(fle)] = md5
        return dct

    def tearDown(self):
        if path.exists(self.out):
            shutil.rmtree(self.out)


class TestBadUsage(AxeTest):
    def __init__(self, methodName='runTest'):
        super(TestBadUsage, self).__init__(methodName)

    def test_bad_command(self):
        command = [self.axe, "-f"]
        self.assertFalse(self.run_and_check_stdout(command))
        self.assertDictEqual({}, self.get_md5_dict())


class TestPareSE(AxeTest):
    def __init__(self, methodName='runTest'):
        super(TestPareSE, self).__init__(methodName)
        self.infq = path.join(self.data, "pare.fq.gz")
        self.barcodes = path.join(self.data, "pare.barcodes")
        self.outfq = path.join(self.out, "pare_se")
        self.nobcdfq = path.join(self.out, "pare_se_unknown_R1.fastq")

    def test_pare_se(self):
        command = [self.axe,
            "-f", self.infq,
            "-F", self.outfq,
            '-b', self.barcodes,
        ]
        self.assertTrue(self.run_and_check_stdout(command))
        files = {
            'pare_se_1_R1.fastq': 'd41d8cd98f00b204e9800998ecf8427e',
            'pare_se_2_R1.fastq': 'd41d8cd98f00b204e9800998ecf8427e',
            'pare_se_3_R1.fastq': 'd41d8cd98f00b204e9800998ecf8427e',
            'pare_se_4_R1.fastq': '8e5eef3323e597b209f79dc9fcd74c9a',
            'pare_se_5_R1.fastq': 'd41d8cd98f00b204e9800998ecf8427e',
            'pare_se_6_R1.fastq': '7228a165f353920328360dedc3a41205',
            'pare_se_7_R1.fastq': 'd41d8cd98f00b204e9800998ecf8427e',
            'pare_se_8_R1.fastq': 'b349d3276ba7c7515d0093b1a49b3959',
            'pare_se_9_R1.fastq': '74b4763271aefcc135425b06730874ba',
            'pare_se_unknown_R1.fastq': 'd450569dd8fd4bdddffbfaeec4980273',
        }
        self.assertDictEqual(files, self.get_md5_dict())

    def test_pare_se_zip(self):
        command = [self.axe,
            "-f", self.infq,
            "-F", self.outfq,
            '-b', self.barcodes,
            '-z', '9',
        ]
        files = {
            'pare_se_1_R1.fastq.gz': '4a4dd3598707603b3f76a2378a4504aa',
            'pare_se_2_R1.fastq.gz': '4a4dd3598707603b3f76a2378a4504aa',
            'pare_se_3_R1.fastq.gz': '4a4dd3598707603b3f76a2378a4504aa',
            'pare_se_4_R1.fastq.gz': '96d21b860a0fc70641ea43d350433d11',
            'pare_se_5_R1.fastq.gz': '4a4dd3598707603b3f76a2378a4504aa',
            'pare_se_6_R1.fastq.gz': 'd6044c04f79c358e4a1d443f8828df18',
            'pare_se_7_R1.fastq.gz': '4a4dd3598707603b3f76a2378a4504aa',
            'pare_se_8_R1.fastq.gz': 'da77b8e95827d362a1702ce4fe75c7a9',
            'pare_se_9_R1.fastq.gz': '9c160b0daa0c73e5ef0994206774a5a0',
            'pare_se_unknown_R1.fastq.gz': 'afd5737935814d756e89c365d2d61c7b',
        }
        self.assertTrue(self.run_and_check_stdout(command))
        self.assertDictEqual(files, self.get_md5_dict())

class TestFakeSE(AxeTest):

    def __init__(self, methodName='runTest'):
        super(TestFakeSE, self).__init__(methodName)
        self.barcodes = path.join(self.data, "fake.barcodes")
        self.outfq = path.join(self.out, "fake_se")
        self.nobcdfq = path.join(self.out, "fake_se_unknown_R1.fastq")

    def _do_test(self, mm_level, mismatch=0):
        infq = path.join(self.data, "fake_{}mm_R1.fq.gz".format(mm_level))
        command = [self.axe,
            "-f", infq,
            "-F", self.outfq,
            '-b', self.barcodes,
            '-m', str(mismatch),
        ]
        self.assertTrue(self.run_and_check_stdout(command))

    def _do_test_zip(self, mm_level, mismatch=0):
        infq = path.join(self.data, "fake_{}mm_R1.fq.gz".format(mm_level))
        command = [self.axe,
            "-f", infq,
            "-F", self.outfq,
            '-b', self.barcodes,
            '-m', str(mismatch),
            '-z', '9',
        ]
        self.assertTrue(self.run_and_check_stdout(command))

    def test_fake_se_0mm(self):
        self._do_test(0)
        files = {
            'fake_se_1_R1.fastq': '836eaf06938d4a41122f284ed487a9c7',
            'fake_se_2_R1.fastq': '836eaf06938d4a41122f284ed487a9c7',
            'fake_se_unknown_R1.fastq': '836eaf06938d4a41122f284ed487a9c7',
        }
        self.assertDictEqual(files, self.get_md5_dict())

    def test_fake_se_0mm_zip(self):
        self._do_test_zip(0)
        files = {
            'fake_se_1_R1.fastq.gz': '3e07353d24a3ecd315067250a6be6047',
            'fake_se_2_R1.fastq.gz': '3e07353d24a3ecd315067250a6be6047',
            'fake_se_unknown_R1.fastq.gz': '3e07353d24a3ecd315067250a6be6047',
        }
        self.assertDictEqual(files, self.get_md5_dict())

    def test_fake_se_1mm(self):
        self._do_test(1, mismatch=1)
        files = {
            'fake_se_1_R1.fastq': '836eaf06938d4a41122f284ed487a9c7',
            'fake_se_2_R1.fastq': '836eaf06938d4a41122f284ed487a9c7',
            'fake_se_unknown_R1.fastq': '836eaf06938d4a41122f284ed487a9c7',
        }
        self.assertDictEqual(files, self.get_md5_dict())

    def test_fake_se_1mm_zip(self):
        self._do_test_zip(1, mismatch=1)
        files = {
            'fake_se_1_R1.fastq.gz': '3e07353d24a3ecd315067250a6be6047',
            'fake_se_2_R1.fastq.gz': '3e07353d24a3ecd315067250a6be6047',
            'fake_se_unknown_R1.fastq.gz': '3e07353d24a3ecd315067250a6be6047',
        }
        self.assertDictEqual(files, self.get_md5_dict())

    def test_fake_se_2mm(self):
        self._do_test(2)
        files = {
            'fake_se_1_R1.fastq': 'd41d8cd98f00b204e9800998ecf8427e',
            'fake_se_2_R1.fastq': 'd41d8cd98f00b204e9800998ecf8427e',
            'fake_se_unknown_R1.fastq': 'a6de105b6c5abbc2d0d16440333adc64',
        }
        self.assertDictEqual(files, self.get_md5_dict())

    def test_fake_se_2mm_zip(self):
        self._do_test_zip(2)
        files = {
            'fake_se_1_R1.fastq.gz': '4a4dd3598707603b3f76a2378a4504aa',
            'fake_se_2_R1.fastq.gz': '4a4dd3598707603b3f76a2378a4504aa',
            'fake_se_unknown_R1.fastq.gz': 'ee6979b139dbd898f058fd7649f87da2',
        }
        self.assertDictEqual(files, self.get_md5_dict())


class TestRegressions(AxeTest):
    """Edge cases that used to crash, silently drop reads or give wrong output.
    Inputs are generated on the fly."""

    BARCODES = "Barcode\tID\nATCACG\tA\nCGATGT\tB\n"
    COMBO_BARCODES = "ATCACG\tCCCC\tA\nCGATGT\tGGGG\tB\n"

    def __init__(self, methodName='runTest'):
        super(TestRegressions, self).__init__(methodName)

    def setUp(self):
        super(TestRegressions, self).setUp()
        self.tmp = tempfile.mkdtemp()
        self.barcodes = self.write("bc.tsv", self.BARCODES)
        self.combo_barcodes = self.write("cbc.tsv", self.COMBO_BARCODES)
        self.prefix = path.join(self.out, "reg")

    def tearDown(self):
        shutil.rmtree(self.tmp)
        super(TestRegressions, self).tearDown()

    def write(self, name, text):
        fpath = path.join(self.tmp, name)
        with open(fpath, "w") as fh:
            fh.write(text)
        return fpath

    def fastq(self, name, reads):
        text = "".join("@{}\n{}\n+\n{}\n".format(n, s, "I" * len(s))
                       for n, s in reads)
        return self.write(name, text)

    def read_out(self, name):
        with open(path.join(self.out, name)) as fh:
            return fh.read()

    def run_axe(self, *args):
        proc = sp.run([self.axe, "-q"] + list(args), stdout=sp.PIPE,
                      stderr=sp.STDOUT)
        return proc.returncode, proc.stdout.decode(errors="replace")

    def test_read_equals_barcode(self):
        # Used to run off the end of the read buffer
        infq = self.fastq("in.fq", [("r1", "ATCACG"), ("r2", "CGATGT")])
        rc, out = self.run_axe("-f", infq, "-F", self.prefix,
                               "-b", self.barcodes)
        self.assertEqual(rc, 0, out)
        # No sequence left after the barcode, so a single N is written
        self.assertEqual(self.read_out("reg_A_R1.fastq"), "@r1\nN\n+\nI\n")
        self.assertEqual(self.read_out("reg_B_R1.fastq"), "@r2\nN\n+\nI\n")
        self.assertEqual(self.read_out("reg_unknown_R1.fastq"), "")

    def test_read_shorter_than_barcode(self):
        infq = self.fastq("in.fq", [("r1", "ATCA"), ("r2", "")])
        rc, out = self.run_axe("-f", infq, "-F", self.prefix,
                               "-b", self.barcodes)
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.read_out("reg_A_R1.fastq"), "")
        self.assertIn("@r1\nATCA\n", self.read_out("reg_unknown_R1.fastq"))

    def test_invalid_first_base(self):
        # Used to skip the unmatched first base and match the barcode at 1
        infq = self.fastq("in.fq", [("r1", "XATCACGTTTT"),
                                    ("r2", "ATCACGTTTT")])
        rc, out = self.run_axe("-f", infq, "-F", self.prefix,
                               "-b", self.barcodes)
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.read_out("reg_A_R1.fastq"),
                         "@r2\nTTTT\n+\nIIII\n")
        self.assertEqual(self.read_out("reg_unknown_R1.fastq"),
                         "@r1\nXATCACGTTTT\n+\nIIIIIIIIIII\n")

    def test_malformed_record_is_an_error(self):
        # Quality shorter than sequence in 2nd record. Used to silently
        # stop processing and exit 0.
        text = "@r1\nATCACGTT\n+\nIIIIIIII\n@r2\nATCACGTT\n+\nIII\n"
        infq = self.write("in.fq", text +
                          "@r3\nATCACGTT\n+\nIIIIIIII\n")
        rc, out = self.run_axe("-f", infq, "-F", self.prefix,
                               "-b", self.barcodes)
        self.assertNotEqual(rc, 0, out)

    def test_paired_files_of_unequal_length(self):
        r1 = self.fastq("r1.fq", [("a", "ATCACGTT"), ("b", "ATCACGTT")])
        r2 = self.fastq("r2.fq", [("a", "TTTT")])
        rc, out = self.run_axe("-f", r1, "-r", r2, "-F", self.prefix,
                               "-R", self.prefix, "-b", self.barcodes)
        self.assertNotEqual(rc, 0, out)

    def test_two_mismatches(self):
        # 2+ mismatches used to fail with "already in trie"
        infq = path.join(self.data, "fake_2mm_R1.fq.gz")
        rc, out = self.run_axe("-m", "2", "-f", infq, "-F", self.prefix,
                               "-b", path.join(self.data, "fake.barcodes"))
        self.assertEqual(rc, 0, out)
        files = self.get_md5_dict()
        self.assertEqual(files["reg_1_R1.fastq"], files["reg_2_R1.fastq"])
        self.assertEqual(files["reg_1_R1.fastq"],
                         '836eaf06938d4a41122f284ed487a9c7')

    def test_long_mismatch_option(self):
        # Used to segfault on `--mismatch 1`
        infq = path.join(self.data, "fake_1mm_R1.fq.gz")
        rc, out = self.run_axe("--mismatch", "1", "-f", infq,
                               "-F", self.prefix,
                               "-b", path.join(self.data, "fake.barcodes"))
        self.assertEqual(rc, 0, out)
        files = self.get_md5_dict()
        self.assertEqual(files["reg_1_R1.fastq"],
                         '836eaf06938d4a41122f284ed487a9c7')

    def test_bad_mismatch_value(self):
        infq = self.fastq("in.fq", [("r1", "ATCACGTT")])
        for bad in ("x", "-1", "1x", ""):
            rc, out = self.run_axe("-m", bad, "-f", infq, "-F", self.prefix,
                                   "-b", self.barcodes)
            self.assertNotEqual(rc, 0, out)

    def test_output_directory_prefix(self):
        # `-F dir` (no trailing slash) used to write `dirA_R1.fastq`
        outdir = path.join(self.out, "outdir")
        os.makedirs(outdir)
        infq = self.fastq("in.fq", [("r1", "ATCACGTT")])
        for prefix in (outdir, outdir + "/"):
            rc, out = self.run_axe("-f", infq, "-F", prefix,
                                   "-b", self.barcodes)
            self.assertEqual(rc, 0, out)
            self.assertEqual(sorted(os.listdir(outdir)),
                             ["A_R1.fastq", "B_R1.fastq",
                              "unknown_R1.fastq"])
            self.assertEqual(os.listdir(self.out), ["outdir"])

    def test_trim_r2_short_mate(self):
        # R2 shorter than the barcode used to underflow, then crash
        r1 = self.fastq("r1.fq", [("a", "ATCACGTTTT")])
        r2 = self.fastq("r2.fq", [("a", "AA")])
        rc, out = self.run_axe("-2", "-f", r1, "-r", r2, "-F", self.prefix,
                               "-R", self.prefix, "-b", self.barcodes)
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.read_out("reg_A_R1.fastq"),
                         "@a\nTTTT\n+\nIIII\n")
        self.assertEqual(self.read_out("reg_A_R2.fastq"), "@a\nN\n+\nI\n")

    def test_trim_r2_leaves_mate_untrimmed_by_default(self):
        r1 = self.fastq("r1.fq", [("a", "ATCACGTTTT")])
        r2 = self.fastq("r2.fq", [("a", "AA")])
        rc, out = self.run_axe("-f", r1, "-r", r2, "-F", self.prefix,
                               "-R", self.prefix, "-b", self.barcodes)
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.read_out("reg_A_R2.fastq"),
                         "@a\nAA\n+\nII\n")

    def test_combo_short_reads(self):
        # Read == barcode used to over-read; truncation used to underflow
        r1 = self.fastq("r1.fq", [("a", "ATCACG"), ("b", "CGATGTAAA")])
        r2 = self.fastq("r2.fq", [("a", "CCCCTTT"), ("b", "GGGG")])
        rc, out = self.run_axe("-c", "-f", r1, "-r", r2, "-F", self.prefix,
                               "-R", self.prefix, "-b", self.combo_barcodes)
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.read_out("reg_A_R1.fastq"), "@a\nN\n+\nI\n")
        self.assertEqual(self.read_out("reg_A_R2.fastq"),
                         "@a\nTTT\n+\nIII\n")
        self.assertEqual(self.read_out("reg_B_R1.fastq"),
                         "@b\nAAA\n+\nIII\n")
        self.assertEqual(self.read_out("reg_B_R2.fastq"), "@b\nN\n+\nI\n")

    def test_conflicting_barcodes(self):
        # Barcodes 1 mismatch apart can't be separated with -m 1
        bcs = self.write("close.tsv", "AAAAAA\tA\nAAAAAC\tB\n")
        infq = self.fastq("in.fq", [("a", "AAAAAATTT"), ("b", "AAAAACTTT"),
                                    ("tie", "AAAAAGTTT"), ("a1", "CAAAAATTT")])
        rc, out = self.run_axe("-m", "1", "-f", infq, "-F", self.prefix,
                               "-b", bcs)
        self.assertNotEqual(rc, 0, out)
        # With --permissive, exact matches stay exact, and only unambiguous
        # mutants are matched.
        rc, out = self.run_axe("-m", "1", "-p", "-f", infq, "-F", self.prefix,
                               "-b", bcs)
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.read_out("reg_A_R1.fastq"),
                         "@a\nTTT\n+\nIII\n@a1\nTTT\n+\nIII\n")
        self.assertEqual(self.read_out("reg_B_R1.fastq"),
                         "@b\nTTT\n+\nIII\n")
        self.assertEqual(self.read_out("reg_unknown_R1.fastq"),
                         "@tie\nAAAAAGTTT\n+\nIIIIIIIII\n")

    def test_combo_close_barcodes(self):
        # R1 barcodes within 1 mismatch of each other. B's exact barcode was
        # once taken for a mutant of A, and so assigned to A.
        bcs = self.write("cbc.tsv", "ATCACG\tCCCC\tA\nATCACT\tGGGG\tB\n")
        r1 = self.fastq("r1.fq", [("a", "ATCACGAA"), ("b", "ATCACTAA")])
        r2 = self.fastq("r2.fq", [("a", "CCCCTT"), ("b", "GGGGTT")])
        rc, out = self.run_axe("-c", "-m", "1", "-p", "-f", r1, "-r", r2,
                               "-F", self.prefix, "-R", self.prefix,
                               "-b", bcs)
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.read_out("reg_A_R1.fastq"),
                         "@a\nAA\n+\nII\n")
        self.assertEqual(self.read_out("reg_B_R1.fastq"),
                         "@b\nAA\n+\nII\n")

    def test_barcode_file_quirks(self):
        # blank lines, comments, lower case and CRLF line endings are fine
        bcs = self.write("q.tsv", "Barcode\tID\r\n\n# note\natcacg\tA\r\n"
                         "\nCGATGT\tB\n\n")
        infq = self.fastq("in.fq", [("r1", "ATCACGTT")])
        rc, out = self.run_axe("-f", infq, "-F", self.prefix, "-b", bcs)
        self.assertEqual(rc, 0, out)
        self.assertEqual(self.read_out("reg_A_R1.fastq"),
                         "@r1\nTT\n+\nII\n")

    def test_bad_barcode_files(self):
        infq = self.fastq("in.fq", [("r1", "ATCACGTT")])
        for text in ("ATCACX\tA\n",                 # not a base
                     "ATCACG\tA\nCGATGT\tA\n",    # duplicate ID
                     "ATCACG\tA\nATCACG\tB\n",    # duplicate barcode
                     "# nothing here\n",             # no barcodes
                     "ATCACG\n"):                    # no ID
            bcs = self.write("bad.tsv", text)
            rc, out = self.run_axe("-f", infq, "-F", self.prefix, "-b", bcs)
            self.assertNotEqual(rc, 0, (text, out))

    def test_barcode_shorter_than_mismatches(self):
        # Used to abort() inside GSL
        bcs = self.write("short.tsv", "AC\tA\nGT\tB\n")
        infq = self.fastq("in.fq", [("r1", "ACTT")])
        rc, out = self.run_axe("-m", "3", "-p", "-f", infq, "-F", self.prefix,
                               "-b", bcs)
        self.assertEqual(rc, 0, out)

    def test_corrupt_gzip_terminates(self):
        # A read error in the middle of a record used to loop forever
        import gzip
        reads = [("r%d" % i, "ATCACG" + "ACGT" * 30) for i in range(5000)]
        raw = path.join(self.tmp, "big.fq")
        self.fastq("big.fq", reads)
        with open(raw, "rb") as fh:
            data = fh.read()
        gz = bytearray(gzip.compress(data))
        mid = len(gz) // 2
        for i in range(mid, mid + 64):
            gz[i] ^= 0xFF
        bad = self.write("bad.fq.gz", "")
        with open(bad, "wb") as fh:
            fh.write(gz)
        proc = sp.run([self.axe, "-q", "-f", bad, "-F", self.prefix,
                       "-b", self.barcodes], stdout=sp.PIPE, stderr=sp.STDOUT,
                      timeout=60)
        self.assertNotEqual(proc.returncode, 0)

    def test_fasta_input_rejected(self):
        fa = self.write("in.fa", ">r1\nATCACGTT\n")
        rc, out = self.run_axe("-f", fa, "-F", self.prefix,
                               "-b", self.barcodes)
        self.assertNotEqual(rc, 0, out)

    def test_empty_input_and_summary(self):
        # No reads: no division by zero in the summary, and a clean exit
        infq = self.write("empty.fq", "")
        proc = sp.run([self.axe, "-f", infq, "-F", self.prefix,
                       "-b", self.barcodes], stdout=sp.PIPE, stderr=sp.STDOUT)
        out = proc.stdout.decode(errors="replace")
        self.assertEqual(proc.returncode, 0, out)
        self.assertNotIn("nan", out.lower())
        self.assertNotIn("inf", out.lower())

    def test_usage_errors(self):
        infq = self.fastq("in.fq", [("r1", "ATCACGTT")])
        proc = sp.run([self.axe], stdout=sp.PIPE, stderr=sp.STDOUT)
        self.assertNotEqual(proc.returncode, 0)
        rc, out = self.run_axe("-z", "12", "-f", infq, "-F", self.prefix,
                               "-b", self.barcodes)
        self.assertNotEqual(rc, 0, out)


if __name__ == '__main__':
    log = logging.getLogger("AxeTest")
    fmt = logging.Formatter('%(message)s')
    cons = logging.StreamHandler()
    cons.setLevel(logging.DEBUG)
    cons.setFormatter(fmt)
    log.addHandler(cons)
    log.setLevel(logging.DEBUG)
    unittest.main()
