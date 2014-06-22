/*
 * ============================================================================
 *
 *       Filename:  trieBCD.c
 *
 *    Description:  Demultiplex reads by 5' barcodes
 *
 *        Version:  1.0
 *        Created:  11/06/14 12:17:17
 *       Revision:  none
 *        License:  GPLv3+
 *       Compiler:  gcc, clang
 *
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */

#include "trieBCD.h"

struct tbd_barcode *
tbd_barcode_create(const char *seq, size_t len)
{
    struct tbd_barcode *bcd = NULL;

    if (seq == NULL || len < 1) {
        return NULL;
    }
    bcd = km_malloc(sizeof(*bcd));
    bcd->seq = strndup(seq, len+1);
    if (bcd->seq == NULL) {
        km_free(bcd);
        return NULL;
    }
    bcd->len = len;
    bcd->count = 0;
    return bcd;
}

void
tbd_barcode_destroy_(struct tbd_barcode *barcode)
{
    if (!tbd_barcode_ok(barcode)) return;
    km_free(barcode->seq);
    barcode->len = 0;
    km_free(barcode);
}

struct tbd_config *
tbd_config_create(void)
{
    struct tbd_config *config = km_calloc(1, sizeof(*config));
    /* km_calloc never returns null, we use errprintexit as the err handler */
    return config;
}

void
tbd_config_destroy_(struct tbd_config *config)
{
    size_t iii = 0;
    size_t jjj = 0;

    if (config == NULL) {
        return;
    }

    km_free(config->barcode_file);
    for (iii = 0; iii < 2; iii++) {
        km_free(config->infiles[iii]);
    }
    km_free(config->out_prefixes[0]);
    km_free(config->out_prefixes[1]);
    km_free(config->infiles[0]);
    km_free(config->infiles[1]);
    for (iii = 0; iii < config->n_barcodes_1; iii ++) {
        for (jjj = 0; jjj < config->n_barcodes_2; jjj++) {
            tbd_output_destroy(config->outputs[iii][jjj]);
            tbd_barcode_destroy(config->barcodes[iii][jjj]);
        }
    }
    tbd_output_destroy(config->unknown_output);
}


static char *
_tbd_format_outfile_path (const char *prefix, const char *id, int read,
        const char *ext)
{
    char buf[4096];
    int res = 0;

    if (prefix == NULL || id == NULL) {
        return NULL;
    }
    res = snprintf(buf, 4096, "%s_%s_R%d.%s", prefix, id, read, ext);
    if (res >= 4096) {
        return NULL;
    }
    return strndup(buf, 4096);
}

struct tbd_output *
tbd_output_create(const char *fwd_fpath, const char *rev_fpath,
        enum read_mode mode, const char *fp_mode)
{
    struct tbd_output *out = NULL;

    if (mode == READS_UNKNOWN || fwd_fpath == NULL || \
            (mode == READS_PAIRED && rev_fpath == NULL)) {
        return NULL;
    }
    out = km_calloc(1, sizeof(*out));
    out->mode = mode;
    out->fwd_file = seqfile_create(fwd_fpath, fp_mode);
    if (out->fwd_file == NULL) {
        km_free(out);
        return NULL;
    }
    out->rev_file = seqfile_create(rev_fpath, fp_mode);
    if (out->rev_file == NULL) {
        seqfile_destroy(out->fwd_file);
        km_free(out);
        return NULL;
    }
    out->count = 0;
    return out;
}

void
tbd_output_destroy_(struct tbd_output *output)
{
    if (output != NULL) {
        seqfile_destroy(output->fwd_file);
        seqfile_destroy(output->rev_file);
        output->mode = READS_UNKNOWN;
        output->count = 0llu;
    }
}

