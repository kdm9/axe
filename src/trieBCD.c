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

struct tbd_config *
tbd_config_create(void)
{
    struct tbd_config *config = km_calloc(1, sizeof(*config));

    /* km_calloc never returns null, we use errprintexit as the err handler */
    config->ok = 0;
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
    for (iii = 0; iii < config->n_outputs_1; iii ++) {
        for (jjj = 0; jjj < config->n_outputs_2; jjj++) {
            tbd_output_destroy(config->outputs[iii][jjj]);
        }
    }
    tbd_output_destroy(config->unknown_output);
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

