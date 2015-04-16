/*
 * ============================================================================
 *
 *       Filename:  axe.c
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

#include "axe.h"
#include "gsl_combination.h"

/* Holds the current timestamp, so we don't have to free the returned string
 * from now(). */
char _time_now[10] = "";

struct axe_barcode *
axe_barcode_create(void)
{
    struct axe_barcode *bcd = NULL;

    bcd = qes_calloc(1, sizeof(*bcd));
    return bcd;
}

void
axe_barcode_destroy_(struct axe_barcode *barcode)
{
    if (!axe_barcode_ok(barcode)) return;
    qes_free(barcode->seq1);
    qes_free(barcode->seq2);
    qes_free(barcode->id);
    barcode->len1 = 0;
    barcode->len2 = 0;
    qes_free(barcode);
}

struct axe_config *
axe_config_create(void)
{
    struct axe_config *config = qes_calloc(1, sizeof(*config));

    /* qes_calloc never returns null, we use errprintexit as the err handler */
    return config;
}

void
axe_config_destroy_(struct axe_config *config)
{
    size_t iii = 0;

    if (config == NULL) {
        return;
    }
    /* File names */
    qes_free(config->barcode_file);
    qes_free(config->table_file);
    qes_free(config->out_prefixes[0]);
    qes_free(config->out_prefixes[1]);
    qes_free(config->infiles[0]);
    qes_free(config->infiles[1]);
    /* outputs */
    if (config->outputs != NULL) {
        for (iii = 0; iii < config->n_barcode_pairs; iii ++) {
            axe_output_destroy(config->outputs[iii]);
        }
    }
    qes_free(config->outputs);
    axe_output_destroy(config->unknown_output);
    /* barcode pairs */
    if (config->barcodes != NULL) {
        for (iii = 0; iii < config->n_barcode_pairs; iii++) {
            axe_barcode_destroy(config->barcodes[iii]);
        }
    }
    qes_free(config->barcodes);
    /* barcode lookup */
    if (config->barcode_lookup != NULL) {
        for (iii = 0; iii < config->n_barcodes_1; iii++) {
            qes_free(config->barcode_lookup[iii]);
        }
    }
    qes_free(config->barcode_lookup);
    /* Tries */
    axe_trie_destroy(config->fwd_trie);
    axe_trie_destroy(config->rev_trie);
    qes_free(config);
}


static char *
_axe_format_outfile_path (const char *prefix, const char *id, int read,
                          const char *ext)
{
    char buf[4096];
    int res = 0;

    if (prefix == NULL || id == NULL) {
        return NULL;
    }
    if (read > 0) {
        res = snprintf(buf, 4096, "%s_%s_R%d.%s", prefix, id, read, ext);
    } else {
        res = snprintf(buf, 4096, "%s_%s_il.%s", prefix, id, ext);
    }
    if (res >= 4096) {
        return NULL;
    }
    return strndup(buf, 4096);
}

struct axe_output *
axe_output_create(const char *fwd_fpath, const char *rev_fpath,
                  enum read_mode mode, const char *fp_mode)
{
    struct axe_output *out = NULL;

    if (mode == READS_UNKNOWN || fwd_fpath == NULL || \
        (mode == READS_PAIRED && rev_fpath == NULL)) {
        return NULL;
    }
    out = qes_calloc(1, sizeof(*out));
    out->mode = mode;
    out->fwd_file = qes_seqfile_create(fwd_fpath, fp_mode);
    if (out->fwd_file == NULL) {
        qes_free(out);
        return NULL;
    }
    qes_seqfile_set_format(out->fwd_file, FASTQ_FMT);
    if (rev_fpath != NULL) {
        out->rev_file = qes_seqfile_create(rev_fpath, fp_mode);
        if (out->rev_file == NULL) {
            qes_seqfile_destroy(out->fwd_file);
            qes_free(out);
            return NULL;
        }
        qes_seqfile_set_format(out->rev_file, FASTQ_FMT);
    } else {
        out->rev_file = NULL;
    }
    return out;
}

void
axe_output_destroy_(struct axe_output *output)
{
    if (output != NULL) {
        qes_seqfile_destroy(output->fwd_file);
        qes_seqfile_destroy(output->rev_file);
        output->mode = READS_UNKNOWN;
        qes_free(output);
    }
}

static inline struct axe_barcode *
read_barcode_combo(char *line)
{
    char seq1[100] = "";
    char seq2[100] = "";
    char id[100] = "";
    int res = 0;
    struct axe_barcode *barcode = NULL;

    if (line == NULL) {
        return NULL;
    }
    res = sscanf(line, "%99s\t%99s\t%99s", seq1, seq2, id);
    if (res < 3) {
        fprintf(stderr, "Error, sscanf returned %i\n", res);
        return NULL;
    }
    barcode = axe_barcode_create();
    if (barcode == NULL) {
        return NULL;
    }
    /* Duplicate on the heap the R1 seq */
    barcode->seq1 = strndup(seq1, 100);
    if (barcode->seq1 == NULL) goto error;
    barcode->len1 = strnlen(seq1, 100);
    /* Second barcode too */
    barcode->seq2 = strndup(seq2, 100);
    if (barcode->seq2 == NULL) goto error;
    barcode->len2 = strnlen(seq2, 100);
    /* And the ID */
    barcode->id = strndup(id, 100);
    if (barcode->id == NULL) goto error;
    barcode->idlen = strnlen(id, 100);
    return barcode;

error:
    axe_barcode_destroy(barcode);
    return NULL;
}

static inline struct axe_barcode *
read_barcode_single(char *line)
{
    char seq[100] = "";
    char id[100] = "";
    int res = 0;
    struct axe_barcode * barcode = NULL;

    if (line == NULL) {
        return NULL;
    }
    res = sscanf(line, "%99s\t%99s", seq, id);
    if (res < 2) {
        return NULL;
    }
    barcode = axe_barcode_create();
    if (barcode == NULL) {
        return NULL;
    }
    /* Duplicate on the heap the R1 seq */
    barcode->seq1 = strndup(seq, 100);
    if (barcode->seq1 == NULL) goto error;
    barcode->len1 = strnlen(seq, 100);
    /* And the ID */
    barcode->id = strndup(id, 100);
    if (barcode->id == NULL) goto error;
    barcode->idlen = strnlen(id, 100);
    return barcode;

error:
    axe_barcode_destroy(barcode);
    return NULL;
}

