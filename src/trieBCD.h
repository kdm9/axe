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
    void *trie;
    int mismatch_level;
};

struct tbd_config {
    char *barcode_file;
    char *infiles[2];
    char *out_prefixes[2];
    /* Array of output files. Access by cfg->ofs[1st_bcd_idx][2nd_bcd_idx] */
    struct tbd_output ***outputs;
    size_t n_outputs_1; /* Number of first read barcodes */
    size_t n_outputs_2; /* Number of second read barcodes */
    struct tbd_output *unknown_output; /* output for unknown files */
    enum read_mode in_mode;
    enum read_mode out_mode;
    int out_compress_level;
    int mismatches;
    struct tbd_trie **tries;
    size_t n_tries;
    int ok                      :1;
    int name_outfiles_by_seq    :1;
};


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
  Paramters:      struct tbd_config *config:
  Description:    Opens the file.
  Returns:        struct tbd_output *:
 *===========================================================================*/
struct tbd_output *tbd_output_create(struct tbd_config *config);

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

int tbd_trie_create(int mismatch_level);

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


char **hamming_mutate_dna(size_t *n_results_o, const char *str, size_t len,
        int dist, int keep_original);
int product(uint64_t len, uint64_t elem, uintptr_t *choices, int at_start);
int combinations(uint64_t len, uint64_t elem, uintptr_t *choices);

#endif /* TRIEBCD_H */
