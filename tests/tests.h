/*
 * ============================================================================
 *
 *       Filename:  tests.h
 *
 *    Description:  All tests for trieBCD, and all common includes.
 *
 *        Version:  1.0
 *        Created:  20/06/14 17:16:52
 *       Revision:  none
 *        License:  GPLv3+
 *       Compiler:  gcc, clang
 *
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#ifndef TBD_TESTS_H
#define TBD_TESTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>

/* TinyTest */
#include "tinytest/tinytest.h"
#include "tinytest/tinytest_macros.h"

#include <kmutil.h>

#include "trieBCD.h"


/* Core tests */
extern struct testcase_t core_tests[];
#endif /* ifndef TBD_TESTS_H */