int
axe_read_barcodes(struct axe_config *config)
{
    struct qes_file *qf = NULL;
    struct axe_barcode *this_barcode = NULL;
    struct axe_barcode **barcodes = NULL;
    size_t n_barcode_pairs = 0; /* Entries in file */
    size_t n_barcodes_alloced = 8;
    const char *bad_fname_chars = "'\"!@#$%^&*()+=~`[]{}\\|;:/?><,";
    char *line = NULL;
    char *tmp = NULL;
    size_t linesz = 128;
    ssize_t linelen = 0;
    size_t iii = 0;

    if (!axe_config_ok(config)) {
        return -1;
    }
    barcodes = qes_calloc(n_barcodes_alloced, sizeof(*barcodes));
    qf = qes_file_open(config->barcode_file, "r");
    line = qes_malloc(linesz);

    while ((linelen = qes_file_readline_realloc(qf, &line, &linesz)) > 0) {
        /* Skip an optional header line */
        if (strncmp(line, "Barcode", 7) == 0 || \
            strncmp(line, "barcode", 7) == 0) {
            continue;
        }
        /* Skip #-comment or ;-comment */
        if (line[0] == '#' || line[0] == ';') {
            continue;
        }
        /* Reallocate the array if we need to */
        if (n_barcode_pairs == n_barcodes_alloced) {
            n_barcodes_alloced *= 2;
            barcodes = qes_realloc(barcodes,
                                   n_barcodes_alloced * sizeof(*barcodes));
        }
        /* Read the barcode line into a ``struct axe_barcode`` */
        if (config->match_combo) {
            this_barcode = read_barcode_combo(line);
        } else {
            this_barcode = read_barcode_single(line);
        }
        if (this_barcode == NULL) {
            fprintf(stderr, "Couldn't parse barcode line '%s'\n", line);
            goto error;
        }
        /* Replace all bad chars with '-' */
        tmp = strpbrk(this_barcode->id, bad_fname_chars);
        while (tmp != NULL) {
            *tmp = '-'; /* Replace with dash */
            tmp = strpbrk(tmp + 1, bad_fname_chars);
        }
        /* Add the barcode to the array */
        barcodes[n_barcode_pairs++] = this_barcode;
    }

    /* Save the array to the config struct */
    config->barcodes = barcodes;
    config->n_barcode_pairs = n_barcode_pairs;
    qes_file_close(qf);
    qes_free(line);
    if (config->verbosity > 0) {
        fprintf(stderr, "[read_barcodes] (%s) Read in barcodes\n",
                nowstr());
    }
    return 0;

error:
    if (barcodes != NULL) {
        for (iii = 0; iii < n_barcode_pairs; iii++) {
            axe_barcode_destroy(barcodes[iii]);
        }
    }
    qes_file_close(qf);
    qes_free(line);
    return 1;
}

static int
setup_barcode_lookup_single(struct axe_config *config)
{
    size_t iii = 0;

    if (!axe_config_ok(config)) {
        return -1;
    }
    config->n_barcodes_1 = config->n_barcode_pairs;
    config->n_barcodes_2 = 0;
    config->barcode_lookup = qes_malloc(config->n_barcodes_1 *
                                        sizeof(*config->barcode_lookup));
    for (iii = 0; iii < config->n_barcode_pairs; iii++) {
        config->barcode_lookup[iii] = qes_malloc(
                                          sizeof(**config->barcode_lookup));
        config->barcode_lookup[iii][0] = iii;
    }
    return 0;
}

static int
setup_barcode_lookup_combo(struct axe_config *config)
{
    struct axe_barcode *this_barcode = NULL;
    size_t n_barcodes_1 = 0; /* R1 barcodes */
    size_t n_barcodes_2 = 0;
    size_t iii = 0;
    intptr_t tmp = 0;
    int ret = -1;
    int res = 0;
    size_t bcd1 = 0;
    size_t bcd2 = 0;
    struct axe_trie *seq1_trie = NULL;
    struct axe_trie *seq2_trie = NULL;

    if (!axe_config_ok(config)) {
        return -1;
    }
    /* Make "hash table" of barcode => unique bcd num. We use tries, as they
       work fine as a associatve map, and we've already got the headers, lib
       etc in the system. */
    seq1_trie = axe_trie_create();
    seq2_trie = axe_trie_create();
    assert(seq1_trie != NULL && seq2_trie != NULL);
    for (iii = 0; iii < config->n_barcode_pairs; iii++) {
        this_barcode = config->barcodes[iii];
        if (!axe_barcode_ok(this_barcode)) {
            fprintf(stderr, "[setup_lookup] Bad barcode at %" PRIu64 "\n", iii);
            goto error;
        }
        if (!axe_trie_get(seq1_trie, this_barcode->seq1, &tmp)) {
            axe_trie_add(seq1_trie, this_barcode->seq1, n_barcodes_1++);
        }
        if (!axe_trie_get(seq2_trie, this_barcode->seq2, &tmp)) {
            axe_trie_add(seq2_trie, this_barcode->seq2, n_barcodes_2++);
        }
    }
    config->n_barcodes_1 = n_barcodes_1;
    config->n_barcodes_2 = n_barcodes_2;
    /* Make barcode lookup */
    config->barcode_lookup = qes_malloc(n_barcodes_1 *
                                        sizeof(*config->barcode_lookup));
    for (bcd1 = 0; bcd1 < config->n_barcodes_1; bcd1++) {
        config->barcode_lookup[bcd1] = qes_calloc(n_barcodes_2,
                                       sizeof(**config->barcode_lookup));
        memset(config->barcode_lookup[bcd1], -1,
               n_barcodes_2 * sizeof(**config->barcode_lookup));
    }
    /* Setup barcode lookup */
    for (iii = 0; iii < config->n_barcode_pairs; iii++) {
        this_barcode = config->barcodes[iii];
        /* already checked barcode above */
        res = axe_trie_get(seq1_trie, this_barcode->seq1,
                            (intptr_t *)(&bcd1));
        if (!res) goto error;
        res = axe_trie_get(seq2_trie, this_barcode->seq2,
                            (intptr_t *)(&bcd2));
        if (!res) goto error;
        config->barcode_lookup[bcd1][bcd2] = iii;
    }
    ret = 0;

exit:
    axe_trie_destroy(seq1_trie);
    axe_trie_destroy(seq2_trie);
    return ret;

error:
    ret = 1;
    goto exit;
}