int
tbd_load_barcodes(struct tbd_config *config)
{
    intptr_t barcode_idx = 0;
    size_t iii = 0;
    int retval = -1;

    if (!tbd_config_ok(config) || !config->have_cli_opts) {
        return -1;
    }
    /* Create and alloc tries */
    config->tries = km_malloc(config->mismatches * sizeof(*config->tries));
    for (iii = 0; iii < config->mismatches; iii++) {
        config->tries[iii] = tbd_trie_create();
        if (config->tries[iii] == NULL) {
            km_free(config->tries);
            return 1;
        }
    }
    seqfile_t *bcd_sf = seqfile_create(config->barcode_file, "r");
    SEQFILE_ITER_SINGLE_BEGIN(bcd_sf, this_bcd, bcd_ln)
        char **mutated = NULL;
        size_t num_mutated = 0;
        size_t jjj = 0;
        int ret = 0;

        /* Make mutated barcodes and add to trie */
        for (iii = 0; iii < config->mismatches; iii++) {
            mutated = hamming_mutate_dna(&num_mutated, this_bcd->seq.s,
                                         this_bcd->seq.l, iii, 0);
            for (jjj = 0; jjj < num_mutated; jjj++) {
                ret = tbd_trie_add(config->tries[iii], mutated[jjj],
                             barcode_idx);
                if (ret != 0) {
                    printf("Big fuckup about here\n"); /* TODO */
                    retval = 1;
                    break;
                }
            }
        }
        /* Open barcode files */
        /* TODO */
        barcode_idx++;
    SEQFILE_ITER_SINGLE_END(this_bcd)
    return retval;
}

int
combinations(uint64_t len, uint64_t elem, uintptr_t *choices)
{
    int at_start = 1;
    ssize_t iii = 0;
    if (len < elem || choices == NULL) {
        /* error value, so don't use (!ret) as your test for the end of the
           enclosing while loop, or on error you'll have an infinite loop */
        return -1;
    }
    /* Check if we're at the start, i.e. all items are 0 */
    for (iii = 0; iii < elem; iii++) {
        at_start &= choices[iii] == 0;
    }
    if (at_start) {
        /* In the first iteration, we set the choices to the first ``elem``
           valid choices, i.e., 0 ... elem - 1 */
        for (iii = 0; iii < elem; iii++) {
            choices[iii] = iii;
        }
        return 1;
    }
    /* Can we increment the final element? */
    if (choices[elem - 1] < len - 1) {
        choices[elem - 1]++;
        return 1;
    } else {
        /* Count backwards until we can increment a choice */
        iii = elem - 1;
        while (iii >= 0) {
            uint64_t this_max = len - (elem - iii);
            if (choices[iii] <  this_max) {
                /* Woo, we've found something to increment. */
                ssize_t jjj;
                /* Increment this choice */
                choices[iii]++;
                /* fill the incrementing forwards. */
                for (jjj = iii + 1; jjj < elem; jjj++) {
                    choices[jjj] = choices[jjj - 1] + 1;
                }
                return 1;
            }
            iii--;
        }
        for (iii = 0; iii < elem; iii++) {
            choices[iii] = 0llu;
        }
        return 0;
    }
}

int
product(uint64_t len, uint64_t elem, uintptr_t *choices, int at_start)
{
    ssize_t iii = 0;
    if (len < elem || choices == NULL) {
        /* error value, so don't use (!ret) as your test for the end of the
           enclosing while loop, or on error you'll have an infinite loop */
        return -1;
    }
    if (at_start) {
        /* [0, 0, ..., 0] is a valid set */
        return 1;
    }
    iii = elem - 1;
    while (iii >= 0) {
        if (choices[iii] <  len - 1) {
            /* Woo, we've found something to increment. */
            ssize_t jjj;
            /* Increment this choice */
            choices[iii]++;
            /* fill forwards with 0. */
            for (jjj = iii + 1; jjj < elem; jjj++) {
                choices[jjj] = 0;
            }
            return 1;
        }
        iii--;
    }
    for (iii = 0; iii < elem; iii++) {
        choices[iii] = 0llu;
    }
    return 0;
}

