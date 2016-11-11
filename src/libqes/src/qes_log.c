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
 *       Filename:  qes_log.c
 *
 *    Description:  Logging module of libngs2
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "qes_log.h"


struct qes_logger *
qes_logger_create(void)
{
    return qes_calloc(1, sizeof(struct qes_logger));
}

int
qes_logger_init(struct qes_logger  *logger,
                const char         *name,
                enum qes_log_level  level)
{
    if (logger == NULL) return 1;

    if (name != NULL) {
        logger->name = strdup(name);
    } else if (logger->name != NULL) {
        free(logger->name);
        logger->name = NULL;
    }
    logger->level = level;
    return 0;
}

int
qes_logger_add_destination_formatted(struct qes_logger   *logger,
                                     FILE                *stream,
                                     enum qes_log_level   level,
                               char *(*formatter)(struct qes_log_entry *entry))
{
    struct qes_log_destination *new = NULL;
    size_t new_sz = logger->n_destinations + 1;

    new = qes_realloc(logger->destinations,
                      new_sz * sizeof(*logger->destinations));
    if (new == NULL) {
        return 1;
    }
    logger->destinations = new;
    logger->n_destinations = new_sz;
    /* For ease of reference, save the ptr to the (new) final struct in
     * the array */
    new = &new[new_sz - 1];
    new->stream = stream;
    new->level = level;
    new->formatter = formatter;
    return 0;
}

void
_qes_logger_destroy(struct qes_logger  *logger)
{
    if (logger != NULL) {
        qes_free(logger->name);
        qes_free(logger->destinations);
        qes_free(logger);
    }
}

struct qes_log_entry *
qes_log_entry_create(void)
{
    return qes_calloc(1, sizeof(struct qes_log_entry));
}

int
qes_log_entry_init(struct qes_log_entry        *entry,
                   enum qes_log_level           level,
                   const char                  *message)
{
    if (entry == NULL || message == NULL) return -1;

    entry->level = level;
    entry->message = strdup(message);
    return 0;
}

int
qes_log_entry_format_va(struct qes_log_entry   *entry,
                        enum qes_log_level      level,
                        const char             *format,
                        va_list                 args)
{
    int res = 0;
    char *message = NULL;

    /* Format the error message w/ user input */
    res = vasprintf(&message, format, args);
    if (res < 1) {
        /* Alloc inside vasprintf failed */
        return 1;
    }
    /* Make the entry struct */
    res = qes_log_entry_init(entry, level, message);
    free(message);
    return res;
}

int
qes_log_entry_format(struct qes_log_entry  *entry,
                     enum qes_log_level     level,
                     const char            *format,
                     ...)
{
    va_list args;
    int res = 0;

    /* Format the error message w/ user input */
    va_start(args, format);
    res = qes_log_entry_format_va(entry, level, format, args);
    va_end(args);
    return res;
}

void
_qes_log_entry_destroy(struct qes_log_entry    *entry)
{
    if (entry != NULL) {
        qes_free(entry->message);
        qes_free(entry);
    }
}
void qes_log_entry_clear(struct qes_log_entry *entry)
{
    if (entry != NULL) {
        qes_free(entry->message);
        entry->level = QES_LOG_DEBUG;
    }
}

int
qes_logger_write_entry(struct qes_logger      *logger,
                       struct qes_log_entry   *entry)
{
    size_t iii;
    int res;

    if (logger == NULL || entry == NULL) return 1;

    /* Message is to unimportant for this logger */
    if (logger->level > entry->level) return 0;

    for (iii = 0; iii < logger->n_destinations; iii++) {
        char *formatted = NULL;
        struct qes_log_destination *dest = &logger->destinations[iii];

        /* Message is to unimportant for this destination */
        if (dest->level > entry->level) continue;

        formatted = dest->formatter(entry);
        if (formatted == NULL) return 1;
        res = fprintf(dest->stream, "%s", formatted);
        fflush(dest->stream);
        qes_free(formatted);
        if (res < 0) return 1;
    }
    return 0;
}

int
qes_log_message(struct qes_logger      *logger,
                enum qes_log_level      level,
                const char             *message)
{
    struct qes_log_entry entry;
    int res = 0;

    res = qes_log_entry_format(&entry, level, "%s", message);
    if (res != 0) return res;
    res = qes_logger_write_entry(logger, &entry);
    qes_log_entry_clear(&entry);
    return res;
}

int
qes_log_format(struct qes_logger       *logger,
               enum qes_log_level       level,
               const char              *format,
               ...)
{
    struct qes_log_entry entry;
    va_list args;
    int res = 0;

    va_start(args, format);
    res = qes_log_entry_format_va(&entry, level, format, args);
    va_end(args);
    if (res != 0) return res;
    res = qes_logger_write_entry(logger, &entry);
    qes_log_entry_clear(&entry);
    return res;
}

char *
qes_log_formatter_plain(struct qes_log_entry *entry)
{
    /* In the plain-text case, we just pass the message as is. */
    if (entry == NULL) return NULL;
    if (entry->message == NULL) return NULL;
    return strdup(entry->message);
}

char *
qes_log_formatter_pretty(struct qes_log_entry *entry)
{
    char *buf = NULL;
    const char *colour = ANSIRST;
    const char *reset = ANSIRST;
    char marker = ' ';
    int res = 0;

    if (entry == NULL || entry->message == NULL) return NULL;

    if (entry->level <= QES_LOG_DEBUG) {
        marker = '.';
        colour = ANSIBEG ATDIM FGCYN BGBLK ANSIEND;
        reset = "";
    } else if (entry->level <= QES_LOG_INFO) {
        marker = '*';
        colour = ANSIBEG ATNRM FGGRN BGBLK ANSIEND;
    } else if (entry->level <= QES_LOG_WARNING) {
        marker = '!';
        colour = ANSIBEG ATULN FGYEL BGBLK ANSIEND;
    } else if (entry->level <= QES_LOG_ERROR) {
        marker = 'E';
        colour = ANSIBEG ATBLD FGMAG BGBLK ANSIEND;
    } else {
        marker = 'F';
        colour = ANSIBEG ATBLD ATBNK FGRED BGBLK ANSIEND;
    }
    res = asprintf(&buf, "%s[%c] %s%s", colour, marker, entry->message, reset);
    if (res > 0) {
        return buf;
    } else {
        return NULL;
    }
}
