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
 *       Filename:  logdemo.c
 *    Description:  Demontrate libqes logging
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include <qes_log.h>

int
main (int argc, char *argv[])
{
    struct qes_logger *logger = qes_logger_create();

    (void) argc;
    (void) argv;

    qes_logger_init(logger, "Test Logger", QES_LOG_DEBUG);
    qes_logger_add_destination_formatted(logger, stdout, QES_LOG_DEBUG,
                                         &qes_log_formatter_pretty);

    qes_log_message_debug(logger, "Debug message, nice and quiet\n");
    qes_log_message_info(logger, "Informative message, clearer\n");
    qes_log_message_warning(logger, "Warning message, pay attention!\n");
    qes_log_message_error(logger, "Error message, something's gone wrong\n");
    qes_log_message_fatal(logger, "Fatal message, I'm leaving now\n");

    qes_logger_destroy(logger);
    return EXIT_SUCCESS;
}
