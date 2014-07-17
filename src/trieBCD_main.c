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
#include <time.h>

#include <getopt.h>
static char _time_now[10] = "";

static void
print_version(void)
{
    fprintf(stderr, "trieBCD Version %s\n", TRIEBCD_VERSION);
    exit(0);
}

static void
print_help(void)
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
print_usage(void)
{
    fprintf(stderr, "Demultiplex 5' barcoded reads quickly and accurately.\n\n");
    fprintf(stderr, "USAGE:\n");
    fprintf(stderr, "trieBCD [-mzc2] -b -u [-U] (-f [-r] | -i) (-F [-R] | -I)\n");
    fprintf(stderr, "trieBCD -h\n");
    fprintf(stderr, "trieBCD -v\n\n");
    fprintf(stderr, "OPTIONS:\n");
    fprintf(stderr, "    -m, --mismatch\tMaximum hamming distance mismatch. [int, default 1]\n");
    fprintf(stderr, "    -z, --ziplevel\tGzip compression level, or 0 for plain text [int, default 0]\n");
    fprintf(stderr, "    -c, --combinatorial\tUse combinatorial barcode matching. [flag, default OFF]\n");
    fprintf(stderr, "    -2, --trim-r2\tTrim barcode from R2 read as well as R1. [flag, default OFF]\n");
    fprintf(stderr, "    -b, --barcodes\tBarcode file. See --help for example. [file]\n");
    fprintf(stderr, "    -f, --fwd-in\tInput forward read. [file]\n");
    fprintf(stderr, "    -F, --fwd-out\tOutput forward read prefix. [file]\n");
    fprintf(stderr, "    -r, --rev-in\tInput reverse read. [file]\n");
    fprintf(stderr, "    -R, --rev-out\tOutput reverse read prefix. [file]\n");
    fprintf(stderr, "    -i, --ilfq-in\tInput interleaved paired reads. [file]\n");
    fprintf(stderr, "    -I, --ilfq-out\tOutput interleaved paired reads prefix. [file]\n");
    fprintf(stderr, "    -u, --unknown-r1\tUnknown barcode forward/interleaved read file. [file]\n");
    fprintf(stderr, "    -U, --unknown-r2\tUnknown barcode reverse read file. [file]\n");
    fprintf(stderr, "    -h, --help\t\tPrint this usage plus additional help.\n");
    fprintf(stderr, "    -V, --version\tPrint version string.\n");
    fprintf(stderr, "    -v, --verbose\tBe more verbose. Additive, -vv is more vebose than -v.\n");
    fprintf(stderr, "    -q, --quiet\tBe very quiet.\n");
    fprintf(stderr, "\n");
}

static const char *tbd_opts = "m:z:c2b:f:F:r:R:i:I:t:u:U:hVvq";
static const struct option tbd_longopts[] = {
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
    { "table-file", required_argument,  NULL,   't' },
    { "help",       no_argument,        NULL,   'h' },
    { "version",    no_argument,        NULL,   'V' },
    { "verbose",    no_argument,        NULL,   'v' },
    { NULL,         0,                  NULL,    0  }
};

