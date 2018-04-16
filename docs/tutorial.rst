************
Axe Tutorial
************

In this tutorial, we'll use Axe to demultiplex some paired-end,
combinatorially-index Genotyping-by-Sequencing reads. The data for this
tutorial is available from figshare:
https://figshare.com/articles/axe-tutorial_tar/6143720 .

Axe should be run as the initial step of any analysis: don't use sequence QC
tools like AdapterRemoval or Trimmomatic before using axe, as indexes may be
trimmed away, or pairing information removed.

Step 0: Download the trial data
-------------------------------

This will download the trial data, and extract it on the fly:

.. code-block:: bash

   curl -LS https://ndownloader.figshare.com/files/11094782 | tar xv

Step 1: prepare a key file
--------------------------

The key file associates index sequences with sample names. A key file can be
prepared in a spreadsheet editor, like LibreOffice Calc, or Excel. The format
is quite strict, and is described in detail in the online usage documentation.

Let's now inspect the keyfile I have provided for the tutorial.

.. code-block:: bash

   head axe-keyfile.tsv


Step 2: Demultiplex with Axe
----------------------------


In this step, we will demultiplex our interleaved input file to per-sample
interleaved output files. To see a full range of Axe's options, please run
``axe-demux -h``, or inspect the online usage documentation.

First, let's inspect the input.

.. code-block:: bash

   zcat axe-tutorial.fastq.gz | head -n 8

Then, we need to ensure that axe has somewhere to put the demultiplexed reads.
Axe outputs one file (or more, depending on pairing) per sample. Axe does so by
appending the sample name to some prefix (as given by the ``-I``, ``-F``,
and/or ``-R`` options). If this prefix is a directory, then sample fastq files
will be created in that sub-directory, but the directory must exist. Let's make
an output directory:

.. code-block:: bash

   mkdir -p output

Now, let's demultiplex the reads!

.. code-block:: bash

   axe-demux -i axe-tutorial.fastq.gz -I output/ \
      -c -b axe-keyfile.tsv -t demux-stats.tsv -z 1

The command above demultiplexes reads from ``axe-tutorial.fastq.gz`` into
separate files under ``output``, based on the combinatorial (``-c``)
sample-to-index-sequence mapping described in ``axe-keyfile.tsv``, and saves a
file of statistics as ``demux-stats.tsv``. Note that we have enabled
compression of output files using the ``-z`` option, in case you don't have
much disk space available. This will make Axe slightly slower.
