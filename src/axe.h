/*
 * ============================================================================
 *
 *       Filename:  axe.h
 *    Description:  Demultiplex reads by 5' barcodes
 *      Copyright:  2014-2015 Kevin Murray <spam@kdmurray.id.au>
 *        License:  GNU GPL v3+
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ============================================================================
 */

#ifndef AXE_H
#define AXE_H

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <time.h>

#include <qes_util.h>
#include <qes_seq.h>
#include <qes_seqfile.h>
#include <qes_log.h>

#include "datrie/trie.h"
#include "datrie/alpha-map.h"
#include "axe_config.h"

/* General rules:
 *  Most functions are declared as `int X(...);`. These functions return:
 *   -1 on parameter error (NULLs, bad values etc)
 *    0 on success
 *    1 on failure
 *  Thus, one can check success with ret = X(...); if (ret != 0) {panic();}
 *  If a function returns a pointer, NULL is the error value.
 */

enum read_mode {
    READS_UNKNOWN = 0,
    READS_SINGLE = 1,
    READS_PAIRED = 2,
    READS_INTERLEAVED = 3,
};

struct axe_output {
    struct qes_seqfile *fwd_file;
    struct qes_seqfile *rev_file;
    enum read_mode mode;
};

struct axe_trie {
    Trie *trie; /* From datrie.h */
    int mismatch_level;
    size_t max_len;
    size_t min_len;
};

struct axe_barcode {
    char *seq1;
    char *seq2;
    char *id;
    size_t len1;
    size_t len2;
    size_t idlen;
    uint64_t count;
};

struct axe_config {
    char *barcode_file;
    char *table_file;
    char *infiles[2];
    char *out_prefixes[2];
    struct axe_barcode **barcodes;
    struct axe_output **outputs;
    /* Array of output files. Access by bcd_lookup[1st_bcd_idx][2nd_bcd_idx]
       Values will be 0 <= x < n_barcode_pairs. barcodes or outputs can then
       be indexed w/ this number */
    ssize_t **barcode_lookup;
    size_t *mismatch_counts;
    size_t n_barcodes_1; /* Number of first read barcodes */
    size_t n_barcodes_2; /* Number of second read barcodes */
    size_t n_barcode_pairs;
    struct axe_output *unknown_output; /* output for unknown files */
    struct axe_trie *fwd_trie;
    struct axe_trie *rev_trie;
    struct qes_logger *logger;
    enum read_mode in_mode;
    enum read_mode out_mode;
    int out_compress_level;
    size_t mismatches;
    uint64_t reads_processed;
    uint64_t reads_demultiplexed;
    uint64_t reads_failed;
    float time_taken;
    int verbosity;
    int have_cli_opts           :1; /* Set to 1 once CLI is parsed */
    int match_combo             :1; /* Match using combinatorial strategy */
    int permissive              :1; /* Don't error on mutated bcd confict */
    int trim_rev                :1; /* Trim rev read same as fwd read */
    int debug                   :1; /* Enable debug mode */
};

extern unsigned int format_call_number;

char *
axe_formatter(struct qes_log_entry *entry);

#define AXE_LOG_PROGRESS 11
#define axe_format_progress(log, fmt, ...)  \
        qes_log_format(log, AXE_LOG_PROGRESS, fmt, __VA_ARGS__)
#define axe_message_progress(log, msg)      \
    qes_log_message(log, AXE_LOG_PROGRESS, msg)

#define AXE_LOG_BOLD 12
#define axe_format_bold(log, fmt, ...)  \
        qes_log_format(log, AXE_LOG_BOLD, fmt, __VA_ARGS__)
#define axe_message_bold(log, msg)      \
    qes_log_message(log, AXE_LOG_BOLD, msg)

static inline int
axe_config_ok(const struct axe_config *config)
{
    if (config == NULL) return 0;
    return 1;
}

static inline int
axe_trie_ok(const struct axe_trie *trie)
{
    if (trie == NULL) return 0;
    if (trie->trie == NULL) return 0;
    if (trie->min_len > trie->max_len) return 0;
    return 1;
}

static inline int
axe_barcode_ok(const struct axe_barcode *barcode)
{
    if (barcode == NULL) return 0;
    if (barcode->seq1 == NULL || barcode->len1 == 0) return 0;
    if (barcode->id == NULL || barcode->idlen == 0) return 0;
    return 1;
}

static inline int
axe_barcode_ok_combo(const struct axe_barcode *barcode)
{
    if (barcode == NULL) return 0;
    if (barcode->seq1 == NULL || barcode->len1 == 0) return 0;
    if (barcode->seq2 == NULL || barcode->len2 == 0) return 0;
    if (barcode->id == NULL || barcode->idlen == 0) return 0;
    return 1;
}

