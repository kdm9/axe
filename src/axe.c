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

struct axe_barcode *
axe_barcode_create(void)
{
    struct axe_barcode *bcd = NULL;

    bcd = km_calloc(1, sizeof(*bcd));
    return bcd;
}

void
axe_barcode_destroy_(struct axe_barcode *barcode)
{
    if (!axe_barcode_ok(barcode)) return;
    km_free(barcode->seq1);
    km_free(barcode->seq2);
    km_free(barcode->id);
    barcode->len1 = 0;
    barcode->len2 = 0;
    km_free(barcode);
}

struct axe_config *
axe_config_create(void)
{
    struct axe_config *config = km_calloc(1, sizeof(*config));

    /* km_calloc never returns null, we use errprintexit as the err handler */
    return config;
}

void
axe_config_destroy_(struct axe_config *config)
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
        if (config->n_barcodes_2 == 0) {
            /* Special case, as we store it as though we have a reverse read
               barcode & we can't do ``for (;0<0;) {blah}`` or nothing gets
               written out. */
            axe_output_destroy(config->outputs[iii][0]);
        } else {
            for (jjj = 0; jjj < config->n_barcodes_2; jjj++) {
                axe_output_destroy(config->outputs[iii][jjj]);
            }
        }
        km_free(config->outputs[iii]);
    }
    km_free(config->outputs);
    for (iii = 0; iii < config->n_barcode_pairs; iii++) {
        axe_barcode_destroy(config->barcodes[iii]);
    }
    km_free(config->barcodes);
    axe_output_destroy(config->unknown_output);
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
        res = snprintf(buf, 4096, "%s_%s.%s", prefix, id, ext);
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
    out = km_calloc(1, sizeof(*out));
    out->mode = mode;
    out->fwd_file = seqfile_create(fwd_fpath, fp_mode);
    seqfile_set_format(out->fwd_file, FASTQ_FMT);
    if (out->fwd_file == NULL) {
        km_free(out);
        return NULL;
    }
    if (rev_fpath != NULL) {
        out->rev_file = seqfile_create(rev_fpath, fp_mode);
        if (out->rev_file == NULL) {
            seqfile_destroy(out->fwd_file);
            km_free(out);
            return NULL;
        }
        seqfile_set_format(out->rev_file, FASTQ_FMT);
    } else {
        out->rev_file = NULL;
    }
    out->count = 0;
    return out;
}