int
axe_setup_barcode_lookup(struct axe_config *config)
{
    if (!axe_config_ok(config)) {
        return -1;
    }
    if (config->match_combo) {
        return setup_barcode_lookup_combo(config);
    }
    return setup_barcode_lookup_single(config);
}

int
axe_make_tries(struct axe_config *config)
{
    if (!axe_config_ok(config)) {
        return -1;
    }
    config->fwd_trie = axe_trie_create();
    if (config->fwd_trie == NULL) {
        goto error;
    }
    if (config->match_combo) {
        config->rev_trie = axe_trie_create();
        if (config->rev_trie == NULL) {
            goto error;
        }
    }
    return 0;

error:
    fprintf(stderr, "[make_tries] ERROR: axe_trie_create returned NULL\n");
    axe_trie_destroy(config->fwd_trie);
    if (config->match_combo) {
        axe_trie_destroy(config->rev_trie);
    }
    return 1;
}

static char *
axe_make_file_ext(const struct axe_config *config)
{
    if (!axe_config_ok(config)) {
        return NULL;
    }
    if (config->out_compress_level > 0 &&
        config->out_compress_level < 10) {
        return strdup("fastq.gz");
    }
    return strdup("fastq");
}

static char *
axe_make_zmode(const struct axe_config *config)
{
    if (!axe_config_ok(config)) {
        return NULL;
    }
    if (config->out_compress_level > 0 &&
        config->out_compress_level < 10) {
        char tmp[3] = "";
        snprintf(tmp, 3, "w%d", config->out_compress_level);
        return strdup(tmp);
    }
    return strdup("wT");
}

static inline int
load_tries_combo(struct axe_config *config)
{
    int bcd1 = -1;
    int bcd2 = -1;
    int retval = 0;
    char **mutated = NULL;
    size_t num_mutated = 0;
    int ret = 0;
    size_t iii = 0;
    size_t jjj = 0;
    size_t mmm = 0;
    struct axe_barcode *this_bcd = NULL;
    intptr_t tmp = 0;

    if (!axe_config_ok(config)) {
        fprintf(stderr, "[load_tries] Bad config\n");
        ret = -1;
        goto exit;
    }
    /* Make mutated barcodes and add to trie */
    for (iii = 0; iii < config->n_barcode_pairs; iii++) {
        this_bcd = config->barcodes[iii];
        if (!axe_barcode_ok(this_bcd)) {
            fprintf(stderr, "[load_tries] Bad R1 barcode at %" PRIu64 "\n", iii);
            ret = -1;
            goto exit;
        }
        /* Either lookup the index of the first read in the barcode table, or
         * insert this barcode into the table, storing its index.
         * Note the NOT here. */
        if (!axe_trie_get(config->fwd_trie, this_bcd->seq1, &tmp)) {
            ret = axe_trie_add(config->fwd_trie, this_bcd->seq1, ++bcd1);
            if (ret != 0) {
                fprintf(stderr, "ERROR: Could not load barcode %s into trie %" PRIu64 "\n",
                        this_bcd->seq1, iii);
                ret = 1;
                goto exit;
            }
        } else {
            continue;
        }
        for (jjj = 1; jjj <= config->mismatches; jjj++) {
            /* Do the forwards read barcode */
            mutated = hamming_mutate_dna(&num_mutated, this_bcd->seq1,
                                         this_bcd->len1, jjj, 0);
            if (mutated == NULL) {
                ret = 1;
                goto exit;
            }
            for (mmm = 0; mmm < num_mutated; mmm++) {
                ret = axe_trie_add(config->fwd_trie, mutated[mmm], bcd1);
                if (ret != 0) {
                    if (config->permissive) {
                        if (config->verbosity >= 0) {
                            fprintf(stderr,
                                    "[%s] warning: Will only match to %dmm\n",
                                    __func__, (int)jjj - 1);
                        }
                        axe_trie_delete(config->fwd_trie, mutated[mmm]);
                        qes_free(mutated[mmm]);
                        continue;
                    }
                    fprintf(stderr,
                            "[%s] ERROR: Barcode %s already in fwd trie (%dmm) %s\n",
                            __func__, mutated[mmm], (int)jjj, this_bcd->seq1);
                    retval = 1;
                    goto exit;
                }
                qes_free(mutated[mmm]);
            }
            qes_free(mutated);
        }
    }
    /* Ditto for the reverse read */
    for (iii = 0; iii < config->n_barcode_pairs; iii++) {
        this_bcd = config->barcodes[iii];
        /* Likewise for the reverse read index */
        if (!axe_trie_get(config->rev_trie, this_bcd->seq2, &tmp)) {
            ret = axe_trie_add(config->rev_trie, this_bcd->seq2, ++bcd2);
            if (ret != 0) {
                fprintf(stderr, "ERROR: Could not load barcode %s into trie %" PRIu64 "\n",
                        this_bcd->seq2, iii);
                retval = 1;
                goto exit;
            }
        } else {
            continue;
        }
        for (jjj = 1; jjj <= config->mismatches; jjj++) {
            num_mutated = 0;
            mutated = hamming_mutate_dna(&num_mutated, this_bcd->seq2,
                                         this_bcd->len2, jjj, 0);
            if (mutated == NULL) {
                ret = 1;
                goto exit;
            }
            for (mmm = 0; mmm < num_mutated; mmm++) {
                ret = axe_trie_add(config->rev_trie, mutated[mmm], bcd2);
                if (ret != 0) {
                    if (config->permissive) {
                        if (config->verbosity >= 0) {
                            fprintf(stderr,
                                    "[%s] warning: Will only match %s to %dmm\n",
                                    __func__, this_bcd->id, (int)jjj - 1);
                        }
                        trie_delete(config->rev_trie->trie,
                                    mutated[mmm]);
                        qes_free(mutated[mmm]);
                        continue;
                    }
                    fprintf(stderr,
                            "[%s] ERROR: Barcode %s already in rev trie (%dmm)\n",
                            __func__, mutated[mmm], (int)jjj);
                    retval = 1;
                    goto exit;
                }
                qes_free(mutated[mmm]);
            }
            qes_free(mutated);
        }
    }
    /* we got here, so we succeeded. set retval accordingly */
    retval = 0;

exit:
    if (mutated != NULL) {
        for (mmm = 0; mmm < num_mutated; mmm++) {
            qes_free(mutated[mmm]);
        }
        qes_free(mutated);
    }
    return retval;
}

