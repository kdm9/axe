/*
 * Copyright 2015 Kevin Murray <spam@kdmurray.id.au>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * ============================================================================
 *
 *       Filename:  test_log.c
 *
 *    Description:  Test logging
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "tests.h"
#include <qes_util.h>
#include <qes_log.h>


static void
test_qes_logger_setup(void *ptr)
{
    int res = 0;
    struct qes_logger *logger = NULL;

    (void) ptr;
    /* Test that when created, the logger has logical initial values */
    logger = qes_logger_create();
    tt_ptr_op(logger, !=, NULL);
    tt_ptr_op(logger->destinations, ==, NULL);
    tt_ptr_op(logger->name, ==, NULL);
    tt_int_op(logger->n_destinations, ==, 0);
    tt_int_op(logger->level, ==, QES_LOG_DEBUG);
    tt_int_op(logger->lock, ==, 0);

    /* Add a destination, check it is there */
    res = qes_logger_add_destination(logger, stderr, QES_LOG_ERROR);
    tt_int_op(res, ==, 0);
    tt_ptr_op(logger->destinations, !=, NULL);
    tt_ptr_op(logger->destinations[0].stream, ==, stderr);
    tt_int_op(logger->destinations[0].level, ==, QES_LOG_ERROR);
    tt_int_op(logger->n_destinations, ==, 1);
    qes_logger_destroy(logger);
    tt_ptr_op(logger, ==, NULL);

end:
    if (logger != NULL) {
        if (logger->destinations != NULL) free(logger->destinations);
        if (logger->name != NULL) free(logger->name);
        free(logger);
    }
}


static void
test_qes_logger_init(void *ptr)
{
    int res = 0;
    struct qes_logger *logger = NULL;

    (void) ptr;
    logger = qes_logger_create();
    tt_ptr_op(logger, !=, NULL);

    /* Test the init function sets values correctly */
    res = qes_logger_init(logger, "test logger", QES_LOG_INFO);
    tt_int_op(res, ==, 0);
    tt_ptr_op(logger->name, !=, NULL);
    tt_int_op(logger->level, ==, QES_LOG_INFO);
    tt_int_op(logger->lock, ==, 0);

    /* test that we error out if we give init NULL */
    res = qes_logger_init(NULL, "test logger", QES_LOG_INFO);
    tt_int_op(res, ==, 1);

    /* Test that we silently set logger->name to NULL if we pass NULL */
    res = qes_logger_init(logger, NULL, QES_LOG_INFO);
    tt_int_op(res, ==, 0);
    tt_ptr_op(logger->name, ==, NULL);

end:
    if (logger != NULL) {
        if (logger->destinations != NULL) free(logger->destinations);
        if (logger->name != NULL) free(logger->name);
        free(logger);
    }
}

static void
test_qes_logger_logging(void *ptr)
{
    int res = 0;
    struct qes_logger *logger = NULL;
    char *log_fname = NULL;
    char *truth_fname = NULL;
    FILE *log_file = NULL;

    (void) ptr;
    /* Set up a logger, with an actual file as output */
    logger = qes_logger_create();
    tt_ptr_op(logger, !=, NULL);
    truth_fname = find_data_file("truth/log_test.txt");
    tt_ptr_op(truth_fname, !=, NULL);
    log_fname = get_writable_file();
    tt_ptr_op(log_fname, !=, NULL);
    log_file = fopen(log_fname, "w");
    tt_ptr_op(log_file, !=, NULL);
    res = qes_logger_add_destination(logger, log_file, QES_LOG_INFO);
    tt_int_op(res, ==, 0);

    /* Test printing to the file using the message functions */
    qes_log_message_debug(logger, "Hello World\n"); /* wont' print */
    qes_log_message_info(logger, "Hello World\n"); /* should print */

    /* Test printing to the file with the format functions */
    qes_log_format_debug(logger, "%s\n", "Hello World");
    qes_log_format_info(logger, "%s\n", "Hello World");

    /* Test the output file was created correctly */
    tt_int_op(filecmp(log_fname, truth_fname), ==, 0);

end:
    if (log_file != NULL) fclose(log_file);
    if (truth_fname != NULL) free(truth_fname);
    if (log_fname != NULL) free(log_fname);
    qes_logger_destroy(logger);
}

static void
test_qes_log_entry(void *ptr)
{
    int res = 0;
    struct qes_log_entry *entry = NULL;

    (void) ptr;
    /* Test that create gives us a valid entry with sane defaults */
    entry = qes_log_entry_create();
    tt_ptr_op(entry, !=, NULL);
    tt_ptr_op(entry->message, ==, NULL);
    tt_int_op(entry->level, ==, QES_LOG_DEBUG);

    /* Check that init fills the entry appropriately */
    res = qes_log_entry_init(entry, QES_LOG_ERROR, "Test");
    tt_int_op(res, ==, 0);
    tt_ptr_op(entry, !=, NULL);
    tt_str_op(entry->message, ==, "Test");
    tt_int_op(entry->level, ==, QES_LOG_ERROR);

    /* test that clear  clears an entry, without freeing the entry itself */
    qes_log_entry_clear(entry);
    tt_ptr_op(entry, !=, NULL);
    tt_ptr_op(entry->message, ==, NULL);
    tt_int_op(entry->level, ==, QES_LOG_DEBUG);

    /* test formatting an entry */
    res = qes_log_entry_format(entry, QES_LOG_ERROR, "%s", "Test");
    tt_int_op(res, ==, 0);
    tt_ptr_op(entry, !=, NULL);
    tt_str_op(entry->message, ==, "Test");
    tt_int_op(entry->level, ==, QES_LOG_ERROR);

    qes_log_entry_destroy(entry);
    tt_ptr_op(entry, ==, NULL);

end:
    qes_log_entry_destroy(entry);
}

struct testcase_t qes_log_tests[] = {
    { "qes_logger_setup", test_qes_logger_setup, 0, NULL, NULL},
    { "qes_logger_init", test_qes_logger_init, 0, NULL, NULL},
    { "qes_logger_logging", test_qes_logger_logging, 0, NULL, NULL},
    { "qes_log_entry", test_qes_log_entry, 0, NULL, NULL},
    END_OF_TESTCASES
};