static int
parse_args(struct tbd_config *config, int argc, char * const *argv)
{
    int c = 0;
    int optind = 0;

    if (!tbd_config_ok(config) || argc < 1 || argv == NULL) {
        goto error;
    }
    /* Set some sane defaults */
    /* Most things will default to 0, and we calloc the config struct, so we
     * don't need to explicity set them. */
    config->mismatches = 1;
    config->verbosity = 0;
    /* Parse argv using getopt */
    while ((c = getopt_long(argc, argv, tbd_opts, tbd_longopts, &optind)) > 0){
        switch (c) {
            case 'm':
                config->mismatches = atol(optarg);
                break;
            case 'z':
                config->out_compress_level = atoi(optarg);
                break;
            case 'c':
                config->match_combo = 1;
                break;
            case '2':
                config->trim_rev = 1;
                break;
            case 'b':
                config->barcode_file = strdup(optarg);
                break;
            case 'f':
                if (config->in_mode == READS_INTERLEAVED) {
                    goto error;
                    break;
                }
                config->infiles[0] = strdup(optarg);
                if (config->in_mode == READS_UNKNOWN) {
                    config->in_mode = READS_SINGLE;
                }
                break;
            case 'F':
                config->out_prefixes[0] = strdup(optarg);
                config->out_mode = READS_SINGLE;
                break;
            case 'r':
                if (config->in_mode == READS_INTERLEAVED) {
                    goto error;
                    break;
                }
                config->infiles[1] = strdup(optarg);
                config->in_mode = READS_PAIRED;
                break;
            case 'R':
                config->out_prefixes[1] = strdup(optarg);
                config->out_mode = READS_PAIRED;
                break;
            case 'u':
                config->unknown_files[0] = strdup(optarg);
                break;
            case 'U':
                config->unknown_files[1] = strdup(optarg);
                break;
            case 't':
                config->table_file = strdup(optarg);
                break;
            case 'h':
                goto help;
            case 'V':
                goto version;
            case 'v':
                config->verbosity += 1;
                break;
            case 'q':
                config->verbosity -= 1;
                break;
            case '?':
            default:
                /* Getopt long prints its own error msg */
                goto error;
        }
    }
    /* Check options are sane */
    if (config->barcode_file == NULL) {
        fprintf(stderr, "ERROR: Barcode file must be provided\n");
        goto error;
    }
    if (config->mismatches > 4) {
        fprintf(stderr, "ERROR: Silly mismatch level %zu\n",
                config->mismatches);
        goto error;
    }
    if (config->in_mode == READS_UNKNOWN) {
        fprintf(stderr, "ERROR: Input file(s) must be provided\n");
        goto error;
    }
    if (config->infiles[0] == NULL) {
        switch (config->in_mode) {
            case READS_SINGLE:
                fprintf(stderr, "ERROR: Setting forward read input file failed.\n");
                break;
            case READS_PAIRED:
                fprintf(stderr, "ERROR: Forward read file must be provided.\n");
                break;
            case READS_INTERLEAVED:
                fprintf(stderr, "ERROR: Setting interleaved input file failed.\n");
                break;
            case READS_UNKNOWN:
            default:
                break;
        }
        goto error;
    }
    if (config->infiles[1] == NULL) {
        switch (config->in_mode) {
            case READS_SINGLE:
            case READS_INTERLEAVED:
                /* Not an error */
                break;
            case READS_PAIRED:
                fprintf(stderr, "ERROR: Setting revese read input file failed.\n");
                goto error;
                break;
                fprintf(stderr, "ERROR: Setting interleaved input file failed.\n");
                goto error;
                break;
            case READS_UNKNOWN:
            default:
                goto error;
                break;
        }
    }
    if (config->infiles[1] != NULL) {
        switch (config->in_mode) {
            case READS_PAIRED:
                /* Not an error */
                break;
            case READS_INTERLEAVED:
                fprintf(stderr, "ERROR: Revese read input file set in interleaved mode.\n");
                goto error;
                break;
            case READS_SINGLE:
            case READS_UNKNOWN:
            default:
                /* Misc weirdness */
                goto error;
                break;
        }
    }
    if (config->out_prefixes[0] == NULL) {
        switch (config->out_mode) {
            case READS_SINGLE:
                fprintf(stderr, "ERROR: Setting forward read output prefix failed.\n");
                break;
            case READS_PAIRED:
                fprintf(stderr, "ERROR: Forward read prefix must be provided.\n");
                break;
            case READS_INTERLEAVED:
                fprintf(stderr, "ERROR: Setting interleaved output prefix failed.\n");
                break;
            case READS_UNKNOWN:
            default:
                break;
        }
        goto error;
    }
    if (config->out_prefixes[1] == NULL) {
        switch (config->out_mode) {
            case READS_SINGLE:
            case READS_INTERLEAVED:
                /* Not an error */
                break;
            case READS_PAIRED:
                fprintf(stderr, "ERROR: Setting revese read output prefix failed.\n");
                goto error;
                break;
                fprintf(stderr, "ERROR: Setting interleaved output prefix failed.\n");
                goto error;
                break;
            case READS_UNKNOWN:
            default:
                goto error;
                break;
        }
    }
    if (config->out_prefixes[1] != NULL) {
        switch (config->out_mode) {
            case READS_PAIRED:
                /* Not an error */
                break;
            case READS_INTERLEAVED:
                fprintf(stderr, "ERROR: Revese read output prefix set in interleaved mode.\n");
                goto error;
                break;
            case READS_SINGLE:
            case READS_UNKNOWN:
            default:
                /* Misc weirdness */
                goto error;
                break;
        }
    }
    if (config->unknown_files[0] == NULL) {
        switch (config->out_mode) {
            case READS_SINGLE:
            case READS_PAIRED:
                fprintf(stderr, "ERROR: Forward read unknown barcode output file must be provided.\n");
                break;
            case READS_INTERLEAVED:
                fprintf(stderr, "ERROR: Interleaved paired read unknown barcode output file must be provided.\n");
                break;
            case READS_UNKNOWN:
            default:
                break;
        }
        goto error;
    }
    if (config->unknown_files[1] == NULL) {
        switch (config->out_mode) {
            case READS_PAIRED:
                fprintf(stderr, "ERROR: Reverse read unknown barcode output file must be provided.\n");
                goto error;
                break;
            case READS_SINGLE:
            case READS_INTERLEAVED:
                break;
            case READS_UNKNOWN:
            default:
                goto error;
                break;
        }
    }
    if (config->unknown_files[1] != NULL) {
        switch (config->out_mode) {
            case READS_PAIRED:
                break;
            case READS_SINGLE:
            case READS_INTERLEAVED:
                fprintf(stderr, "ERROR: Reverse read unknown output file cannot");
                fprintf(stderr, " be provided with interleaved/single end output.\n");
                goto error;
                break;
            case READS_UNKNOWN:
            default:
                goto error;
                break;
        }
    }
    config->have_cli_opts = 1;
    return 0;
version:
    tbd_config_destroy(config);
    print_version();
    return 0;
help:
    config->have_cli_opts = 0;
    return 2;
error:
    config->have_cli_opts = 0;
    return 1;
}

