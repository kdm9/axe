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

    fprintf(stderr, "Output files are named according to either the read\n");
    fprintf(stderr, "header's identifier. \n");
}
static void
print_usage()
{
    fprintf(stderr, "trieBCD\n\n");
    fprintf(stderr, "Demultiplex 5' barcoded reads quickly and accurately.\n\n");
    fprintf(stderr, "USAGE:\n");
    fprintf(stderr, "trieBCD [-mz] -b (-f [-r] | -c) (-F [-R] | -C)\n\n");
    fprintf(stderr, "OPTIONS:\n");
    fprintf(stderr, "    -f, --fwd-in\tInput forward read.\n");
    fprintf(stderr, "    -F, --fwd-out\tOutput forward read prefix.\n");
    fprintf(stderr, "    -r, --rev-in\tInput reverse read.\n");
    fprintf(stderr, "    -R, --rev-out\tOutput reverse read prefix.\n");
    fprintf(stderr, "    -c, --combo-in\tInput combo read.\n");
    fprintf(stderr, "    -C, --combo-out\tOutput combo read prefix.\n");
}

static int
parse_args(struct tbd_config *config, int argc, const char **argv)
{
    return 1;
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
        print_help();
        goto end;
    }
    ret = tbd_make_barcode_tries(config);
    if (ret != 0) goto end;
end:
    tbd_config_destroy(config);
    return ret;
}
