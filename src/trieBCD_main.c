/*
 * ============================================================================
 *
 *       Filename:  trieBCD_main.c
 *
 *    Description:  Main loop for trieBCD
 *
 *        Version:  1.0
 *        Created:  11/06/14 13:37:00
 *       Revision:  none
 *        License:  GPLv3+
 *       Compiler:  gcc, clang
 *
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */


#include "trieBCD.h"

#include <getopt.h>


static void
print_help()
{

    fprintf(stderr, "All mandatory short options are mandatory in their\n");
    fprintf(stderr, "long option form. Likewise, all short options that take\n");
    fprintf(stderr, "an argument must be given an argument in their long form\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "If a forward read input is given, a forward read output\n");
    fprintf(stderr, "must be. Likewise for a reverse/interleaved input. If either\n");
    fprintf(stderr, "forward and/or reverse reads are given, interleaved input\n");
    fprintf(stderr, "cannot be. However, one can input interleaved paired reads\n");
    fprintf(stderr, "and output seperate forwards and reverse reads, and vice versa.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "The barcode file is a tab-seperated tabular file with an\n");
    fprintf(stderr, "optional header, and has two alternative formats. The standard\n");
    fprintf(stderr, "form (see below) is expected unless --combinatorial is given.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "The standard format is:\n");
    fprintf(stderr, "Barcode\tID\n");
    fprintf(stderr, "ACTA\tA1\n");
    fprintf(stderr, "CCTC\tA2\n");
    fprintf(stderr, "...\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "The combinatorial format is:\n");
    fprintf(stderr, "Barcode1\tBarcode2\tID\n");
    fprintf(stderr, "ACTA\tACGC\tA1\n");
    fprintf(stderr, "CCTC\tTCTA\tA2\n");
    fprintf(stderr, "...\n");
    fprintf(stderr, "\n");
}
static void
print_usage()
{
    fprintf(stderr, "trieBCD\n\n");
    fprintf(stderr, "Demultiplex 5' barcoded reads quickly and accurately.\n\n");
    fprintf(stderr, "USAGE:\n");
    fprintf(stderr, "trieBCD [-mzc2] -b (-f [-r] | -i) (-F [-R] | -C)\n");
    fprintf(stderr, "trieBCD -h\n");
    fprintf(stderr, "trieBCD -v\n\n");
    fprintf(stderr, "OPTIONS:\n");
    fprintf(stderr, "    -m, --mismatch\tMaximum hamming distance mismatch. [int, default 1]\n");
    fprintf(stderr, "    -z, --ziplevel\tGzip compression level, or -1 for plain text [int, default -1]\n");
    fprintf(stderr, "    -c, --combinatorial\tUse combinatorial barcode matching. [flag, default OFF]\n");
    fprintf(stderr, "    -2, --trim-r2\tTrim barcode from R2 read as well as R1. [flag, default OFF]\n");
    fprintf(stderr, "    -b, --barcodes\tBarcode file. See --help for example. [file]\n");
    fprintf(stderr, "    -f, --fwd-in\tInput forward read. [file]\n");
    fprintf(stderr, "    -F, --fwd-out\tOutput forward read prefix. [file]\n");
    fprintf(stderr, "    -r, --rev-in\tInput reverse read. [file]\n");
    fprintf(stderr, "    -R, --rev-out\tOutput reverse read prefix. [file]\n");
    fprintf(stderr, "    -i, --ilfq-in\tInput interleaved paired reads. [file]\n");
    fprintf(stderr, "    -I, --ilfq-out\tOutput interleaved paired reads prefix. [file]\n");
    fprintf(stderr, "    -h, --help\t\tPrint this usage plus additional help.\n");
    fprintf(stderr, "    -v, --version\tPrint version string.\n");
    fprintf(stderr, "\n");
}

static const char *tdb_opts = "m:z:c2b:f:F:r:R:i:I:hv";
static const struct options tbd_longopts[] = {
    { "mismatch",   optional_argument,  NULL,   'm' },
    { "ziplevel",   required_argument,  NULL,   'z' },
    { "combinatorial", no_argument,     NULL,   'c' },
    { "trim-r2",    no_argument,        NULL,   '2' },
    { "barcodes",   required_argument,  NULL,   'b' },
    { "fwd-in",     required_argument,  NULL,   'f' },
    { "fwd-out",    required_argument,  NULL,   'F' },
    { "rev-in",     required_argument,  NULL,   'r' },
    { "rev-out",    required_argument,  NULL,   'R' },
    { "ilfq-in",    required_argument,  NULL,   'i' },
    { "ilfq-out",   required_argument,  NULL,   'I' },
    { "help",       no_argument,        NULL,   'h' },
    { "version",    no_argument,        NULL,   'V' },
    { NULL,         0,                  NULL,    0  }
};

static int
parse_args(struct tbd_config *config, int argc, const char **argv)
{
    int c = 0;
    int optind = 0;

    if (!tbd_config_ok(config) || argc < 1 || argv == NULL) {
        return -1;
    }
    while ((c = getopt_long(argc, argv, tbd_opts, tbd_longopts, &optind)) > 0){
        ...
    }
    config->have_cli_opts = 1;
    return 2;
}

int
main (int argc, const char **argv)
{
    int ret = 0;
    struct tbd_config *config = tbd_config_create();

    if (config == NULL) {
        ret = EXIT_FAILURE;
        goto end;
    }
    ret = parse_args(config, argc, argv);
    if (ret != 0) {
        print_usage();
        if (ret == 2) {
            print_help();
        }
        goto end;
    }
    ret = tbd_read_barcodes(config);
    if (ret != 0) goto end;
    ret = tbd_load_tries_and_make_ouputs(config);
    if (ret != 0) goto end;
end:
    tbd_config_destroy(config);
    return ret;
}
