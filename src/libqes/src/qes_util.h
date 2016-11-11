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
 ============================================================================
 *
 *       Filename:  qes_util.h
 *
 *    Description:  Wrappers around std library functions
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#ifndef QES_UTIL_H
#define QES_UTIL_H


/* #####   HEADER FILE INCLUDES   ########################################## */
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "qes_config.h"


/*
 * Cross-platform bollocks. Thanks windows.
 */

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#define QES_PATHSEP "\\"
#else
#define QES_PATHSEP "/"
#include <unistd.h>
#endif

/*
 * Misc constants
 */

extern const char *libqes_version;

#define QES_MAX_FN_LEN (1<<16)
/* Size of buffers for file IO */
#define    QES_FILEBUFFER_LEN (16384)
/* Starting point for allocing a char pointer. Set to slightly larger than the
   standard size of whatever you're reading in. */
#define    __INIT_LINE_LEN (128)

/*
 * Macro helpers from tor
 */

/* Expands to a syntactically valid empty statement.  */
#define STMT_NIL (void)0

/* Expands to a syntactically valid empty statement, explicitly (void)ing its
 * argument. */
#define STMT_VOID(a) while (0) { (void)(a); }

/* STMT_BEGIN and STMT_END are used to wrap blocks inside macros so that
 * the macro can be used as if it were a single C statement. */
#ifdef __GNUC__
    #define STMT_BEGIN (void) __extension__ ({
    #define STMT_END })
#elif defined(sun) || defined(__sun__)
    #define STMT_BEGIN if (1) {
    #define STMT_END } else STMT_NIL
#else
    #define STMT_BEGIN do {
    #define STMT_END } while (0)
#endif

/* This can be helpful in some macros, particularly with #pragma */
#ifndef STRINGIFY
    #define STRINGIFY(a) #a
#endif

/*
 * Error handling functions
 */

/* use the stdlib exit function by default, during testing we can #define this
 * to some kind of error handler if we need to. */
#ifndef  QES_EXIT_FN
    #define QES_EXIT_FN exit
#endif


/* By default, we use this error handler. At compile or include time, we can
 * chose another more appropriate one if we need to. */
#ifndef  QES_DEFAULT_ERR_FN
    #define QES_DEFAULT_ERR_FN errprintexit
#endif


#define QES_ERRFN_ARGS const char *msg,  const char *file, int line, ...
void errnil(QES_ERRFN_ARGS);
void errprint (QES_ERRFN_ARGS);
void errprintexit (QES_ERRFN_ARGS)  __attribute__ ((noreturn));
typedef void (*qes_errhandler_func) (const char*, const char *, int, ...);

/* qes_roundupz:
 *   Round up a `size_t` to the next highest power of two.
 */
/* Flogged from http://stackoverflow.com/a/1322548 and
   http://graphics.stanford.edu/~seander/bithacks.html, and kseq.h */
static inline size_t
qes_roundupz (size_t sz)
{
    /* Decrement v only if v is not already a power of 2 */
    /* I.e, roundup things already a power of 2 */
    if ((sz & (sz - 1)) != 0) sz--;
    /* mask all bits below MSB to 1 */
    sz |= sz>>1;
    sz |= sz>>2;
    sz |= sz>>4;
    sz |= sz>>8;
    sz |= sz>>16;
#if UINTPTR_MAX == 0xffffffffffffffff /* 64-bit system */
    sz |= sz>>32;
#endif
    return sz + 1;
}

static inline uint32_t
qes_roundup32 (uint32_t u32)
{
    /* Roundup things already a power of 2 */
    if ((u32 & (u32 - 1)) != 0) u32--;
    /* mask all bits below MSB to 1 */
    u32 |= u32>>1;
    u32 |= u32>>2;
    u32 |= u32>>4;
    u32 |= u32>>8;
    u32 |= u32>>16;
    return u32 + 1;
}

static inline uint64_t
qes_roundup64 (uint64_t u64)
{
    /* Roundup things already a power of 2 */
    if ((u64 & (u64 - 1)) != 0) u64--;
    /* mask all bits below MSB to 1 */
    u64 |= u64>>1;
    u64 |= u64>>2;
    u64 |= u64>>4;
    u64 |= u64>>8;
    u64 |= u64>>16;
    u64 |= u64>>32;
    return u64 + 1;
}


/*  INLINE FUNCTIONS */

/* Memory allocation/deallocation */
static inline void *
qes_calloc_ (size_t n, size_t size, qes_errhandler_func onerr, const char *file,
        int line)
{
    void *ret = calloc(n, size);
    if (ret == NULL) {
        (*onerr)("calloc returned NULL -- Out of memory", file, line);
        return NULL;
    } else {
        return ret;
    }
}
#define qes_calloc(n, sz) \
    qes_calloc_(n, sz, QES_DEFAULT_ERR_FN, __FILE__, __LINE__)
#define qes_calloc_errnil(n, sz) \
    qes_calloc_(n, sz, errnil, __FILE__, __LINE__)
#define qes_calloc_errprint(n, sz) \
    qes_calloc_(n, sz, errprint, __FILE__, __LINE__)
#define qes_calloc_errprintexit(n, sz) \
    qes_calloc_(n, sz, errprintexit, __FILE__, __LINE__)

static inline void *
qes_malloc_ (size_t size, qes_errhandler_func onerr, const char *file, int line)
{
    void *ret = malloc(size);
    if (ret == NULL) {
        (*onerr)("malloc returned NULL -- Out of memory", file, line);
        return NULL;
    } else {
        return ret;
    }
}
#define qes_malloc(sz) \
    qes_malloc_(sz, QES_DEFAULT_ERR_FN, __FILE__, __LINE__)
#define qes_malloc_errnil(sz) \
    qes_malloc_(sz, errnil, __FILE__, __LINE__)
#define qes_malloc_errprint(sz) \
    qes_malloc_(sz, errprint, __FILE__, __LINE__)
#define qes_malloc_errprintexit(sz) \
    qes_malloc_(sz, errprintexit, __FILE__, __LINE__)

static inline void *
qes_realloc_ (void *data, size_t size, qes_errhandler_func onerr, const char *file,
        int line)
{
    void *ret = realloc(data, size);
    if (ret == NULL) {
        (*onerr)("realloc returned NULL -- Out of memory", file, line);
        return NULL;
    } else {
        return ret;
    }
}

#define qes_realloc(ptr, sz) \
    qes_realloc_(ptr, sz, QES_DEFAULT_ERR_FN, __FILE__, __LINE__)
#define qes_realloc_errnil(ptr, sz) \
    qes_realloc_(ptr, sz, errnil, __FILE__, __LINE__)
#define qes_realloc_errprint(ptr, sz) \
    qes_realloc_(ptr, sz, errprint, __FILE__, __LINE__)
#define qes_realloc_errprintexit(ptr, sz) \
    qes_realloc_(ptr, sz, errprintexit, __FILE__, __LINE__)
#define qes_free(data)               \
    STMT_BEGIN                      \
    if (data != NULL) {             \
        free(data);                 \
        data = NULL;                \
    }                               \
    STMT_END

#endif /* QES_UTIL_H */
