/*
 * ============================================================================
 *
 *       Filename:  test.c
 *
 *    Description:  Tests for axe
 *
 *        Version:  1.0
 *        Created:  20/06/14 17:14:55
 *       Revision:  none
 *        License:  GPLv3+
 *       Compiler:  gcc, clang
 *
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */
#include "tests.h"


struct testgroup_t axe_tests[] = {
    {"libaxe/", core_tests},
    END_OF_GROUPS
};


/*
 * ===  FUNCTION  =============================================================
 *         Name:  main
 *  Description:  Run all tests
 * ============================================================================
 */

int
main (int argc, const char *argv[])
{
    int res;
    int our_argc = argc;
    const char **our_argv = argv;
    char *data_prefix;

    data_prefix = NULL;
    if (argc>1) {
       data_prefix = strdup(argv[1]);
       our_argc -= 1;
       our_argv += 1;
    }
    if (data_prefix == NULL) {
        data_prefix = strdup(".");
    }
    if (access(data_prefix, W_OK | X_OK | R_OK) != 0) {
        fprintf(stderr, "Could not access data prefix dir '%s'\n", data_prefix);
        free(data_prefix);
        exit(EXIT_FAILURE);
    }
    res = tinytest_main(argc-1, argv+1, axe_tests);
    free(data_prefix);
    return res;
}