void
axe_output_destroy_(struct axe_output *output)
{
    if (output != NULL) {
        seqfile_destroy(output->fwd_file);
        seqfile_destroy(output->rev_file);
        output->mode = READS_UNKNOWN;
        output->count = 0llu;
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
        return NULL;
    }
    barcode = axe_barcode_create();
    if (barcode != NULL) {
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
    zfile_t *zf = NULL;
    struct axe_barcode *this_barcode = NULL;
    struct axe_barcode **barcodes = NULL;
    size_t n_barcodes = 0;
    size_t n_barcodes_alloced = 8;
    char *line = NULL;
    size_t linesz = 0;
    ssize_t linelen = 0;
    size_t iii = 0;
    int tmp = 0;
    struct axe_trie *seq1_trie = NULL;
    struct axe_trie *seq2_trie = NULL;

    if (!axe_config_ok(config)) {
        return -1;
    }
    barcodes = km_calloc(n_barcodes_alloced, sizeof(*barcodes));
    zf = zfopen(config->barcode_file, "r");
    line = km_malloc(128);
    if (config->match_combo) {
        seq1_trie = axe_trie_create();
        seq2_trie = axe_trie_create();
    }
    while ((linelen = zfreadline_realloc(zf, &line, &linesz)) > 0) {
        if (strncmp(line, "Barcode", 7) == 0 || \
            strncmp(line, "barcode", 7) == 0) {
            continue;
        }
        if (n_barcodes + 1 >= n_barcodes_alloced) {
            n_barcodes_alloced *= 2;
            barcodes = km_realloc(barcodes,
                                  n_barcodes_alloced * sizeof(*barcodes));
        }
        if (config->match_combo) {
            this_barcode = read_barcode_combo(line);
            barcodes[n_barcodes++] = this_barcode;
            if (this_barcode == NULL) {
                goto error;
            }
            if (!trie_retrieve(seq1_trie->trie, this_barcode->seq1, &tmp)) {
                axe_trie_add(seq1_trie, this_barcode->seq1, 0);
            }
            if (!trie_retrieve(seq2_trie->trie, this_barcode->seq2, &tmp)) {
                axe_trie_add(seq2_trie, this_barcode->seq2, 0);
            }
        } else {
            this_barcode = read_barcode_single(line);
            barcodes[n_barcodes++] = this_barcode;
            if (this_barcode == NULL) {
                goto error;
            }
        }
    }
    config->barcodes = barcodes;
    config->n_barcode_pairs = n_barcodes;
    config->n_barcodes_1 = n_barcodes;
    config->n_barcodes_2 = 0;
    return 0;
error:
    if (barcodes != NULL) {
        for (iii = 0; iii < n_barcodes - 1; iii++) {
            axe_barcode_destroy(barcodes[iii]);
        }
    }
    return 1;
}

int
axe_make_tries(struct axe_config *config)
{
    size_t iii = 0;

    if (!axe_config_ok(config)) {
        return -1;
    }
    /* We need 1 trie for 0 mm, so add 1 to cfg->mm */
    config->fwd_tries = km_malloc((config->mismatches + 1) *
                                  sizeof(*config->fwd_tries));
    for (iii = 0; iii <= config->mismatches; iii++) {
        config->fwd_tries[iii] = axe_trie_create();
        if (config->fwd_tries[iii] == NULL) {
            fprintf(stderr,
                    "[make_tries] ERROR: axe_trie_create returned NULL\n");
            km_free(config->fwd_tries);
            return 1;
        }
    }
    if (config->match_combo) {
        config->rev_tries = km_malloc(config->mismatches * \
                                      sizeof(*config->rev_tries));
        for (iii = 0; iii <= config->mismatches; iii++) {
            config->rev_tries[iii] = axe_trie_create();
            if (config->rev_tries[iii] == NULL) {
                km_free(config->rev_tries);
                return 1;
            }
        }
    }
    return 0;
}

static char *
axe_make_file_ext(const struct axe_config *config)
{
    if (!axe_config_ok(config)) {
        return NULL;
    }
    if (config->out_mode == READS_INTERLEAVED) {
        if (config->out_compress_level > 1) {
            return strdup("ilfq.gz");
        }
        return strdup("ilfq");
    }
    if (config->out_compress_level > 1) {
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
    if (config->out_compress_level > 1) {
        char tmp[10] = "";
        snprintf(tmp, 9, "w%d", config->out_compress_level);
        return strdup(tmp);
    }
    return strdup("wT");
}

static inline int
make_outputs_combo(struct axe_config *config)
{
    size_t bcd1 = 0;
    size_t bcd2 = 0;

    if (!axe_config_ok(config)) {
        return -1;
    }
    /* Open barcode files */
    for (bcd1 = 0; bcd1 < config->n_barcodes_1; bcd1++) {
        for (bcd2 = 0; bcd2 < config->n_barcodes_2; bcd2++) {
        }
    }
    return -1;
}

static inline int
load_tries_make_outputs_combo(struct axe_config *config)
{
    /* TODO here */
    int bcd1 = 0;
    int bcd2 = 0;
    int retval = 0;
    char **mutated = NULL;
    size_t num_mutated = 0;
    int ret = 0;
    size_t iii = 0;
    size_t jjj = 0;
    size_t mmm = 0;
    struct axe_barcode *this_bcd = NULL;

    /* TODO: remove this once this func is implemented. */
    return -1;
    if (!axe_config_ok(config)) {
        fprintf(stderr, "[load_tries] Bad config\n");
        return -1;
    }
    /* Make mutated barcodes and add to trie */
    for (iii = 0; iii < config->n_barcode_pairs; iii++) {
        this_bcd = config->barcodes[iii];
        if (!axe_barcode_ok(this_bcd)) {
            fprintf(stderr, "[load_tries] Bad barcode at %zu\n", iii);
            return -1;
        }
        /* Either lookup the index of the first read in the barcode table, or
         * insert this barcode into the table, storing its index.
         * Note the NOT here. */
        if (!trie_retrieve(config->fwd_tries[0]->trie, this_bcd->seq1, &bcd1)) {
            bcd1 = config->n_barcodes_1++;
            ret = axe_trie_add(config->fwd_tries[0], this_bcd->seq1, iii);
            if (ret != 0) {
                fprintf(stderr, "ERROR: Could not load barcode %s into trie %zu\n",
                        this_bcd->seq1, iii);
                return 1;
            }
        }
        /* Likewise for the reverse read index */
        if (!trie_retrieve(config->rev_tries[0]->trie, this_bcd->seq2, &bcd2)) {
            bcd2 = config->n_barcodes_2++;
            ret = axe_trie_add(config->rev_tries[0], this_bcd->seq2, iii);
            if (ret != 0) {
                fprintf(stderr, "ERROR: Could not load barcode %s into trie %zu\n",
                        this_bcd->seq2, iii);
                return 1;
            }
        }
        for (jjj = 1; jjj <= config->mismatches; jjj++) {
            /* Do the forwards read barcode */
            mutated = hamming_mutate_dna(&num_mutated, this_bcd->seq1,
                                         this_bcd->len1, jjj, 0);
            for (mmm = 0; mmm < num_mutated; mmm++) {
                ret = axe_trie_add(config->fwd_tries[jjj], mutated[mmm], iii);
                if (ret != 0) {
                    fprintf(stderr, "%s: Barcode confict! %s already in trie (%dmm)",
                            config->ignore_barcode_confict ? "WARNING": "ERROR",
                            mutated[mmm], (int)jjj);
                    return 1;
                }
                km_free(mutated[mmm]);
            }
            km_free(mutated);
            /* Ditto for the reverse read */
            mutated = hamming_mutate_dna(&num_mutated, this_bcd->seq2,
                                         this_bcd->len2, iii, 0);
            for (mmm = 0; mmm < num_mutated; mmm++) {
                ret = axe_trie_add(config->fwd_tries[jjj], mutated[mmm], iii);
                if (ret != 0) {
                    fprintf(stderr, "%s: Barcode confict! %s already in trie (%dmm)",
                            config->ignore_barcode_confict ? "WARNING": "ERROR",
                            mutated[mmm], (int)jjj);
                    return 1;
                }
                km_free(mutated[mmm]);
            }
            km_free(mutated);
        }
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
    int tmp = 0;
    struct axe_barcode *this_bcd = NULL;

    if (!axe_config_ok(config)) {
        fprintf(stderr, "[load_tries] Bad config\n");
        return -1;
    }
    /* Make mutated barcodes and add to trie */
    for (iii = 0; iii < config->n_barcodes_1; iii++) {
        this_bcd = config->barcodes[iii];
        if (!axe_barcode_ok(this_bcd)) {
            fprintf(stderr, "[load_tries] Bad barcode at %zu\n", iii);
            return -1;
        }
        /* Either lookup the index of the first read in the barcode table, or
         * insert this barcode into the table, storing its index.
         * Note the NOT here. */
        if (!trie_retrieve(config->fwd_tries[0]->trie, this_bcd->seq1, &tmp)) {
            ret = axe_trie_add(config->fwd_tries[0], this_bcd->seq1, iii);
            if (ret != 0) {
                fprintf(stderr,
                        "ERROR: Could not load barcode %s into trie %zu\n",
                        this_bcd->seq1, iii);
                return 1;
            }
            TBD_DEBUG_LOG("[load_tries] New r1 barcode (not in trie)\n");
        } else {
            fprintf(stderr, "ERROR: Duplicate barcode %s\n", this_bcd->seq1);
            return 1;
        }
        for (jjj = 1; jjj <= config->mismatches; jjj++) {
            TBD_DEBUG_LOG("[load_tries] Mutating r1 barcode\n");
            mutated = hamming_mutate_dna(&num_mutated, this_bcd->seq1,
                                         this_bcd->len1, jjj, 0);
            TBD_DEBUG_LOG("[load_tries] Mutated r1 barcode\n");
            for (mmm = 0; mmm < num_mutated; mmm++) {
                ret = axe_trie_add(config->fwd_tries[jjj], mutated[mmm], iii);
                if (ret != 0) {
                    fprintf(stderr,
                            "[load_barcodes] %s: Barcode %s already in trie (%dmm)\n",
                            config->ignore_barcode_confict ? "WARNING": "ERROR",
                            mutated[mmm], (int)jjj);
                    return 1;
                }
                km_free(mutated[mmm]);
            }
            km_free(mutated);
            num_mutated = 0;
        }
    }
    return 0;
}

static inline int
make_outputs_single(struct axe_config *config)
{
    size_t iii = 0;
    char *name_fwd = NULL;
    char *file_ext = NULL;
    char *zmode = NULL;
    struct axe_barcode *this_bcd = NULL;

    if (!axe_config_ok(config)) {
        fprintf(stderr, "[make_outputs] Bad config\n");
        return -1;
    }
    file_ext = axe_make_file_ext(config);
    zmode = axe_make_zmode(config);
    config->outputs = km_calloc(config->n_barcodes_1,
                                 sizeof(*config->outputs));
    for (iii = 0; iii < config->n_barcodes_1; iii++) {
        this_bcd = config->barcodes[iii];
        config->outputs[iii] = km_calloc(1, sizeof(**config->outputs));
        /* Open barcode files */
        if (config->out_mode == READS_SINGLE) {
            name_fwd = _axe_format_outfile_path(config->out_prefixes[0],
                                                this_bcd->id, 1, file_ext);
        } else if (config->out_mode == READS_INTERLEAVED) {
            name_fwd =  _axe_format_outfile_path(config->out_prefixes[0],
                                                this_bcd->id, 0, file_ext);
        }
        config->outputs[iii][0] = axe_output_create(name_fwd, NULL,
                                                    config->in_mode, zmode);

        if (config->outputs[iii][0] == NULL) {
            fprintf(stderr, "[make_outputs] couldn't create file at %s\n",
                    name_fwd);
            goto error;
        }
    }
    config->unknown_output = axe_output_create(config->unknown_files[0],
                                               config->unknown_files[1],
                                               config->in_mode, zmode);
    return 0;
error:
    return 1;
}

int
axe_load_tries(struct axe_config *config)
{
    if (!axe_config_ok(config)) {
        return -1;
    }
    if (config->match_combo) {
        return load_tries_make_outputs_combo(config);
    } else {
        return load_tries_single(config);
    }
}

int
axe_make_outputs(struct axe_config *config)
{
    if (!axe_config_ok(config)) {
        return -1;
    }
    if (config->match_combo) {
        return load_tries_make_outputs_combo(config);
    } else {
        return make_outputs_single(config);
    }
}


static int
process_file_single(struct axe_config *config)
{
    seqfile_t *insf = NULL;
    struct axe_output *outsf1 = NULL;
    intptr_t bcd1 = -1;
    size_t iii = 0;
    int ret = 0;
    size_t bcd1_len = 0;
    int have_error = 0;

    if (!axe_config_ok(config)) {
        fprintf(stderr, "[process_file] Bad config struct\n");
        return -1;
    }
    insf = seqfile_create(config->infiles[0], "r");
    if (insf == NULL) {
        fprintf(stderr, "[process_file] Couldn't open seqfile %s\n",
                config->infiles[0]);
        goto error;
    }
    switch(config->in_mode) {
        case READS_SINGLE:
            goto single;
            break;
        case READS_INTERLEAVED:
            goto interleaved;
            break;
        case READS_PAIRED:
        case READS_UNKNOWN:
        default:
            fprintf(stderr, "[process_file_single] Bad infile mode %i\n",
                    config->in_mode);
            goto error;
            break;
    }
single:
    SEQFILE_ITER_SINGLE_BEGIN(insf, seq, seqlen)
        ret = 1;
        for (iii = 0; iii <= config->mismatches; iii++) {
            ret = axe_match_read(&bcd1, config->fwd_tries[iii], seq);
            if (ret == 0) {
                break;
            }
        }
        if (ret != 0) {
            /* No match */
            seqfile_write(config->unknown_output->fwd_file, seq);
            continue;
        }
        /* Found a match */
        outsf1 = config->outputs[bcd1][0];
        bcd1_len = config->barcodes[bcd1]->len1;
        config->barcodes[bcd1]->count++;

        if (seq->seq.l <= bcd1_len) {
            /* Don't write out seqs shorter than the barcode */
            continue;
        }
        /* Bit of the ol' switcheroo. We keep the seq's char pointers, so we
           need to switch them back to their orig. values, but don't want to
           copy. Kludgy, I know. */
        seq->seq.s += bcd1_len;
        seq->seq.l -= bcd1_len;
        seq->qual.s += bcd1_len;
        seq->qual.l -= bcd1_len;
        ret = seqfile_write(outsf1->fwd_file, seq);
        if (ret < 1) {
            fprintf(stderr,
                    "[process_file] Error: writing to outfile %s failed\n%s\n",
                    outsf1->fwd_file->zf->path, zferror(outsf1->fwd_file->zf));
            have_error = 1;
            seq->seq.s -= bcd1_len;
            seq->seq.l += bcd1_len;
            seq->qual.s -= bcd1_len;
            seq->qual.l += bcd1_len;
            break;
        }
        seq->seq.s -= bcd1_len;
        seq->seq.l += bcd1_len;
        seq->qual.s -= bcd1_len;
        seq->qual.l += bcd1_len;
        outsf1->count++;
    SEQFILE_ITER_SINGLE_END(seq)
    if (!have_error) goto clean_exit;
    else goto error;
interleaved:
    SEQFILE_ITER_INTERLEAVED_BEGIN(insf, seq1, seq2, seqlen1, seqlen2)

    SEQFILE_ITER_INTERLEAVED_END(seq1, seq2)
    if (!have_error) goto clean_exit;
    else goto error;
clean_exit:
    seqfile_destroy(insf);
    return 0;
error:
    seqfile_destroy(insf);
    return 1;
}

int
axe_process_file(struct axe_config *config)
{
    if (!axe_config_ok(config)) {
        return -1;
    }
    if (config->match_combo) {
        fprintf(stderr, "Combo not Implemented\n");
        return -2;
    } else {
        return process_file_single(config);
    }
}

int
combinations(int64_t len, int64_t elem, uintptr_t *choices, int at_start)
{
    ssize_t iii = 0;
    if (len < elem || choices == NULL) {
        /* error value, so don't use (!ret) as your test for the end of the
           enclosing while loop, or on error you'll have an infinite loop */
        return -1;
    }
    /* Check if we're at the start, i.e. all items are 0 */
    if (at_start) {
        /* In the first iteration, we set the choices to the first ``elem``
           valid choices, i.e., 0 ... elem - 1 */
        for (iii = 0; iii < elem; iii++) {
            choices[iii] = iii;
        }
        return 1;
    }
    /* Can we increment the final element? */
    if (choices[elem - 1] < (uintptr_t)(len - 1)) {
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
    while ((mut_ret = combinations(len, dist, mut_indicies, !mut_ret)) == 1) {
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
axe_trie_add (struct axe_trie *trie, const char *seq, uintptr_t data)
{
    if (!axe_trie_ok(trie) || seq == NULL) return -1;
    if (trie_store_if_absent(trie->trie, seq, data)) {
        return 0;
    }
    return 1;
}

inline int
axe_match_read (intptr_t *value, struct axe_trie *trie, const seq_t *seq)
{
    TrieState *trie_iter = NULL;
    size_t seq_pos = 0;
    size_t max_match_len = 0;
    int res = 0;
    intptr_t our_val = -1;

    /* *value is set to -1 on anything bad happening including failed lookup */
    if (value == NULL || !axe_trie_ok(trie) || !seq_ok(seq)) {
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
        fprintf(stderr, "[match_read] trie_root() returned NULL!\n");
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
    /* We walk the final step w/ EOS char, to "get" the data assocaited w/
       barcode key */
    if (trie_state_walk(trie_iter, TRIE_CHAR_TERM)) {
        our_val = trie_state_get_data(trie_iter);
    }
    trie_state_free(trie_iter);
    *value = our_val;
    if (our_val != -1) {
        /* we've succeeded */
        return 0;
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
            fprintf(tab_fp, "%s\t%s\t%s\t%" PRIu64 "\n", this_bcd->seq1, this_bcd->seq2,
                    this_bcd->id, this_bcd->count);
        } else {
            fprintf(tab_fp, "%s\t%s\t%" PRIu64 "\n", this_bcd->seq1,
                    this_bcd->id, this_bcd->count);
        }
    }
    res = fclose(tab_fp);
    if (res != 0) {
        fprintf(stderr, "[write_table] ERROR: Could not close FILE * %p\n%s\n",
                tab_fp, strerror(errno));
        return 1;
    }
    return 0;
}

int
axe_print_summary(const struct axe_config *config, FILE *stream)
{
#define print(...) fprintf(stream, __VA_ARGS__)
    if (!axe_config_ok(config)) {
        return -1;
    }
    if (config->verbosity < 0) {
        /* Say nothing if we're being quiet */
        return 0;
    }
    if (config->verbosity > 1) {
        print("Being verbose");
    }

#undef print
    return 0;
}
