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
 *       Filename:  qes_log.h
 *
 *    Description:  Logging module of libngs2
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#ifndef QES_LOG_H
#define QES_LOG_H

#include <stdlib.h>
#include <stdio.h>
#include <qes_util.h>

#define ANSIBEG  "\033["
#define ANSIEND  "m"

#define ANSIRST  ANSIBEG "0" ANSIEND

#define ATNRM  "0;"
#define ATBLD  "1;"
#define ATDIM  "2;"
#define ATULN  "3;"
#define ATBNK  "5;"
#define ATREV  "7;"
#define ATHID  "8;"

#define FGBLK  "30;"
#define FGRED  "31;"
#define FGGRN  "32;"
#define FGYEL  "33;"
#define FGBLU  "34;"
#define FGMAG  "35;"
#define FGCYN  "36;"
#define FGWHT  "37;"

#define BGBLK  "40"
#define BGRED  "41"
#define BGGRN  "42"
#define BGYEL  "43"
#define BGBLU  "44"
#define BGMAG  "45"
#define BGCYN  "46"
#define BGWHT  "47"

enum qes_log_level {
    /* The idea is that the user can add values between these, if they need
     * to. a la Python's logging module. */
    QES_LOG_DEBUG = 0,
    QES_LOG_INFO = 10,
    QES_LOG_WARNING = 20,
    QES_LOG_ERROR = 30,
    QES_LOG_FATAL = 40,
};

typedef enum qes_log_level QesLogLevel;

struct qes_log_entry {
    char *message;
    enum qes_log_level level;
};

typedef struct qes_log_entry QesLogEntry;
struct qes_log_destination {
    FILE *stream;
    enum qes_log_level level;
    char *(*formatter)(struct qes_log_entry *entry);
};
typedef struct qes_log_destination QesLogDestination;

struct qes_logger {
    struct qes_log_destination *destinations;
    size_t n_destinations;
    enum qes_log_level level;
    char *name;
    int lock;
};
typedef struct qes_logger QesLogger;


struct qes_logger *qes_logger_create(void);
int qes_logger_init(struct qes_logger *logger, const char *name,
                    enum qes_log_level level);
int qes_logger_add_destination_formatted(struct qes_logger *logger,
                                         FILE *stream,
                                         enum qes_log_level level,
                               char *(*formatter)(struct qes_log_entry *entry));
#define qes_logger_add_destination(log, stream, level)                      \
    qes_logger_add_destination_formatted(log, stream, level,                \
                                         &qes_log_formatter_plain)
void _qes_logger_destroy(struct qes_logger *logger);
#define qes_logger_destroy(l) STMT_BEGIN _qes_logger_destroy(l); l = NULL; STMT_END


struct qes_log_entry *qes_log_entry_create(void);
int qes_log_entry_init(struct qes_log_entry *entry, enum qes_log_level level,
                       const char *message);
void qes_log_entry_clear(struct qes_log_entry *entry);

char *qes_log_formatter_plain(struct qes_log_entry *entry);
char *qes_log_formatter_pretty(struct qes_log_entry *entry);

int qes_log_entry_format(struct qes_log_entry *entry, enum qes_log_level level,
                         const char *format, ...);
int qes_log_entry_format_va(struct qes_log_entry *entry,
                            enum qes_log_level level, const char *format,
                            va_list args);
int qes_logger_write_entry(struct qes_logger *logger,
                           struct qes_log_entry *entry);
void _qes_log_entry_destroy(struct qes_log_entry *log_entry);
#define qes_log_entry_destroy(l) STMT_BEGIN _qes_log_entry_destroy(l); l = NULL; STMT_END


int qes_log_message(struct qes_logger *logger, enum qes_log_level level,
                    const char *message);
#ifndef NDEBUG
#define qes_log_message_debug(log, msg) qes_log_message(log, QES_LOG_DEBUG, msg)
#else
#define qes_log_message_debug(log, msg)
#endif
#define qes_log_message_info(log, msg) qes_log_message(log, QES_LOG_INFO, msg)
#define qes_log_message_warning(log, msg) qes_log_message(log, QES_LOG_WARNING, msg)
#define qes_log_message_error(log, msg) qes_log_message(log, QES_LOG_ERROR, msg)
#define qes_log_message_fatal(log, msg) qes_log_message(log, QES_LOG_FATAL, msg)


int qes_log_format(struct qes_logger *logger, enum qes_log_level level,
                   const char *format, ...);
#ifndef NDEBUG
#define qes_log_format_debug(log, fmt, ...) \
        qes_log_format(log, QES_LOG_DEBUG, fmt, __VA_ARGS__)
#else
#define qes_log_format_debug(log, fmt, ...)
#endif
#define qes_log_format_info(log, fmt, ...) \
        qes_log_format(log, QES_LOG_INFO, fmt, __VA_ARGS__)
#define qes_log_format_warning(log, fmt, ...) \
        qes_log_format(log, QES_LOG_WARNING, fmt, __VA_ARGS__)
#define qes_log_format_error(log, fmt, ...) \
        qes_log_format(log, QES_LOG_ERROR, fmt, __VA_ARGS__)
#define qes_log_format_fatal(log, fmt, ...) \
        qes_log_format(log, QES_LOG_FATAL, fmt, __VA_ARGS__)


#endif /* QES_LOG_H */