static inline int
load_tries_single(struct axe_config *config)
{
    char **mutated = NULL;
    size_t num_mutated = 0;
    int ret = 0;
    size_t iii = 0;
    size_t jjj = 0;
    size_t mmm = 0;
    intptr_t tmp = 0;
    int retval = -1;
    struct axe_barcode *this_bcd = NULL;

    if (!axe_config_ok(config)) {
        fprintf(stderr, "[load_tries] Bad config\n");
        return -1;
    }
    /* Make mutated barcodes and add to trie */
    for (iii = 0; iii < config->n_barcode_pairs; iii++) {
        this_bcd = config->barcodes[iii];
        if (!axe_barcode_ok(this_bcd)) {
            fprintf(stderr, "[load_tries] Bad barcode at %" PRIu64 "\n", iii);
            return -1;
        }
        /* Either lookup the index of the first read in the barcode table, or
         * insert this barcode into the table, storing its index.
         * Note the NOT here. */
        if (!axe_trie_get(config->fwd_trie, this_bcd->seq1, &tmp)) {
            ret = axe_trie_add(config->fwd_trie, this_bcd->seq1, (int)iii);
            if (ret != 0) {
                fprintf(stderr,
                        "ERROR: Could not load barcode %s into trie %" PRIu64 "\n",
                        this_bcd->seq1, iii);
                return 1;
            }
        } else {
            fprintf(stderr, "ERROR: Duplicate barcode %s\n", this_bcd->seq1);
            return 1;
        }
        for (jjj = 1; jjj <= config->mismatches; jjj++) {
            mutated = hamming_mutate_dna(&num_mutated, this_bcd->seq1,
                                         this_bcd->len1, jjj, 0);
            if (mutated == NULL) {
                ret = 1;
                goto exit;
            }
            for (mmm = 0; mmm < num_mutated; mmm++) {
                ret = axe_trie_add(config->fwd_trie, mutated[mmm], iii);
                if (ret != 0) {
                    if (config->permissive) {
                        if (config->verbosity >= 0) {
                            fprintf(stderr,
                                    "[%s] warning: Will only match to %dmm\n",
                                    __func__, (int)jjj - 1);
                        }
                        trie_delete(config->fwd_trie->trie,
                                    mutated[mmm]);
                        qes_free(mutated[mmm]);
                        continue;
                    }
                    fprintf(stderr,
                            "[%s] ERROR: Barcode %s already in trie (%dmm)\n",
                            __func__, mutated[mmm], (int)jjj);
                    retval = 1;
                    goto exit;
                }
                qes_free(mutated[mmm]);
            }
            qes_free(mutated);
            num_mutated = 0;
        }
    }
    /* we got here, so we succeeded */
    retval = 0;

exit:
    if (mutated != NULL) {
        for (mmm = 0; mmm < num_mutated; mmm++) {
            qes_free(mutated[mmm]);
        }
        qes_free(mutated);
    }
    return retval;
}

int
axe_make_outputs(struct axe_config *config)
{
    size_t iii = 0;
    char *name_fwd = NULL;
    char *name_rev = NULL;
    char *file_ext = NULL;
    char *zmode = NULL;
    struct axe_barcode *this_bcd = NULL;

    if (!axe_config_ok(config)) {
        fprintf(stderr, "[make_outputs] Bad config\n");
        return -1;
    }
    file_ext = axe_make_file_ext(config);
    zmode = axe_make_zmode(config);
    config->outputs = qes_calloc(config->n_barcode_pairs,
                                 sizeof(*config->outputs));
    /* For each sample, make the filename, make an output */
    for (iii = 0; iii < config->n_barcode_pairs; iii++) {
        this_bcd = config->barcodes[iii];
        /* Open barcode files */
        switch (config->out_mode) {
        case READS_SINGLE:
            name_fwd = _axe_format_outfile_path(config->out_prefixes[0],
                                                this_bcd->id, 1, file_ext);
            name_rev = NULL;
            break;
        case READS_PAIRED:
            name_fwd = _axe_format_outfile_path(config->out_prefixes[0],
                                                this_bcd->id, 1, file_ext);
            name_rev = _axe_format_outfile_path(config->out_prefixes[1],
                                                this_bcd->id, 2, file_ext);
            break;
        case READS_INTERLEAVED:
            name_fwd =  _axe_format_outfile_path(config->out_prefixes[0],
                                                 this_bcd->id, 0, file_ext);
            name_rev = NULL;
            break;
        case READS_UNKNOWN:
        default:
            fprintf(stderr, "[make_outputs] Error: bad output mode %ui\n",
                    config->out_mode);
            goto error;
        }
        config->outputs[iii] = axe_output_create(name_fwd, name_rev,
                               config->out_mode, zmode);

        if (config->outputs[iii] == NULL) {
            fprintf(stderr, "[make_outputs] couldn't create file at %s\n",
                    name_fwd);
            goto error;
        }
        qes_free(name_fwd);
        qes_free(name_rev);
    }
    /* Generate the unknown file in the same manner, using id == unknown */
    switch (config->out_mode) {
    case READS_SINGLE:
        name_fwd = _axe_format_outfile_path(config->out_prefixes[0],
                                            "unknown", 1, file_ext);
        name_rev = NULL;
        break;
    case READS_PAIRED:
        name_fwd = _axe_format_outfile_path(config->out_prefixes[0],
                                            "unknown", 1, file_ext);
        name_rev = _axe_format_outfile_path(config->out_prefixes[1],
                                            "unknown", 2, file_ext);
        break;
    case READS_INTERLEAVED:
        name_fwd =  _axe_format_outfile_path(config->out_prefixes[0],
                                             "unknown", 0, file_ext);
        name_rev = NULL;
        break;
    case READS_UNKNOWN:
    default:
        fprintf(stderr, "[make_outputs] Error: bad output mode %ui\n",
                config->out_mode);
        goto error;
    }
    config->unknown_output = axe_output_create(name_fwd, name_rev,
                             config->out_mode, zmode);
    if (config->unknown_output == NULL) {
        fprintf(stderr, "[make_outputs] couldn't create file at %s\n",
                name_fwd);
        goto error;
    }
    qes_free(file_ext);
    qes_free(zmode);
    return 0;

error:
    qes_free(name_fwd);
    qes_free(name_rev);
    qes_free(file_ext);
    qes_free(zmode);
    return 1;
}