static const char *
now(void)
{
    time_t rawtime;

    time(&rawtime);
    strftime(_time_now, 10, "%H:%M:%S", localtime(&rawtime));
    return _time_now;
}

int
main (int argc, char * const *argv)
{
    int ret = 0;
    struct tbd_config *config = tbd_config_create();

    if (config == NULL) {
        ret = EXIT_FAILURE;
        goto end;
    }
    ret = parse_args(config, argc, argv);
    fprintf(stderr, "[main] Starting demultiplexing at %s\n", now());
    TBD_DEBUG_LOG("[main] CLI args parsed\n");
    if (ret != 0) {
        print_usage();
        if (ret == 2) {
            print_help();
        }
        goto end;
    }
    ret = tbd_read_barcodes(config);
    TBD_DEBUG_LOG("[main] tbd_read_barcodes done\n");
    if (ret != 0) {
        fprintf(stderr, "[main] ERROR: tbd_read_barcodes returned %i\n", ret);
        goto end;
    }
    ret = tbd_make_tries(config);
    TBD_DEBUG_LOG("[main] tbd_make_tries done\n");
    if (ret != 0) {
        fprintf(stderr, "[main] ERROR: tbd_make_tries returned %i\n", ret);
        goto end;
    }
    ret = tbd_load_tries(config);
    TBD_DEBUG_LOG("[main] tbd_load_tries done\n");
    if (ret != 0) {
        fprintf(stderr, "[main] ERROR: tbd_load_tries returned %i\n", ret);
        goto end;
    }
    ret = tbd_make_outputs(config);
    TBD_DEBUG_LOG("[main] tbd_make_outputs done\n");
    if (ret != 0) {
        fprintf(stderr, "[main] ERROR: tbd_make_outputs returned %i\n", ret);
        goto end;
    }
    ret = tbd_process_file(config);
    TBD_DEBUG_LOG("[main] tbd_process_file done\n");
    if (ret != 0) {
        fprintf(stderr, "[main] ERROR: tbd_process_file returned %i\n", ret);
        goto end;
    }
    ret = tbd_print_summary(config, stderr);
    TBD_DEBUG_LOG("[main] tbd_print_summary done\n");
    if (ret != 0) {
        fprintf(stderr, "[main] ERROR: tbd_print_summary returned %i\n", ret);
        goto end;
    }
    ret = tbd_write_table(config);
    TBD_DEBUG_LOG("[main] tbd_write_table done\n");
    if (ret != 0) {
        fprintf(stderr, "[main] ERROR: tbd_write_table returned %i\n", ret);
        goto end;
    }
end:
    tbd_config_destroy(config);
    fprintf(stderr, "[main] Finishing demultiplexing at %s\n", now());
    return ret;
}
