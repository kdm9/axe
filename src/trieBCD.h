/*
 * ============================================================================
 *
 *       Filename:  trieBCD.h
 *
 *    Description:  Demultiplex reads by 5' barcodes
 *
 *        Version:  1.0
 *        Created:  11/06/14 12:19:39
 *       Revision:  none
 *        License:  GPLv3+
 *       Compiler:  gcc, clang
 *
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */
#ifndef TRIEBCD_H
#define TRIEBCD_H

#include <stdint.h>
#include <stdlib.h>

#include <kmutil.h>
#include <kmseq.h>
#include <kmseqfile.h>

#include "datrie/trie.h"
#include "datrie/alpha-map.h"

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

struct tbd_output {
    seqfile_t *fwd_file;
    seqfile_t *rev_file;
    enum read_mode mode;
    uint64_t count;
};

struct tbd_trie {
    Trie *trie; /* From datrie.h */
    int mismatch_level;
    size_t max_len;
    size_t min_len;
};

struct tbd_barcode {
    char *seq1;
    char *seq2;
    char *id;
    size_t len1;
    size_t len2;
    size_t idlen;
    uint64_t count;
};

struct tbd_config {
    char *barcode_file;
    char *infiles[2];
    char *out_prefixes[2];
    char *unknown_files[2];
    struct tbd_barcode **barcodes;
    /* Array of output files. Access by cfg->ofs[1st_bcd_idx][2nd_bcd_idx] */
    struct tbd_output ***outputs;
    size_t n_barcodes_1; /* Number of first read barcodes */
    size_t n_barcodes_2; /* Number of second read barcodes */
    size_t n_barcode_pairs;
    struct tbd_output *unknown_output; /* output for unknown files */
    enum read_mode in_mode;
    enum read_mode out_mode;
    int out_compress_level;
    size_t mismatches;
    struct tbd_trie **fwd_tries;
    struct tbd_trie **rev_tries;
    int have_cli_opts           :1; /* Set to 1 once CLI is parsed */
    int match_combo             :1; /* Match using combinatorial strategy */
    int ignore_barcode_confict  :1;
    int trim_rev                :1;
};

static inline int
tbd_config_ok(const struct tbd_config *config)
{
    if (config == NULL) return 0;
    return 1;
}

static inline int
tbd_trie_ok(const struct tbd_trie *trie)
{
    if (trie == NULL) return 0;
    if (trie->trie == NULL) return 0;
    if (trie->min_len > trie->max_len) return 0;
    return 1;
}

static inline int
tbd_barcode_ok(const struct tbd_barcode *barcode)
{
    if (barcode == NULL) return 0;
    if (barcode->seq1 == NULL || barcode->len1 == 0) return 0;
    if (barcode->id == NULL || barcode->idlen == 0) return 0;
    return 1;
}

static inline int
tbd_barcode_ok_combo(const struct tbd_barcode *barcode)
{
    if (barcode == NULL) return 0;
    if (barcode->seq1 == NULL || barcode->len1 == 0) return 0;
    if (barcode->seq2 == NULL || barcode->len2 == 0) return 0;
    if (barcode->id == NULL || barcode->idlen == 0) return 0;
    return 1;
}

static inline int
tbd_output_ok(const struct tbd_output *output)
{
    if (output == NULL) return 0;
    if (output->mode == READS_UNKNOWN) return 0;
    if (output->fwd_file == NULL) return 0;
    if (output->mode == READS_PAIRED && output->rev_file == NULL) return 0;
    return 1;
}

#if 0
/* #ifndef NDEBUG */
#define TBD_DEBUG_LOG(x) STMT_BEGIN fprintf(stderr, x); STMT_END
#else
#define TBD_DEBUG_LOG(x) (void)(x);
#endif


/*===  FUNCTION  ============================================================*
Name:           tbd_config_create
Paramters:      void
Description:    Create a struct tbd_config on the heap and initialise members
                to empty/null values.
Returns:        struct tbd_config *: A valid, empty struct tbd_config, or NULL
                on any error.
 *===========================================================================*/
struct tbd_config *tbd_config_create(void);

/*===  FUNCTION  ============================================================*
Name:           tbd_config_destroy
Paramters:      struct tbd_config *: config struct on heap to destroy.
Description:    Destroy a ``struct tbd_config`` on the heap, and set its
                pointer variable to NULL;
Returns:        void
 *===========================================================================*/
void tbd_config_destroy_(struct tbd_config *config);
#define tbd_config_destroy(cfg) STMT_BEGIN                                  \
    tbd_config_destroy_(cfg);                                               \
    cfg = NULL;                                                             \
    STMT_END


/*===  FUNCTION  ============================================================*
Name:           tbd_output_create
Paramters:      const char *fwd_fpath: Forwards/interleaved read filepath
                const char *rev_fpath: Reverse read filepath
                enum read_mode mode: Output mode
                const char *fp_mode: zfopen() mode specifier. See
                    /usr/include/zlib.h for valid values.
Description:    Creates and opens file members of a struct tbd_output
Returns:        struct tbd_output *: A valid struct tbd_output, or NULL on
                failure of any kind
 *===========================================================================*/
struct tbd_output *tbd_output_create(const char *fwd_fpath,
        const char *rev_fpath, enum read_mode mode, const char *fp_mode);

/*===  FUNCTION  ============================================================*
Name:           tbd_output_destroy
Paramters:      struct tbd_output *: output struct on heap to destroy.
Description:    Destroy a ``struct tbd_output`` on the heap, and set its
                pointer variable to NULL;
Returns:        void
 *===========================================================================*/
void tbd_output_destroy_(struct tbd_output *output);
#define tbd_output_destroy(out) STMT_BEGIN                                  \
    tbd_output_destroy_(out);                                               \
    out = NULL;                                                             \
    STMT_END


struct tbd_trie *tbd_trie_create(void);

/*===  FUNCTION  ============================================================*
Name:           tbd_trie_destroy
Paramters:      struct tbd_trie *: trie struct on heap to destroy.
Description:    Destroy a ``struct tbd_trie`` on the heap, and set its
                pointer variable to NULL;
Returns:        void
 *===========================================================================*/
void tbd_trie_destroy_(struct tbd_trie *trie);
#define tbd_trie_destroy(trie) STMT_BEGIN                                   \
    tbd_trie_destroy_(trie);                                                \
    trie = NULL;                                                            \
    STMT_END

struct tbd_barcode *tbd_barcode_create(void);
void tbd_barcode_destroy_(struct tbd_barcode *barcode);
#define tbd_barcode_destroy(barcode) STMT_BEGIN                             \
    tbd_barcode_destroy_(barcode);                                          \
    barcode = NULL;                                                         \
    STMT_END



int tbd_read_barcodes(struct tbd_config *config);
int tbd_make_tries(struct tbd_config *config);
int tbd_load_tries(struct tbd_config *config);
int tbd_make_outputs(struct tbd_config *config);
int tbd_trie_add(struct tbd_trie *trie, const char *seq, size_t len);
int tbd_process_file(struct tbd_config *config);
int tbd_print_summary(const struct tbd_config *config, FILE *stream,
                      int verbosity);

extern int tbd_match_read(intptr_t *value, struct tbd_trie *trie,
                          const seq_t *seq);
int product(int64_t len, int64_t elem, uintptr_t *choices, int at_start);
int combinations(int64_t len, int64_t elem, uintptr_t *choices, int at_start);
char **hamming_mutate_dna(size_t *n_results_o, const char *str, size_t len,
                          unsigned int dist, int keep_original);


#endif /* TRIEBCD_H */