int
axe_load_tries(struct axe_config *config)
{
    int ret = 1;
    if (!axe_config_ok(config)) {
        return -1;
    }
    if (config->match_combo) {
        ret = load_tries_combo(config);
    } else {
        ret = load_tries_single(config);
    }
    if (config->verbosity > 0) {
        fprintf(stderr, "[load_tries] (%s) Barcode tries loaded\n",
                nowstr());
    }
    return ret;
}

static inline int
write_barcoded_read_combo(struct axe_output *out, struct qes_seq *seq1,
                          struct qes_seq *seq2, size_t bcd1_len,
                          size_t bcd2_len)
{
    int ret = 0;

    if (seq1->seq.len <= bcd1_len) {
        /* Truncate seqs to N */
        seq1->seq.str[0] = 'N';
        seq1->seq.str[1] = '\0';
        seq1->seq.len = 1;
        /* Keep first qual 'base' */
        seq1->qual.str[1] = '\0';
        seq1->qual.len = 1;
    }
    if (seq2->seq.len <= bcd2_len) {
        /* Truncate seqs to N */
        seq2->seq.str[0] = 'N';
        seq2->seq.str[1] = '\0';
        seq2->seq.len = 1;
        /* Keep first qual 'base' */
        seq2->qual.str[1] = '\0';
        seq2->qual.len = 1;
    }
    /* Bit of the ol' switcheroo. We keep the seq's char pointers, so we
       need to switch them back to their orig. values, but don't want to
       copy. Kludgy, I know. */
    seq1->seq.str += bcd1_len;
    seq1->seq.len -= bcd1_len;
    seq1->qual.str += bcd1_len;
    seq1->qual.len -= bcd1_len;
    ret = qes_seqfile_write(out->fwd_file, seq1);
    if (ret < 1) {
        fprintf(stderr,
                "[process_file] Error: writing to fwd file %s failed\n%s\n",
                out->fwd_file->qf->path,
                qes_file_error(out->fwd_file->qf));
        seq1->seq.str -= bcd1_len;
        seq1->seq.len += bcd1_len;
        seq1->qual.str -= bcd1_len;
        seq1->qual.len += bcd1_len;
        return 1;
    }
    seq1->seq.str -= bcd1_len;
    seq1->seq.len += bcd1_len;
    seq1->qual.str -= bcd1_len;
    seq1->qual.len += bcd1_len;
    seq2->seq.str += bcd2_len;
    seq2->seq.len -= bcd2_len;
    seq2->qual.str += bcd2_len;
    seq2->qual.len -= bcd2_len;
    if (out->mode == READS_INTERLEAVED) {
        ret = qes_seqfile_write(out->fwd_file, seq2);
        if (ret < 1) {
            fprintf(stderr,
                    "[process_file] Error: writing to il file %s failed\n%s\n",
                    out->fwd_file->qf->path,
                    qes_file_error(out->fwd_file->qf));
            return 1;
        }
    } else if (out->mode == READS_PAIRED) {
        ret = qes_seqfile_write(out->rev_file, seq2);
        if (ret < 1) {
            fprintf(stderr,
                    "[process_file] Error: writing to rev file %s failed\n%s\n",
                    out->rev_file->qf->path,
                    qes_file_error(out->rev_file->qf));
            return 1;
        }
    }
    seq2->seq.str -= bcd2_len;
    seq2->seq.len += bcd2_len;
    seq2->qual.str -= bcd2_len;
    seq2->qual.len += bcd2_len;
    return 0;
}

static inline void
increment_reads_print_progress(struct axe_config *config)
{
    config->reads_processed++;
    if (config->reads_processed % 100000 == 0) {
        if (config->verbosity >= 0) {
            fprintf(stderr, "%s: Processed %.1fM %s\r",
                    nowstr(), (float)(config->reads_processed/1000000.0),
                    config->out_mode == READS_SINGLE ? "reads" : "read pairs");
        }
    }
}

static inline int
process_read_pair_single(struct axe_config *config, struct qes_seq *seq1,
                        struct qes_seq *seq2)
{
    int ret = 0;
    ssize_t bcd = -1;
    size_t barcode_pair_index = 0;
    struct axe_output *outfile = NULL;
    size_t bcd_len = 0;