char **
hamming_mutate_dna(size_t *n_results_o, const char *str, size_t len,
        int dist, int keep_original)
{
    const char alphabet[] = "ACGT";
    const size_t n_letters = 4;
    char *tmp = NULL;
    char **result = NULL;
    size_t results = 0;
    size_t results_alloced = 64;
    size_t iii;
    uint64_t *mut_indicies;
    uint64_t *alphabet_indicies;
    int mut_ret = 1;
    int alpha_ret = 0;

    if (str == NULL || len < 1 || dist < 1) {
        return NULL;
    }

    result = km_malloc(results_alloced * sizeof(*result));
    mut_indicies = km_calloc(dist, sizeof(*mut_indicies));
    alphabet_indicies = km_calloc(dist, sizeof(*alphabet_indicies));

    while ((mut_ret = combinations(len, dist, mut_indicies)) == 1) {
        while ((alpha_ret = product(n_letters, dist, alphabet_indicies, !alpha_ret)) == 1) {
            tmp = strndup(str, len+1);
            for (iii = 0; iii < dist; iii++) {
                char replacement = alphabet[alphabet_indicies[iii]];
                size_t mut_idx = mut_indicies[iii];
                if (tmp[mut_idx] == replacement) {
                    continue;
                }
                tmp[mut_idx] = replacement;
            }

            if (strncmp(str, tmp, len) == 0 && !keep_original) {
                km_free(tmp);
                continue;
            } else {
                if (results + 1 > results_alloced) {
                    results_alloced = kmroundupz(results_alloced);
                    result = km_realloc(result,
                                        results_alloced * sizeof(*result));
                }
                result[results++] = strndup(tmp, len);
                km_free(tmp);
            }
        }
    }
    *n_results_o = results;
    return result;
}

struct tbd_trie *
tbd_trie_create(void)
{
    struct tbd_trie *trie = NULL;
    AlphaMap *map = alpha_map_new();
    int ret = 0;

    /* Make trie AlphaMap */
    if (map == NULL) {
        return NULL;
    }
    #define _AM_ADD(chr)                                                        \
    ret = alpha_map_add_range(map, chr, chr);                               \
    if (ret == 0) {                                                         \
        alpha_map_free(map);                                                \
        return NULL;                                                        \
    }
    _AM_ADD('A')
    _AM_ADD('C')
    _AM_ADD('G')
    _AM_ADD('T')
    #undef _AM_ADD
    trie = km_calloc(1, sizeof(*trie));
    trie->trie = trie_new(map);
    if (trie->trie == NULL) {
        km_free(trie);
        alpha_map_free(map);
        return NULL;
    }
    return trie;
}

int
tbd_trie_add (struct tbd_trie *trie, const char *seq, uintptr_t data)
{
    if (!tbd_trie_ok(trie)) return -1;
    if (trie_store_if_absent(trie->trie, seq, data)) {
        return 0;
    }
    return 1;
}

int
tbd_barcode_match (intptr_t *value, struct tbd_trie *trie, const seq_t *seq)
{
    TrieState *trie_iter = NULL;
    size_t seq_pos = 0;
    size_t max_match_len = 0;
    int res = 0;
    intptr_t our_val = -1;

    /* *value is set to -1 on anything bad happening including failed lookup */
    if (value == NULL || !tbd_trie_ok(trie) || !seq_ok(seq)) {
        *value = -1;
        return -1;
    }
    if (seq->seq.l < trie->min_len) {
        *value = -1;
        return 1;
    }
    /* Only look until the maximum of the largest barcode, or seq len */
    max_match_len = seq->seq.l < trie->max_len ? trie->max_len : seq->seq.l;
    /* Grab tree root iter, and check it. */
    trie_iter = trie_root(trie->trie);
    if (trie_iter == NULL) {
        *value = -1;
        return -1;
    }
    /* Consume seq until we can't */
    for (seq_pos = 0; seq_pos < max_match_len; seq_pos++) {
        res = trie_state_walk(trie_iter, seq->seq.s[seq_pos]);
        if (!res) {
            break;
        }
    }
    if (trie_state_is_terminal(trie_iter)) {
        our_val = trie_state_get_data(trie_iter);
    } else {
        goto end;
    }
end:
    trie_state_free(trie_iter);
    *value = our_val;
    if (our_val != -1) {
        /* we've succeeded */
        return 0;
    }
    return 1;
}