static inline int
axe_output_ok(const struct axe_output *output)
{
    if (output == NULL) return 0;
    if (output->mode == READS_UNKNOWN) return 0;
    if (output->fwd_file == NULL) return 0;
    if (output->mode == READS_PAIRED && output->rev_file == NULL) return 0;
    return 1;
}


/*===  FUNCTION  ============================================================*
Name:           axe_config_create
Parameters:     void
Description:    Create a struct axe_config on the heap and initialise members
                to empty/null values.
Returns:        struct axe_config *: A valid, empty struct axe_config, or NULL
                on any error.
 *===========================================================================*/
struct axe_config *axe_config_create(void);

/*===  FUNCTION  ============================================================*
Name:           axe_config_destroy
Parameters:     struct axe_config *: config struct on heap to destroy.
Description:    Destroy a ``struct axe_config`` on the heap, and set its
                pointer variable to NULL;
Returns:        void
 *===========================================================================*/
void axe_config_destroy_(struct axe_config *config);
#define axe_config_destroy(cfg) STMT_BEGIN                                  \
    axe_config_destroy_(cfg);                                               \
    cfg = NULL;                                                             \
    STMT_END


/*===  FUNCTION  ============================================================*
Name:           axe_output_create
Parameters:     const char *fwd_fpath: Forwards/interleaved read filepath
                const char *rev_fpath: Reverse read filepath
                enum read_mode mode: Output mode
                const char *fp_mode: qes_fopen() mode specifier. See
                    /usr/include/zlib.h for valid values.
Description:    Creates and opens file members of a struct axe_output
Returns:        struct axe_output *: A valid struct axe_output, or NULL on
                failure of any kind
 *===========================================================================*/
struct axe_output *axe_output_create(const char *fwd_fpath,
        const char *rev_fpath, enum read_mode mode, const char *fp_mode);

/*===  FUNCTION  ============================================================*
Name:           axe_output_destroy
Parameters:     struct axe_output *: output struct on heap to destroy.
Description:    Destroy a ``struct axe_output`` on the heap, and set its
                pointer variable to NULL;
Returns:        void
 *===========================================================================*/
void axe_output_destroy_(struct axe_output *output);
#define axe_output_destroy(out) STMT_BEGIN                                  \
    axe_output_destroy_(out);                                               \
    out = NULL;                                                             \
    STMT_END


struct axe_trie *axe_trie_create(void);
extern int axe_trie_get(struct axe_trie *trie, const char *str,
                        intptr_t *data);
extern int axe_trie_add(struct axe_trie *trie, const char *str,
                        intptr_t data);
extern int axe_trie_delete(struct axe_trie *trie, const char *str);
/*===  FUNCTION  ============================================================*
Name:           axe_trie_destroy
Parameters:     struct axe_trie *: trie struct on heap to destroy.
Description:    Destroy a ``struct axe_trie`` on the heap, and set its
                pointer variable to NULL;
Returns:        void
 *===========================================================================*/
void axe_trie_destroy_(struct axe_trie *trie);
#define axe_trie_destroy(trie) STMT_BEGIN                                   \
    axe_trie_destroy_(trie);                                                \
    trie = NULL;                                                            \
    STMT_END


struct axe_barcode *axe_barcode_create(void);
void axe_barcode_destroy_(struct axe_barcode *barcode);
#define axe_barcode_destroy(barcode) STMT_BEGIN                             \
    axe_barcode_destroy_(barcode);                                          \
    barcode = NULL;                                                         \
    STMT_END


/* This is the processing pipeline. These functions should be run in this
   order */
int axe_read_barcodes(struct axe_config *config);
int axe_setup_barcode_lookup(struct axe_config *config);
int axe_make_tries(struct axe_config *config);
int axe_load_tries(struct axe_config *config);
int axe_make_outputs(struct axe_config *config);
int axe_process_file(struct axe_config *config);
int axe_write_table(const struct axe_config *config);
int axe_print_summary(const struct axe_config *config);

/* Libraries or inner functions */
extern int axe_match_read(struct axe_config *config, intptr_t *value,
                          struct axe_trie *trie, const struct qes_seq *seq);
int product(int64_t len, int64_t elem, uintptr_t *choices, int at_start);
char **hamming_mutate_dna(size_t *n_results_o, const char *str, size_t len,
                          unsigned int dist, int keep_original);

extern char _time_now[];
static inline const char *
nowstr(void)
{
    time_t rawtime;

    time(&rawtime);
    strftime(_time_now, 10, "%H:%M:%S", localtime(&rawtime));
    return _time_now;
}


#endif /* AXE_H */