    ret = axe_match_read(&bcd, config->fwd_trie, seq1);
    increment_reads_print_progress(config);
    if (ret != 0) {
        /* No match */
        qes_seqfile_write(config->unknown_output->fwd_file, seq1);
        if (seq2 != NULL) {
            if (config->out_mode == READS_INTERLEAVED) {
                qes_seqfile_write(config->unknown_output->fwd_file, seq2);
            } else {
                qes_seqfile_write(config->unknown_output->rev_file, seq2);
            }
        }
        config->reads_failed++;
        return 0;
    }
    /* Found a match */
    config->reads_demultiplexed++;
    /* FIXME: we need to check bcd doesn't cause segfault */
    barcode_pair_index = config->barcode_lookup[bcd][0];
    outfile = config->outputs[barcode_pair_index];
    bcd_len = config->barcodes[barcode_pair_index]->len1;
    config->barcodes[bcd]->count++;
    if (seq1->seq.len <= bcd_len) {
        /* Don't write out seqs shorter than the barcode */
        return 0;
    }
    /* Bit of the ol' switcheroo. We keep the seq's char pointers, so we need
     * to switch them back to their orig. values, but don't want to copy.
     * Kludgy, I know. */
    seq1->seq.str += bcd_len;
    seq1->seq.len -= bcd_len;
    seq1->qual.str += bcd_len;
    seq1->qual.len -= bcd_len;
    ret = qes_seqfile_write(outfile->fwd_file, seq1);
    if (ret < 1) {
        fprintf(stderr,
                "[write_read_single] Error: writing to R1 file %s failed\n%s\n",
                outfile->fwd_file->qf->path,
                qes_file_error(outfile->fwd_file->qf));
        seq1->seq.str -= bcd_len;
        seq1->seq.len += bcd_len;
        seq1->qual.str -= bcd_len;
        seq1->qual.len += bcd_len;
        return 1;
    }
    seq1->seq.str -= bcd_len;
    seq1->seq.len += bcd_len;
    seq1->qual.str -= bcd_len;
    seq1->qual.len += bcd_len;
    /* And do the same with seq2, if we have one */
    if (seq2 != NULL) {
        if (config->trim_rev) {
            seq2->seq.str += bcd_len;
            seq2->seq.len -= bcd_len;
            seq2->qual.str += bcd_len;
            seq2->qual.len -= bcd_len;
        }
        if (outfile->mode == READS_INTERLEAVED) {
            ret = qes_seqfile_write(outfile->fwd_file, seq2);
            if (ret < 1) {
                fprintf(stderr,
                        "[process_file] Error: writing to il file %s failed\n%s\n",
                        outfile->fwd_file->qf->path,
                        qes_file_error(outfile->fwd_file->qf));
                return 1;
            }
        } else if (outfile->mode == READS_PAIRED) {
            ret = qes_seqfile_write(outfile->rev_file, seq2);
            if (ret < 1) {
                fprintf(stderr,
                        "[process_file] Error: writing to rev file %s failed\n%s\n",
                        outfile->rev_file->qf->path,
                        qes_file_error(outfile->rev_file->qf));
                return 1;
            }
        }
        if (config->trim_rev) {
            seq2->seq.str -= bcd_len;
            seq2->seq.len += bcd_len;
            seq2->qual.str -= bcd_len;
            seq2->qual.len += bcd_len;
        }
    }
    return 0;
}


static int
process_file_single(struct axe_config *config)
{
    struct qes_seqfile *fwdsf = NULL;
    struct qes_seqfile *revsf = NULL;
    int ret = 0;
    int retval = -1;

    if (!axe_config_ok(config)) {
        fprintf(stderr, "[process_file] Bad config struct\n");
        return -1;
    }
    fwdsf = qes_seqfile_create(config->infiles[0], "r");
    if (fwdsf == NULL) {
        fprintf(stderr, "[process_file] Couldn't open seqfile %s\n",
                config->infiles[0]);
        goto exit;
    }
    switch(config->in_mode) {
    case READS_SINGLE:
        goto single;
        break;
    case READS_INTERLEAVED:
        goto interleaved;
        break;
    case READS_PAIRED:
        revsf = qes_seqfile_create(config->infiles[1], "r");
        if (revsf == NULL) {
            fprintf(stderr, "[process_file] Couldn't open seqfile %s\n",
                    config->infiles[1]);
            goto exit;
        }
        goto paired;
        break;
    case READS_UNKNOWN:
    default:
        fprintf(stderr, "[process_file_single] Bad infile mode %ui\n",
                config->in_mode);
        goto exit;
        break;
    }

single:
    QES_SEQFILE_ITER_SINGLE_BEGIN(fwdsf, seq, seqlen) {
        ret = process_read_pair_single(config, seq, NULL);
    }
    QES_SEQFILE_ITER_SINGLE_END(seq);
    retval = ret == 0 ? 0 : 1;
    goto exit;

interleaved:
    QES_SEQFILE_ITER_INTERLEAVED_BEGIN(fwdsf, seq1, seq2, seqlen1, seqlen2) {
        ret = process_read_pair_single(config, seq1, seq2);
    }
    QES_SEQFILE_ITER_INTERLEAVED_END(seq1, seq2);
    retval = ret == 0 ? 0 : 1;
    goto exit;

paired:
    QES_SEQFILE_ITER_PAIRED_BEGIN(fwdsf, revsf, seq1, seq2, seqlen1, seqlen2) {
        ret = process_read_pair_single(config, seq1, seq2);
    }
    QES_SEQFILE_ITER_PAIRED_END(seq1, seq2);
    retval = ret == 0 ? 0 : 1;
    goto exit;
exit:
    qes_seqfile_destroy(fwdsf);
    qes_seqfile_destroy(revsf);
    return retval;
}


static int
process_read_pair_combo(struct axe_config *config, struct qes_seq *seq1,
                        struct qes_seq *seq2)
{
    ssize_t barcode_pair_index = 0;
    intptr_t bcd1 = -1;
    intptr_t bcd2 = -1;
    int r1_ret = 0;
    int r2_ret = 0;
    size_t bcd1_len = 0;
    size_t bcd2_len = 0;
    struct axe_output *outfile = NULL;

    r1_ret = axe_match_read(&bcd1, config->fwd_trie, seq1);
    r2_ret = axe_match_read(&bcd2, config->rev_trie, seq2);
    increment_reads_print_progress(config);
    if (r1_ret != 0 || r2_ret != 0) {
        /* No match */
        qes_seqfile_write(config->unknown_output->fwd_file, seq1);
        if (config->out_mode == READS_INTERLEAVED) {
            qes_seqfile_write(config->unknown_output->fwd_file, seq2);
        } else {
            qes_seqfile_write(config->unknown_output->rev_file, seq2);
        }
        config->reads_failed++;
        return 0;
    }
    /* Found a match */
    barcode_pair_index = config->barcode_lookup[bcd1][bcd2];
    if (barcode_pair_index < 0) {
        /* Invalid match */
        qes_seqfile_write(config->unknown_output->fwd_file, seq1);
        if (config->out_mode == READS_INTERLEAVED) {
            qes_seqfile_write(config->unknown_output->fwd_file, seq2);
        } else {
            qes_seqfile_write(config->unknown_output->rev_file, seq2);
        }
        config->reads_failed++;
        return 0;
    }
    config->reads_demultiplexed++;
    outfile = config->outputs[barcode_pair_index];
    bcd1_len = config->barcodes[barcode_pair_index]->len1;
    bcd2_len = config->barcodes[barcode_pair_index]->len2;
    config->barcodes[barcode_pair_index]->count++;
    return write_barcoded_read_combo(outfile, seq1, seq2, bcd1_len,
                                     bcd2_len);
}


