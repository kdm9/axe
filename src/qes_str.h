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
 *       Filename:  qes_str.h
 *
 *    Description:  String handling functions
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#ifndef QES_STR_H
#define QES_STR_H

#include <qes_util.h>

struct qes_str {
    char *str;
    size_t len;
    size_t capacity;
};


/*===  FUNCTION  ============================================================*
Name:           qes_str_ok
Parameters:     const struct qes_str *str: `struct qes_str` ref to check.
Description:    Checks if a string is OK to use, i.e. checks that `str->str` is
                a valid `char` array.
Returns:        An `int` that evaluates to true or false.
 *===========================================================================*/
static inline int
qes_str_ok (const struct qes_str *str)
{
    return str != NULL && str->str != NULL && str->capacity > 0;
}

/*===  FUNCTION  ============================================================*
Name:           qes_str_init
Parameters:     struct qes_str *str: String to initialise (normally on the
                                     stack).
                size_t len: Initial capacity of `struct qes_str`.
Description:    Initialise a `struct qes_str` (typically on the stack) with
                room for `len` characters.
Returns:        void
 *===========================================================================*/
static inline void
qes_str_init (struct qes_str *str, size_t capacity)
{
    if (str == NULL) return;
    str->len = 0;
    str->str = qes_calloc(capacity, sizeof(*str->str));
    str->capacity = capacity;
}

/*===  FUNCTION  ============================================================*
Name:           qes_str_create
Parameters:     size_t len: Initial capacity of `struct qes_str`.
Description:    Create a `struct qes_str` on the heap with room for `len`
                characters.
Returns:        `struct qes_str *` Pointer to a memory address.
 *===========================================================================*/
static inline struct qes_str *
qes_str_create (size_t capacity)
{
    struct qes_str *str = qes_calloc(1, sizeof(*str));

    /* We don't worry about NULL here. qes_str_init will return before
     * derefencing and we'll return NULL below. */
    qes_str_init(str, capacity);
    return str;
}

static inline int
qes_str_resize (struct qes_str *str, size_t len)
{
    if (str == NULL) return 0;
    if (str->capacity < len + 1) {
        while (str->capacity < len + 1) {
            str->capacity = qes_roundupz(str->capacity);
        }
        str->str = qes_realloc(str->str, str->capacity * sizeof(*str->str));
    }
    return 1;
}

static inline int
qes_str_fill_charptr (struct qes_str *str, const char *cp, size_t len)
{
    if (str == NULL || cp == NULL) return 0;
    if (len == 0) {
        len = strlen(cp);
    }
    qes_str_resize(str, len);
    memcpy(str->str, cp, len);
    str->str[len] = '\0';
    str->len = len;
    return 1;
}

/*===  FUNCTION  ============================================================*
Name:           qes_str_nullify
Parameters:     struct qes_str *str: `struct qes_str` to nullify.
Description:    Invalidates a `struct qes_str` without freeing the `char []`.
Returns:        int: 0 on success, otherwise 1.
 *===========================================================================*/
static inline int
qes_str_nullify (struct qes_str *str)
{
    if (!qes_str_ok(str)) return 1;
    str->str[0] = '\0';
    str->len = 0;
    return 0;
}

static inline int
qes_str_copy (struct qes_str *dest, const struct qes_str *src)
{
    if (!qes_str_ok(src) || dest == NULL) return 1;
    if (!qes_str_ok(dest)) qes_str_init(dest, src->capacity);
    else qes_str_resize(dest, src->capacity);
    memcpy(dest->str, src->str, src->capacity);
    return 0;
}

static inline int
qes_str_cat (struct qes_str *dest, const struct qes_str *src)
{
    if (!qes_str_ok(src) || dest == NULL) return 1;
    if (!qes_str_ok(dest)) qes_str_init(dest, src->capacity);
    else qes_str_resize(dest, dest->len + src->len);

    memcpy(dest->str + dest->len, src->str, src->len);
    dest->len += src->len;
    dest->str[dest->len] = '\0';
    return 0;
}

static inline int
qes_str_truncate (struct qes_str *str, const size_t at)
{
    if (!qes_str_ok(str)) return 1;
    if (at >= str->len) return 0;
    str->str[at] = '\0';
    str->len = at;
    return 0;
}

extern void qes_str_print (const struct qes_str *str, FILE *stream);

/*===  FUNCTION  ============================================================*
Name:           qes_str_destroy
Parameters:     struct qes_str *: `struct qes_str` to destroy.
Description:    Frees `str->str` and the struct qes_str struct itself.
Returns:        void
 *===========================================================================*/
extern void qes_str_destroy (struct qes_str *str);

/*===  FUNCTION  ============================================================*
Name:           qes_str_destroy_cp
Parameters:     struct qes_str *: String to destrop
Description:    Frees `str->str` without freeing the struct qes_str struct
                itself. For use on `struct qes_str`s allocated on the stack.
Returns:        void
 *===========================================================================*/
extern void qes_str_destroy_cp (struct qes_str *str);

#endif /* QES_STR_H */
