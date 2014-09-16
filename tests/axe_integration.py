#!/usr/bin/env python
from __future__ import print_function
import logging
import os
from os import path
import re
import shutil
import subprocess as sp
import sys
import unittest


if len(sys.argv) < 2:
    print("USAGE: axe_integration.py $CMAKE_BINARY_DIR")
    exit(-1)

CMAKE_BINARY_DIR = sys.argv.pop(1)


class AxeTest(unittest.TestCase):
    maxDiff=  None

    def __init__(self, methodName='runTest'):
        super(AxeTest, self).__init__(methodName)
        self.data = path.join(CMAKE_BINARY_DIR, "data")
        self.out = path.join(CMAKE_BINARY_DIR, "out", "integration")
        self.axe = path.join(CMAKE_BINARY_DIR, "bin", "axe")
        self.log = logging.getLogger("AxeTest")
        if not path.exists(self.data) or not path.exists(self.axe):
            print("Please run axe_integration.py after compiling axe")
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
                md5 = sp.check_output(["md5sum", path.join(root, fle)])
                md5, fle = md5.strip().split()
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
    files = {
        'fake_se_1_R1.fastq': '836eaf06938d4a41122f284ed487a9c7',
        'fake_se_2_R1.fastq': '836eaf06938d4a41122f284ed487a9c7',
        'fake_se_unknown_R1.fastq': '836eaf06938d4a41122f284ed487a9c7',
    }
    zfiles = {
        'fake_se_1_R1.fastq.gz': '3e07353d24a3ecd315067250a6be6047',
        'fake_se_2_R1.fastq.gz': '3e07353d24a3ecd315067250a6be6047',
        'fake_se_unknown_R1.fastq.gz': '3e07353d24a3ecd315067250a6be6047',
    }

    def __init__(self, methodName='runTest'):
        super(TestFakeSE, self).__init__(methodName)
        self.barcodes = path.join(self.data, "fake.barcodes")
        self.outfq = path.join(self.out, "fake_se")
        self.nobcdfq = path.join(self.out, "fake_se_unknown_R1.fastq")

    def _do_test(self, mm_level):
        infq = path.join(self.data, "fake_{}mm_R1.fq.gz".format(mm_level))
        command = [self.axe,
            "-f", infq,
            "-F", self.outfq,
            '-b', self.barcodes,
        ]
        self.assertTrue(self.run_and_check_stdout(command))

    def _do_test_zip(self, mm_level):
        infq = path.join(self.data, "fake_{}mm_R1.fq.gz".format(mm_level))
        command = [self.axe,
            "-f", infq,
            "-F", self.outfq,
            '-b', self.barcodes,
            '-z', '9',
        ]
        self.assertTrue(self.run_and_check_stdout(command))

    def test_fake_se_0mm(self):
        self._do_test(0)
        self.assertDictEqual(self.files, self.get_md5_dict())

    def test_fake_se_0mm_zip(self):
        self._do_test_zip(0)
        self.assertDictEqual(self.zfiles, self.get_md5_dict())

    def test_fake_se_1mm(self):
        self._do_test(1)
        self.assertDictEqual(self.files, self.get_md5_dict())

    def test_fake_se_1mm_zip(self):
        self._do_test_zip(1)
        self.assertDictEqual(self.zfiles, self.get_md5_dict())

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
        zfiles = {
            'fake_se_1_R1.fastq.gz': '4a4dd3598707603b3f76a2378a4504aa',
            'fake_se_2_R1.fastq.gz': '4a4dd3598707603b3f76a2378a4504aa',
            'fake_se_unknown_R1.fastq.gz': 'ee6979b139dbd898f058fd7649f87da2',
        }
        self.assertDictEqual(zfiles, self.get_md5_dict())


if __name__ == '__main__':
    log = logging.getLogger("AxeTest")
    fmt = logging.Formatter('%(message)s')
    cons = logging.StreamHandler()
    cons.setLevel(logging.DEBUG)
    cons.setFormatter(fmt)
    log.addHandler(cons)
    log.setLevel(logging.DEBUG)
    unittest.main()