static int
process_file_combo(struct axe_config *config)
{
    struct qes_seqfile *fwdsf = NULL;
    struct qes_seqfile *revsf = NULL;
    int have_error = 0;

    if (!axe_config_ok(config)) {
        fprintf(stderr, "[process_file] Bad config struct\n");
        return -1;
    }
    fwdsf = qes_seqfile_create(config->infiles[0], "r");
    if (fwdsf == NULL) {
        fprintf(stderr, "[process_file] Couldn't open seqfile %s\n",
                config->infiles[0]);
        goto error;
    }
    switch(config->in_mode) {
    case READS_INTERLEAVED:
        goto interleaved;
        break;
    case READS_PAIRED:
        revsf = qes_seqfile_create(config->infiles[1], "r");
        if (revsf == NULL) {
            fprintf(stderr, "[process_file] Couldn't open seqfile %s\n",
                    config->infiles[1]);
            goto error;
        }
        goto paired;
        break;
    case READS_SINGLE:
    case READS_UNKNOWN:
    default:
        fprintf(stderr, "[process_file_combo] Bad infile mode %ui\n",
                config->in_mode);
        goto error;
        break;
    }

interleaved:
    QES_SEQFILE_ITER_INTERLEAVED_BEGIN(fwdsf, seq1, seq2, seqlen1, seqlen2)
    if (process_read_pair_combo(config, seq1, seq2)) {
        have_error = 1;
        break;
    }
    QES_SEQFILE_ITER_INTERLEAVED_END(seq1, seq2)
    if (!have_error) goto clean_exit;
    else goto error;

paired:
    QES_SEQFILE_ITER_PAIRED_BEGIN(fwdsf, revsf, seq1, seq2, seqlen1, seqlen2)
    if (process_read_pair_combo(config, seq1, seq2)) {
        have_error = 1;
        break;
    }
    QES_SEQFILE_ITER_PAIRED_END(seq1, seq2)
    if (!have_error) goto clean_exit;
    else goto error;

clean_exit:
    qes_seqfile_destroy(fwdsf);
    qes_seqfile_destroy(revsf);
    return 0;
error:
    qes_seqfile_destroy(fwdsf);
    qes_seqfile_destroy(revsf);
    return 1;
}


int
axe_process_file(struct axe_config *config)
{
    int ret = 0;
    clock_t start = 0;

    if (!axe_config_ok(config)) {
        return -1;
    }
    start = clock();
    if (config->verbosity >= 0) {
        fprintf(stderr, "[process_file] (%s) Starting demultiplexing\n",
                nowstr());
    }
    if (config->match_combo) {
        ret = process_file_combo(config);
    } else {
        ret = process_file_single(config);
    }
    config->time_taken = (float)(clock() - start) / CLOCKS_PER_SEC;
    if (config->verbosity >= 0) {
        fprintf(stderr, "\r[process_file] (%s) Finished demultiplexing\n",
                nowstr());
    }
    return ret;
}

int
product(int64_t len, int64_t elem, uintptr_t *choices, int at_start)
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
        if (choices[iii] <  (uintptr_t)(len - 1)) {
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
                   unsigned int dist, int keep_original)
{
    const char alphabet[] = "ACGT";
    const size_t n_letters = 4;
    char *tmp = NULL;
    char **result = NULL;
    size_t results = 0;
    size_t results_alloced = 64;
    size_t iii;
    uint64_t *alphabet_indicies;
    int alpha_ret = 0;
    gsl_combination *mut_idx_comb;

    if (str == NULL || len < 1 || dist < 1) {
        return NULL;
    }
    result = qes_malloc(results_alloced * sizeof(*result));
    alphabet_indicies = qes_calloc(dist, sizeof(*alphabet_indicies));
    mut_idx_comb = gsl_combination_calloc(len, dist);
    do {
        while ((alpha_ret = product(n_letters, dist, alphabet_indicies,
                                    !alpha_ret)) == 1) {
            tmp = strndup(str, len+1);
            for (iii = 0; iii < dist; iii++) {
                char replacement = alphabet[alphabet_indicies[iii]];
                size_t mut_idx = gsl_combination_get(mut_idx_comb, iii);
                if (tmp[mut_idx] == replacement) {
                    continue;
                }
                tmp[mut_idx] = replacement;
            }
            if (strncmp(str, tmp, len) == 0 && !keep_original) {
                qes_free(tmp);
                continue;
            } else {
                if (results + 1 > results_alloced) {
                    results_alloced = qes_roundupz(results_alloced);
                    result = qes_realloc(result,
                                         results_alloced * sizeof(*result));
                }
                result[results++] = strndup(tmp, len);
                qes_free(tmp);
            }
        }
    } while (gsl_combination_next(mut_idx_comb) == GSL_SUCCESS);
    gsl_combination_free(mut_idx_comb);
    qes_free(alphabet_indicies);
    *n_results_o = results;
    return result;
}

struct axe_trie *
axe_trie_create(void)
{
    struct axe_trie *trie = NULL;
    AlphaMap *map = alpha_map_new();
    int ret = 0;

    /* Make trie AlphaMap */
    if (map == NULL) {
        return NULL;
    }
#define _AM_ADD(chr)                                                    \
    ret = alpha_map_add_range(map, chr, chr);                               \
    if (ret != 0) {                                                         \
        fprintf(stderr, "[trie_create] Failed to add char %c to alphamap\n",\
                chr);                                                       \
        alpha_map_free(map);                                                \
        return NULL;                                                        \
    }
    _AM_ADD('A')
    _AM_ADD('C')
    _AM_ADD('G')
    _AM_ADD('T')
    _AM_ADD('N')
#undef _AM_ADD
    trie = qes_calloc(1, sizeof(*trie));
    trie->trie = trie_new(map);
    if (trie->trie == NULL) {
        qes_free(trie);
        alpha_map_free(map);
        return NULL;
    }
    alpha_map_free(map);
    return trie;
}

