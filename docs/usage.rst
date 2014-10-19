*********
Axe Usage
*********

Axe has several usage modes. The primary distinction is between the two
alternate barcoding schemes, single and combinatorial barcoding. Single barcode
matching is used when only the first read contains barcode sequences.
Combinatorial barcoding is used when both reads in a read pair contain
independent (typically different) barcode sequences.

For concise reference, the command-line usage of ``axe`` is reproduced below:

.. literalinclude:: usage.txt

Inputs and Outputs
------------------

Regardless of read mode, three input and output schemes are supported:
single-end reads, paired reads (separate R1 and R2 files) and interleaved
paired reads (one file, with R1 and R2 as consecutive reads). If single end
reads are inputted, they must be output as single end reads. If either paired or
interleaved paired reads are read, they can be output as either paired reads or
interleaved paired reads. This applies to both successfully de-multiplexed reads
and reads that could not be de-multiplexed.

The ``-z`` flag can be used to specify that outputs should be compressed using
gzip compression. The ``-z`` flag takes an integer argument between 0 (the
default) and 9, where 0 indicates plain text output (``gzopen`` mode "wT"), and
1-9 indicate that the respective compression level should be used, where 1 is
fastest and 9 is most compact.

The output flags should be prefixes that are used to generate the output file
name based on the barcode's (or barcode pair's) ID. The names are generated as:
``prefix`` + ``_`` + ``barcode ID`` + ``_`` + ``read number`` + ``.extension``.
The output file for reads that could not be demultiplexed is ``prefix`` + ``_``
+ ``unknown`` + ``_`` + ``read number`` + ``.extension``.  The read number is
omitted unless the paired read file scheme is used, and is "il" for interleaved
output. The extension is "fastq"; ".gz" is appended to the extension if the
``-z`` flag is used.

The corresponding CLI flags are:
 - ``-f`` and ``-F``: Single end or paired R1 file input and output
   respectively.
 - ``-r`` and ``-R``: Paired R2 file input and output.
 - ``-i`` and ``-I``: Interleaved paired input and output.

The barcode file
----------------

The barcode file is a tab-separated file with an optional header. It is
mandatory, and is always supplied using the ``-b`` command line flag. The exact
format is dependent on barcoding mode, and is described further in the sections
below. If a header is present, the header line must start with either
`Barcode` or ``barcode``, or it will be interpreted as a barcode line, leading
to a parsing error. Any line starting with ';' or '#' is ignored, allowing
comments to be added in line with barcodes. Please ensure that the software
used to produce the barcode uses ASCII encoding, and does not insert a
Byte-order Mark (BoM) as many text editors can silently use Unicode-based
encoding schemes. I recommend the use of
`LibreOffice Calc <www.libreoffice.org>`_ (part of a free and open source
office suite) to generate barcode tables; Microsoft Excel can also be used.

Mismatch level selection
------------------------

Independent of barcode mode, the ``-m`` flag is used to select the maximum
allowable hamming distance between a read's prefix and a barcode to be
considered as a match. As "mutated" barcodes must be unique, a hamming distance
of one is the default as typically barcodes are designed to differ by a hamming
distance of at least two. Optionally, (using the ``-p`` flag), axe will allow
selective mismatch levels, where, if clashes are observed, the barcode will
only be matched exactly. This allows one to process datasets with barcodes that
don't have a sufficiently high distance between them.

Single barcode mode
-------------------

Single barcode mode is the default mode of operation. Barcodes are matched
against read one (hereafter the forward read), and the barcode is trimmed from
only the forward read, unless the ``-2`` command line flag is given, in which
case a prefix the same length as the matched barcode is also trimmed from the
second or reverse read. Note that sequence of this second read is not checked
before trimming.

In single barcode mode, the barcode file has two columns: ``Barcode`` and
``ID``.

Combinatorial barcode mode
--------------------------

Combinatorial barcode mode is activated by giving the ``-c`` flag on the
command line. Forward read barcodes are matched against the forward read, and
reverse read barcodes are matched against the reverse read. The optimal
barcodes are selected independently, and the barcode pair is selected from
these two barcodes. The respective  barcodes are trimmed from both reads; the
``-2`` command line flag has no effect in combinatorial barcode mode.

In combinatorial barcode mode, the barcode file has three columns:
``Barcode1``, ``Barcode2`` and ``ID``. Individual barcodes can occur many times
within the forward and reverse barcodes, but barcode pairs must be unique
combinations.

The Demultipexing Statistics File
---------------------------------

The ``-t`` option allows the output of per-sample read counts to a
tab-separated file. The file will have a header describing its format, and
includes a line for unbarcoded reads.
