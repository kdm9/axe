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
 *       Filename:  helpers.c
 *
 *    Description:  Helpers for tests
 *        License:  GPLv3+
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "helpers.h"


/* This is how we name out output files, n_writables is the num of writable
   files we've made. */
static int n_writables = 0;
/* This holds the prefix. It is set (via extern) in test.c's main func. */
char *data_prefix = NULL;


/*===  FUNCTION  ============================================================*
Name:           find_data_file
Parameters:      filepath: the file, under ./data, to find
Description:    Finds the full path to ``filepath``
Returns:        A ``char *`` on the heap that contains a valid path to the
                file ``filepath``, or NULL on error;
 *===========================================================================*/

char *
find_data_file(const char * filepath)
{
    size_t buflen = 1<<12;
    size_t len = 0;
    char buf[buflen];

    /* Bail out if we can't get the data_prefix. */
    if (data_prefix == NULL) {
        return NULL;
    }
    len = snprintf(buf, buflen, "%s/data/%s", data_prefix, filepath);
    /* File name is too long, return NULL */
    if (len >= buflen) {
        return NULL;
    }
    /* Null terminate AFTER making sure len < buflen */
    buf[len] = '\0';
    /* Check we can access the file. If so, strdup & return */
    if (access(buf, F_OK) == 0) {
        char *ret = malloc(buflen + 1);
        assert(ret);
        return strncpy(ret, buf, buflen);
    }
    /* If we get to here, something's gone wrong. */
    return NULL;
}


/*===  FUNCTION  ============================================================*
Name:           get_writable_file
Parameters:      void
Description:    Gets a temporary file which can be written to, under the data
                directory.
Returns:        A ``char *`` on the heap that contains a valid path to a file
                writable file, or NULL on error;
 *===========================================================================*/

char *
get_writable_file(void)
{
    size_t buflen = 1<<12;
    size_t len = 0;
    char buf[buflen];
    char *ret = NULL;

    /* Bail out if we can't get the data_prefix. */
    if (data_prefix == NULL) {
        return NULL;
    }
    len = snprintf(buf, buflen, "%s/data/%05d", data_prefix, n_writables++);
    /* File name is too long, return NULL */
    if (len >= buflen) {
        return NULL;
    }
    /* Null terminate AFTER making sure len < buflen */
    buf[len] = '\0';
    /* Check we can access the file. If so, strdup & return */
    ret = malloc(buflen + 1);
    assert(ret);
    return strncpy(ret, buf, buflen);
}


/*===  FUNCTION  ============================================================*
Name:           clean_writable_file
Parameters:      char *: filename
Description:    Unlink ``file`` and free the memory holding the path.
Returns:        void
 *===========================================================================*/

void
clean_writable_file(char *filepath)
{
    if (filepath != NULL) {
        remove(filepath);
        free(filepath);
    }
}


/*===  FUNCTION  ============================================================*
Name:           crc32_file
Parameters:      char *: filename
Description:    Calculate (using zlib) crc32 checksum.
Returns:        const char *: The crc32 sum, or NULL on error.
 *===========================================================================*/

char *
crc32_file(const char *filepath)
{
    FILE *fp = NULL;
    size_t buflen = 1<<10;
    size_t len = 0;
    char buffer[buflen];
    uint32_t crc = 0;
    char crcbuf[9];

    /* Open file */
    fp = fopen(filepath, "rb");
    if (fp == NULL) {
        return NULL;
    }
    while (!feof(fp)) {
        len = fread(buffer, 1, buflen, fp);
        crc = crc32_update(crc, buffer, len);
    }
    len = snprintf(crcbuf, 9, "%08x", crc);
    crcbuf[len] = '\0';
    fclose(fp);
    return strdup(crcbuf);
}

int
filecmp(const char *file1, const char *file2)
{
    /* returns: -1 on error, 0 if identical, 1 if not */
    FILE *fp1 = NULL;
    FILE *fp2 = NULL;
    unsigned char *buff1 = NULL;
    unsigned char *buff2 = NULL;
    const size_t buff_size = 65535;
    int retval = 1;

    if (file1 == NULL || file2 == NULL) return -1;
    fp1 = fopen(file1, "r");
    fp2 = fopen(file2, "r");
    buff1 = malloc(buff_size);
    buff2 = malloc(buff_size);
    if (fp1 == NULL || fp2 == NULL || buff1 == NULL || buff2 == NULL) {
        retval = -1;
        goto exit;
    }

    while (!feof(fp1) && !feof(fp2)) {
        size_t bytes_read = 0;
        size_t res1 = 0;
        size_t res2 = 0;
        res1 = fread(buff1, 1, buff_size, fp1);
        res2 = fread(buff2, 1, buff_size, fp2);
        if (ferror(fp1) != 0 || ferror(fp2) != 0) {
            retval = -1;
            goto exit;
        }
        if (res1 != res2) {
            retval = 1;
            goto exit;
        }
        if (res1 == 0 && res2 == 0) {
            break;
        }
        bytes_read += res1;
        if (memcmp(buff1, buff2, res1) == 0) retval = 0;
    }
exit:
    if (buff1 != NULL) free(buff1);
    if (fp1 != NULL) fclose(fp1);
    if (buff2 != NULL) free(buff2);
    if (fp2 != NULL) fclose(fp2);
    return retval;
}