void
axe_trie_destroy_(struct axe_trie *trie)
{
    if (trie != NULL) {
        /* trie_free doesn't check for null, so we better */
        if (trie->trie != NULL) {
            trie_free(trie->trie);
        }
        qes_free(trie);
    }
}

inline int
axe_trie_get(struct axe_trie *trie, const char *str, intptr_t *data)
{
    if (!axe_trie_ok(trie) || str == NULL) return -1;
    return trie_retrieve(trie->trie, str, data);
}

inline int
axe_trie_delete(struct axe_trie *trie, const char *str)
{
    if (!axe_trie_ok(trie) || str == NULL) return -1;
    return trie_delete(trie->trie, str);
}

inline int
axe_trie_add(struct axe_trie *trie, const char *str, intptr_t data)
{
    if (!axe_trie_ok(trie) || str == NULL) return -1;
    if (trie_store_if_absent(trie->trie, str, data)) {
        return 0;
    }
    return 1;
}

inline int
axe_match_read (ssize_t *value, struct axe_trie *trie,
                const struct qes_seq *seq)
{
    TrieState *trie_iter = NULL;
    TrieState *last_good_state = NULL;
    size_t seq_pos = 0;

    /* value is set to -1 on anything bad happening including failed lookup */
    if (value == NULL || !axe_trie_ok(trie) || !qes_seq_ok(seq)) {
        return -1;
    }
    /* Set *value here, then we just don't update it on error */
    *value = -1;
    if (seq->seq.len < trie->min_len) {
        return 1;
    }
    /* Only look until the maximum of the largest barcode, or seq len */
    /* Grab tree root iter, and check it. */
    trie_iter = trie_root(trie->trie);
    if (trie_iter == NULL) {
        fprintf(stderr, "[match_read] trie_root() returned NULL!\n");
        return -1;
    }
    /* Consume seq until we can't */
    do {
        trie_state_walk(trie_iter, seq->seq.str[seq_pos]);
        if (trie_state_is_terminal(trie_iter)) {
            if (last_good_state != NULL) {
                trie_state_free(last_good_state);
            }
            last_good_state = trie_state_clone(trie_iter);
        }
    } while (trie_state_is_walkable(trie_iter, seq->seq.str[++seq_pos]));
    /* If we get to a terminal state, then great! */
    if (trie_state_is_terminal(trie_iter)) {
        trie_state_walk(trie_iter, '\0');
        trie_state_free(last_good_state);
        *value =  (ssize_t) trie_state_get_data(trie_iter);
        trie_state_free(trie_iter);
        return 0;
    } else if (last_good_state != NULL) {
        trie_state_free(trie_iter);
        trie_state_walk(last_good_state, '\0');
        *value =  (ssize_t) trie_state_get_data(last_good_state);
        trie_state_free(last_good_state);
        return 0;
    }
    trie_state_free(trie_iter);
    if (last_good_state != NULL) {
        trie_state_free(last_good_state);
    }
    return 1;
}

int
axe_write_table(const struct axe_config *config)
{
    FILE *tab_fp = NULL;
    struct axe_barcode *this_bcd = NULL;
    size_t iii = 0;
    int res = 0;

    if (!axe_config_ok(config)) {
        return -1;
    }
    if (config->table_file == NULL) {
        /* we always call this function in the main loop, so we bail out here
           if we don't have a file to write it to. */
        return 0;
    }
    tab_fp = fopen(config->table_file, "w");
    if (tab_fp == NULL) {
        fprintf(stderr, "[write_table] ERROR: Could not open %s\n%s\n",
                config->table_file, strerror(errno));
        return 1;
    }
    if (config->match_combo) {
        fprintf(tab_fp, "R1Barcode\tR2Barcode\tSample\tCount\n");
    } else {
        fprintf(tab_fp, "Barcode\tSample\tCount\n");
    }
    for (iii = 0; iii < config->n_barcode_pairs; iii++) {
        this_bcd = config->barcodes[iii];
        if (config->match_combo) {
            fprintf(tab_fp, "%s\t%s\t%s\t%" PRIu64 "\n", this_bcd->seq1,
                    this_bcd->seq2, this_bcd->id, this_bcd->count);
        } else {
            fprintf(tab_fp, "%s\t%s\t%" PRIu64 "\n", this_bcd->seq1,
                    this_bcd->id, this_bcd->count);
        }
    }
    if (config->match_combo) {
        fprintf(tab_fp, "N\tN\tNo Barcode\t%" PRIu64 "\n",
                config->reads_failed);
    } else {
        fprintf(tab_fp, "N\tNo Barcode\t%" PRIu64 "\n", config->reads_failed);
    }
    res = fclose(tab_fp);
    if (res != 0) {
        fprintf(stderr, "[write_table] ERROR: Couldn't close tab file %s\n%s\n",
                config->table_file, strerror(errno));
        return 1;
    }
    return 0;
}

int
axe_print_summary(const struct axe_config *config, FILE *stream)
{
    const char *tmp;

#define print(...)  fprintf(stream, __VA_ARGS__)
    if (!axe_config_ok(config)) {
        return -1;
    }
    if (config->verbosity < 0) {
        /* Say nothing if we're being quiet */
        return 0;
    }
    fprintf(stream, "\nRun Summary:\n");
    if (config->verbosity > 1) {
        print("Being verbose (not that you'll notice)\n");
    }
    tmp = config->out_mode == READS_SINGLE ? "reads" : "read pairs";
    print("Processed %" PRIu64 " %s in %0.1f seconds (%0.3fk %s/sec)\n",
          config->reads_processed, tmp, config->time_taken,
          (float)(config->reads_processed / 1000) / config->time_taken, tmp);
    print("%" PRIu64 " %s contained valid barcodes\n",
          config->reads_demultiplexed, tmp);
    print("%" PRIu64 " %s could not be demultiplexed\n",
          config->reads_failed, tmp);
#undef print
    return 0;
}
