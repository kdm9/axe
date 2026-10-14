/*
 * ============================================================================
 *
 *       Filename:  axe.c
 *    Description:  Demultiplex reads by 5' barcodes
 *      Copyright:  2014-2016 Kevin Murray <kdmfoss@gmail.com>
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

#include "axe.h"
#include "gsl_combination.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* Holds the current timestamp, so we don't have to free the returned string
 * from now(). */
char _time_now[10] = "";

const char *progress_bar_chars = "|/-\\";
unsigned int format_call_number;

char *
axe_formatter(struct qes_log_entry *entry)
{
    char *buf = NULL;
    const char *colour = ANSIRST;
    const char *reset = ANSIRST;
    char marker = ' ';
    int res = 0;

    if (entry == NULL || entry->message == NULL) return NULL;

    if (entry->level <= QES_LOG_DEBUG) {
        marker = '.';
        colour = ANSIBEG ATDIM FGCYN ANSIEND;
        reset = "";
    } else if (entry->level == AXE_LOG_PROGRESS) {
        marker = progress_bar_chars[format_call_number++ % 4];
        colour = ANSIBEG ATNRM FGGRN ANSIEND;
    } else if (entry->level == AXE_LOG_BOLD) {
        marker = '\0';
        colour = ANSIBEG ATBLD FGCYN ANSIEND;
    } else if (entry->level <= QES_LOG_INFO) {
        marker = '\0';
        colour = ANSIBEG ATNRM FGGRN ANSIEND;
    } else if (entry->level <= QES_LOG_WARNING) {
        marker = '!';
        colour = ANSIBEG ATBLD FGRED ANSIEND;
    } else if (entry->level <= QES_LOG_ERROR) {
        marker = 'E';
        colour = ANSIBEG ATBLD FGMAG ANSIEND;
    } else {
        marker = 'F';
        colour = ANSIBEG ATBLD  FGRED ANSIEND;
    }
    if (marker == '\0') {
        res = asprintf(&buf, "%s%s%s", colour, entry->message, reset);
    } else {
        res = asprintf(&buf, "%s[%c] %s%s", colour, marker, entry->message, reset);
    }
    if (res > 0) {
        return buf;
    } else {
        return NULL;
    }
}

/* Axe barcode struct ctor/dtor */

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

/* Axe config struct ctor/dtor */

struct axe_config *
axe_config_create(void)
{
    struct axe_config *config = qes_calloc(1, sizeof(*config));

    config->logger = qes_logger_create();
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
    qes_log_message_debug(config->logger,
                          "Destroying config structure\n");
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
    /* Logger */
    qes_logger_destroy(config->logger);
    /* config stuct */
    qes_free(config);
}


static char *
_axe_format_outfile_path (const char *prefix, const char *id, int read,
                          const char *ext)
{
    char buf[4096];
    int res = 0;
    char *our_prefix = NULL;
    size_t prefix_len = 0;
    struct stat statbuff;

    if (prefix == NULL || id == NULL) {
        return NULL;
    }

    prefix_len = strlen(prefix);
    our_prefix = qes_malloc(prefix_len + 2);
    memcpy(our_prefix, prefix, prefix_len);
    our_prefix[prefix_len] = '\0';
    if (stat(prefix, &statbuff) == 0 && S_ISDIR(statbuff.st_mode)) {
        /* Our prefix is a directory, so files go inside it. Add a path
         * separator rather than '_', unless there already is one. */
        if (prefix_len == 0 || prefix[prefix_len - 1] != '/') {
            our_prefix[prefix_len] = '/';
            our_prefix[prefix_len + 1] = '\0';
        }
    } else {
        /* Append an underscore to prefix */
        our_prefix[prefix_len] = '_';
        our_prefix[prefix_len + 1] = '\0';
    }

    if (read > 0) {
        res = snprintf(buf, 4096, "%s%s_R%d.%s", our_prefix, id, read, ext);
    } else {
        res = snprintf(buf, 4096, "%s%s_il.%s", our_prefix, id, ext);
    }
    if (res >= 4096) {
        qes_free(our_prefix);
        return NULL;
    }
    qes_free(our_prefix);
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

/* Upper-case ``seq`` in place, and check that it only has bases that the
 * barcode tries can hold. */
static int
axe_barcode_seq_ok(char *seq)
{
    char *c = NULL;

    if (seq == NULL || seq[0] == '\0') {
        return 0;
    }
    for (c = seq; *c != '\0'; c++) {
        *c = (char)toupper((unsigned char)*c);
        if (strchr("ACGTN", *c) == NULL) {
            return 0;
        }
    }
    return 1;
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
    size_t jjj = 0;

    if (!axe_config_ok(config)) {
        return -1;
    }
    barcodes = qes_calloc(n_barcodes_alloced, sizeof(*barcodes));
    qf = qes_file_open(config->barcode_file, "r");
    line = qes_malloc(linesz);

    while ((linelen = qes_file_readline_realloc(qf, &line, &linesz)) > 0) {
        /* Skip blank lines */
        tmp = line;
        while (*tmp != '\0' && isspace((unsigned char)*tmp)) {
            tmp++;
        }
        if (*tmp == '\0') {
            continue;
        }
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
            qes_log_format_fatal(config->logger,
                                 "Couldn't parse barcode line '%s'\n", line);
            qes_log_message_warning(config->logger,
                                    "Check that the format is correct and has "
                                    "UNIX line endings.");
            goto error;
        }
        /* Barcodes are read into fixed-size buffers, so detect truncation. */
        if (this_barcode->len1 >= 99 || this_barcode->len2 >= 99 ||
                this_barcode->idlen >= 99) {
            qes_log_format_fatal(config->logger,
                    "Barcode or ID is too long (max 98 characters) in "
                    "line '%s'\n", line);
            goto error;
        }
        if (!axe_barcode_seq_ok(this_barcode->seq1) ||
                (config->match_combo &&
                 !axe_barcode_seq_ok(this_barcode->seq2))) {
            qes_log_format_fatal(config->logger,
                    "Barcodes may only contain A, C, G, T or N, in "
                    "line '%s'\n", line);
            goto error;
        }
        /* Replace all bad chars with '-' */
        tmp = strpbrk(this_barcode->id, bad_fname_chars);
        while (tmp != NULL) {
            *tmp = '-'; /* Replace with dash */
            tmp = strpbrk(tmp + 1, bad_fname_chars);
        }
        /* Two samples with one ID would write to the same file */
        for (jjj = 0; jjj < n_barcode_pairs; jjj++) {
            if (strcmp(barcodes[jjj]->id, this_barcode->id) == 0) {
                qes_log_format_fatal(config->logger,
                        "Sample ID '%s' is used more than once in the "
                        "barcode file\n", this_barcode->id);
                goto error;
            }
        }
        /* Add the barcode to the array */
        barcodes[n_barcode_pairs++] = this_barcode;
        this_barcode = NULL;
    }
    if (linelen < -1) {
        qes_log_format_fatal(config->logger,
                             "Error reading barcode file %s\n",
                             config->barcode_file);
        goto error;
    }
    if (n_barcode_pairs == 0) {
        qes_log_format_fatal(config->logger,
                             "No barcodes found in barcode file %s\n",
                             config->barcode_file);
        goto error;
    }

    /* Save the array to the config struct */
    config->barcodes = barcodes;
    config->n_barcode_pairs = n_barcode_pairs;
    qes_file_close(qf);
    qes_free(line);
    if (config->verbosity > 0) {
        qes_log_format_info(config->logger,
                            "read_barcodes -- (%s) Read in barcodes\n",
                            nowstr());
    }
    return 0;

error:
    axe_barcode_destroy(this_barcode);
    for (iii = 0; iii < n_barcode_pairs; iii++) {
        axe_barcode_destroy(barcodes[iii]);
    }
    qes_free(barcodes);
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
        if (!axe_barcode_ok_combo(this_barcode)) {
            qes_log_format_fatal(config->logger,
                                 "setup_lookup -- Bad barcode at %zu\n",
                                 iii);
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
        if (config->barcode_lookup[bcd1][bcd2] >= 0) {
            qes_log_format_fatal(config->logger,
                    "setup_lookup -- Barcode pair %s,%s is used by both "
                    "%s and %s\n", this_barcode->seq1, this_barcode->seq2,
                    config->barcodes[config->barcode_lookup[bcd1][bcd2]]->id,
                    this_barcode->id);
            goto error;
        }
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
    qes_log_message_fatal(
            config->logger,
            "make_tries -- ERROR: axe_trie_create returned NULL\n");
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

struct axe_load_ctx {
    struct axe_trie *trie;
    int failed;
};

/* Copy the unambiguous mutants collected for one mismatch level into the
 * real trie. Ambiguous ones were marked with a negative value. */
static bool
axe_load_candidate_cb(const AlphaChar *key, TrieData data, void *user_data)
{
    struct axe_load_ctx *ctx = user_data;

    if (data < 0) {
        return true;
    }
    if (axe_trie_add(ctx->trie, key, data) != 0) {
        ctx->failed = 1;
        return false;
    }
    return true;
}

/* Load ``n`` barcode sequences into ``trie``, as well as all of their mutants
 * of up to ``config->mismatches`` substitutions. ``idx[i]`` is the value that
 * ``seqs[i]`` and its mutants map to. Sequences must be unique.
 *
 * The exact barcodes are all loaded first, then mutants are added one
 * mismatch level at a time, so that a closer match always wins. A mutant that
 * collides with a closer entry, or with another barcode's mutant at the same
 * level, is an error unless --permissive was given. In that case such mutants
 * are left out (so those reads only match a barcode more closely, or not at
 * all). */
static int
axe_load_trie(struct axe_config *config, struct axe_trie *trie,
              const char *what, char *const *seqs, const size_t *lens,
              const intptr_t *idx, size_t n)
{
    size_t level = 0;
    size_t iii = 0;
    size_t mmm = 0;

    for (iii = 0; iii < n; iii++) {
        if (axe_trie_add(trie, seqs[iii], idx[iii]) != 0) {
            qes_log_format_fatal(config->logger,
                    "load_tries -- Could not load %s barcode %s into trie "
                    "(is it a duplicate?)\n", what, seqs[iii]);
            return 1;
        }
    }
    for (level = 1; level <= config->mismatches; level++) {
        struct axe_trie *cand = axe_trie_create();
        struct axe_load_ctx ctx = {trie, 0};
        size_t n_shadowed = 0;
        size_t n_ambiguous = 0;
        size_t n_cand = 0;

        if (cand == NULL) {
            return 1;
        }
        for (iii = 0; iii < n; iii++) {
            char **mutated = NULL;
            size_t num_mutated = 0;

            if (lens[iii] < level) {
                /* Can't have more mismatches than bases */
                continue;
            }
            mutated = hamming_mutate_dna(&num_mutated, seqs[iii], lens[iii],
                                         level, 0);
            if (mutated == NULL) {
                qes_log_format_fatal(config->logger,
                        "load_tries -- Could not mutate barcode %s\n",
                        seqs[iii]);
                axe_trie_destroy(cand);
                return 1;
            }
            for (mmm = 0; mmm < num_mutated; mmm++) {
                intptr_t other = -1;
                int conflict = 0;

                if (axe_trie_get(trie, mutated[mmm], &other)) {
                    /* Collides with an exact barcode, or a closer mutant */
                    conflict = other != idx[iii];
                    if (conflict && config->permissive) {
                        n_shadowed++;
                    }
                } else if (axe_trie_get(cand, mutated[mmm], &other)) {
                    /* Equally close to some other barcode: ambiguous */
                    conflict = other != idx[iii];
                    if (conflict && config->permissive) {
                        if (other >= 0) {
                            trie_store(cand->trie, mutated[mmm], -2);
                            n_ambiguous++;
                        }
                    }
                } else if (axe_trie_add(cand, mutated[mmm], idx[iii]) != 0) {
                    qes_log_format_fatal(config->logger,
                            "load_tries -- Could not load mutant of %s\n",
                            seqs[iii]);
                    goto fail;
                } else {
                    n_cand++;
                }
                if (conflict && !config->permissive) {
                    qes_log_format_fatal(config->logger,
                            "load_tries -- %s barcode %s with %zu mismatch(es)"
                            " (%s) is also within %zu mismatch(es) of another "
                            "barcode. Use fewer mismatches, or --permissive.\n",
                            what, seqs[iii], level, mutated[mmm], level);
                    goto fail;
                }
                qes_free(mutated[mmm]);
                continue;

fail:
                for (; mmm < num_mutated; mmm++) {
                    qes_free(mutated[mmm]);
                }
                qes_free(mutated);
                axe_trie_destroy(cand);
                return 1;
            }
            qes_free(mutated);
        }
        /* (enumerating an empty trie is not safe) */
        if (n_cand > 0 &&
                (!trie_enumerate(cand->trie, axe_load_candidate_cb, &ctx) ||
                 ctx.failed)) {
            qes_log_format_fatal(config->logger,
                    "load_tries -- Could not load %s barcodes with %zu "
                    "mismatch(es)\n", what, level);
            axe_trie_destroy(cand);
            return 1;
        }
        axe_trie_destroy(cand);
        if ((n_shadowed > 0 || n_ambiguous > 0) && config->verbosity >= 0) {
            qes_log_format_warning(config->logger,
                    "load_tries -- %s barcodes: %zu mutated barcodes with %zu "
                    "mismatch(es) conflict with a closer barcode, and %zu are "
                    "ambiguous. These will not be matched.\n", what,
                    n_shadowed, level, n_ambiguous);
        }
    }
    return 0;
}

static int
load_tries_combo(struct axe_config *config)
{
    size_t which = 0;
    size_t iii = 0;
    int ret = 0;

    if (!axe_config_ok(config)) {
        fprintf(stderr, "[load_tries] Bad config\n");
        return -1;
    }
    /* Unique barcodes of R1 then R2, numbered in order of first appearance.
     * This has to match setup_barcode_lookup_combo. */
    for (which = 0; which < 2 && ret == 0; which++) {
        struct axe_trie *seen = axe_trie_create();
        struct axe_trie *target = which == 0 ? config->fwd_trie :
                                               config->rev_trie;
        char **seqs = qes_calloc(config->n_barcode_pairs, sizeof(*seqs));
        size_t *lens = qes_calloc(config->n_barcode_pairs, sizeof(*lens));
        intptr_t *idx = qes_calloc(config->n_barcode_pairs, sizeof(*idx));
        size_t n = 0;
        intptr_t tmp = 0;

        for (iii = 0; iii < config->n_barcode_pairs && ret == 0; iii++) {
            struct axe_barcode *bcd = config->barcodes[iii];
            char *seq = which == 0 ? bcd->seq1 : bcd->seq2;
            size_t len = which == 0 ? bcd->len1 : bcd->len2;

            if (!axe_barcode_ok_combo(bcd)) {
                qes_log_format_fatal(config->logger,
                        "load_tries -- Bad barcode at %zu\n", iii);
                ret = 1;
            } else if (!axe_trie_get(seen, seq, &tmp)) {
                if (axe_trie_add(seen, seq, (intptr_t)n) != 0) {
                    ret = 1;
                    continue;
                }
                seqs[n] = seq;
                lens[n] = len;
                idx[n] = (intptr_t)n;
                n++;
            }
        }
        if (ret == 0) {
            ret = axe_load_trie(config, target, which == 0 ? "R1" : "R2",
                                seqs, lens, idx, n);
        }
        axe_trie_destroy(seen);
        qes_free(seqs);
        qes_free(lens);
        qes_free(idx);
    }
    return ret;
}

static int
load_tries_single(struct axe_config *config)
{
    char **seqs = NULL;
    size_t *lens = NULL;
    intptr_t *idx = NULL;
    size_t iii = 0;
    int ret = 0;

    if (!axe_config_ok(config)) {
        fprintf(stderr, "[load_tries] Bad config\n");
        return -1;
    }
    seqs = qes_calloc(config->n_barcode_pairs, sizeof(*seqs));
    lens = qes_calloc(config->n_barcode_pairs, sizeof(*lens));
    idx = qes_calloc(config->n_barcode_pairs, sizeof(*idx));
    for (iii = 0; iii < config->n_barcode_pairs; iii++) {
        if (!axe_barcode_ok(config->barcodes[iii])) {
            qes_log_format_fatal(config->logger,
                    "load_tries -- Bad barcode at %zu\n", iii);
            ret = 1;
            goto exit;
        }
        seqs[iii] = config->barcodes[iii]->seq1;
        lens[iii] = config->barcodes[iii]->len1;
        idx[iii] = (intptr_t)iii;
    }
    ret = axe_load_trie(config, config->fwd_trie, "R1", seqs, lens, idx,
                        config->n_barcode_pairs);

exit:
    qes_free(seqs);
    qes_free(lens);
    qes_free(idx);
    return ret;
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
    if (config->out_mode == READS_SINGLE &&
            (config->in_mode == READS_PAIRED ||
             config->in_mode == READS_INTERLEAVED) &&
            config->verbosity >= 0) {
        qes_log_message_warning(config->logger,
                "make_outputs -- Paired input, but no reverse/interleaved "
                "output given (-R or -I). Reverse reads will be discarded.\n");
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
    qes_free(name_fwd);
    qes_free(name_rev);
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

/* Write ``seq`` to ``sf`` with its first ``trim`` bases removed. The record is
 * temporarily advanced, and *always* restored before returning, so that the
 * seq can still be freed/reused. If the read is no longer than ``trim``, we
 * write a single N rather than an empty/underflowed record, so that read
 * counts (and mate pairing) are preserved. */
static int
write_trimmed(struct qes_seqfile *sf, struct qes_seq *seq, size_t trim)
{
    size_t qtrim = 0;
    ssize_t ret = 0;

    if (sf == NULL) {
        /* Output mode has no file for this read (e.g. R2 w/ single output) */
        return 1;
    }
    if (trim > 0 && seq->seq.len <= trim) {
        seq->seq.str[0] = 'N';
        seq->seq.str[1] = '\0';
        seq->seq.len = 1;
        if (seq->qual.len > 0) {
            /* Keep first qual 'base' */
            seq->qual.str[1] = '\0';
            seq->qual.len = 1;
        }
        trim = 0;
    }
    qtrim = trim < seq->qual.len ? trim : seq->qual.len;
    seq->seq.str += trim;
    seq->seq.len -= trim;
    seq->qual.str += qtrim;
    seq->qual.len -= qtrim;
    ret = qes_seqfile_write(sf, seq);
    seq->seq.str -= trim;
    seq->seq.len += trim;
    seq->qual.str -= qtrim;
    seq->qual.len += qtrim;
    return ret < 1;
}

/* Write a (possibly paired) matched read to its output, trimming ``trim1``
 * bases from seq1 and ``trim2`` from seq2. Returns 0 on success. */
static int
write_matched_reads(struct axe_config *config, struct axe_output *out,
                    struct qes_seq *seq1, struct qes_seq *seq2,
                    size_t trim1, size_t trim2)
{
    if (write_trimmed(out->fwd_file, seq1, trim1)) {
        qes_log_format_fatal(config->logger,
                "process_file -- Writing to file %s failed\n%s\n",
                out->fwd_file->qf->path, qes_file_error(out->fwd_file->qf));
        return 1;
    }
    if (seq2 == NULL) {
        return 0;
    }
    if (out->mode == READS_INTERLEAVED) {
        if (write_trimmed(out->fwd_file, seq2, trim2)) {
            qes_log_format_fatal(config->logger,
                    "process_file -- Writing to file %s failed\n%s\n",
                    out->fwd_file->qf->path,
                    qes_file_error(out->fwd_file->qf));
            return 1;
        }
    } else if (out->mode == READS_PAIRED) {
        if (write_trimmed(out->rev_file, seq2, trim2)) {
            qes_log_format_fatal(config->logger,
                    "process_file -- Writing to file %s failed\n%s\n",
                    out->rev_file->qf->path,
                    qes_file_error(out->rev_file->qf));
            return 1;
        }
    }
    return 0;
}

/* Write a read (pair) that could not be demultiplexed */
static int
write_unknown(struct axe_config *config, struct qes_seq *seq1,
              struct qes_seq *seq2)
{
    struct axe_output *out = config->unknown_output;
    struct qes_seqfile *dest = NULL;

    if (write_trimmed(out->fwd_file, seq1, 0)) {
        qes_log_format_fatal(config->logger,
                "process_file -- Writing to file %s failed\n%s\n",
                out->fwd_file->qf->path, qes_file_error(out->fwd_file->qf));
        return 1;
    }
    if (seq2 == NULL) {
        return 0;
    }
    dest = out->mode == READS_INTERLEAVED ? out->fwd_file : out->rev_file;
    /* With single-end output mode there is nowhere to write R2 */
    if (dest != NULL && write_trimmed(dest, seq2, 0)) {
        qes_log_format_fatal(config->logger,
                "process_file -- Writing to file %s failed\n%s\n",
                dest->qf->path, qes_file_error(dest->qf));
        return 1;
    }
    return 0;
}

static inline void
increment_reads_print_progress(struct axe_config *config)
{
    config->reads_processed++;
    if (config->reads_processed % 100000 == 0) {
        if (config->verbosity >= 0) {
            axe_format_progress(config->logger,
                                "%s: Processed %.1fM %s\r",
                                nowstr(),
                                (float)(config->reads_processed/1000000.0),
                    config->out_mode == READS_SINGLE ? "reads" : "read pairs");
        }
    }
}

static inline int
process_read_pair_single(struct axe_config *config, struct qes_seq *seq1,
                        struct qes_seq *seq2)
{
    int ret = 0;
    intptr_t bcd = -1;
    size_t barcode_pair_index = 0;
    struct axe_output *outfile = NULL;
    size_t bcd_len = 0;

    ret = axe_match_read(config, &bcd, config->fwd_trie, seq1);
    increment_reads_print_progress(config);
    if (ret != 0) {
        /* No match */
        config->reads_failed++;
        return write_unknown(config, seq1, seq2);
    }
    /* Found a match */
    if (bcd < 0 || (size_t)bcd >= config->n_barcodes_1) {
        qes_log_format_fatal(config->logger,
                             "process_file -- Bad barcode index %zd\n",
                             (ssize_t)bcd);
        return 1;
    }
    config->reads_demultiplexed++;
    barcode_pair_index = config->barcode_lookup[bcd][0];
    outfile = config->outputs[barcode_pair_index];
    bcd_len = config->barcodes[barcode_pair_index]->len1;
    config->barcodes[barcode_pair_index]->count++;
    return write_matched_reads(config, outfile, seq1, seq2, bcd_len,
                               config->trim_rev ? bcd_len : 0);
}

typedef int (*axe_read_func)(struct axe_config *, struct qes_seq *,
                             struct qes_seq *);

/* Iterate over all reads (pairs) in the input, calling ``func`` on each.
 * Unlike the QES_SEQFILE_ITER macros, this distinguishes the end of the file
 * from a malformed record, and complains if paired files differ in length.
 * Returns 0 on success, 1 on any error. */
static int
axe_iterate_reads(struct axe_config *config, struct qes_seqfile *fwdsf,
                  struct qes_seqfile *revsf, enum read_mode mode,
                  axe_read_func func)
{
    struct qes_seq *seq1 = qes_seq_create();
    struct qes_seq *seq2 = mode == READS_SINGLE ? NULL : qes_seq_create();
    const char *fwdname = config->infiles[0];
    int retval = 0;
    ssize_t r1 = 0;
    ssize_t r2 = 0;

    while (1) {
        r1 = qes_seqfile_read(fwdsf, seq1);
        if (r1 == EOF) {
            if (mode == READS_PAIRED &&
                    (r2 = qes_seqfile_read(revsf, seq2)) != EOF) {
                qes_log_format_fatal(config->logger,
                        "process_file -- %s has more reads than %s\n",
                        config->infiles[1], fwdname);
                retval = 1;
            }
            break;
        }
        if (r1 < 0) {
            qes_log_format_fatal(config->logger,
                    "process_file -- Malformed record after %zu reads in "
                    "%s (error %zd)\n", fwdsf->n_records, fwdname, r1);
            retval = 1;
            break;
        }
        if (mode == READS_PAIRED) {
            r2 = qes_seqfile_read(revsf, seq2);
            if (r2 == EOF) {
                qes_log_format_fatal(config->logger,
                        "process_file -- %s has more reads than %s\n",
                        fwdname, config->infiles[1]);
            } else if (r2 < 0) {
                qes_log_format_fatal(config->logger,
                        "process_file -- Malformed record after %zu reads "
                        "in %s (error %zd)\n", revsf->n_records,
                        config->infiles[1], r2);
            }
            if (r2 < 0) {
                retval = 1;
                break;
            }
        } else if (mode == READS_INTERLEAVED) {
            r2 = qes_seqfile_read(fwdsf, seq2);
            if (r2 == EOF) {
                qes_log_format_fatal(config->logger,
                        "process_file -- Interleaved file %s has an odd "
                        "number of reads\n", fwdname);
            } else if (r2 < 0) {
                qes_log_format_fatal(config->logger,
                        "process_file -- Malformed record after %zu reads "
                        "in %s (error %zd)\n", fwdsf->n_records, fwdname, r2);
            }
            if (r2 < 0) {
                retval = 1;
                break;
            }
        }
        if (func(config, seq1, seq2) != 0) {
            retval = 1;
            break;
        }
    }
    qes_seq_destroy(seq1);
    qes_seq_destroy(seq2);
    return retval;
}


static int
process_file_single(struct axe_config *config)
{
    struct qes_seqfile *fwdsf = NULL;
    struct qes_seqfile *revsf = NULL;
    int retval = 1;

    if (!axe_config_ok(config)) {
        return -1;
    }
    fwdsf = qes_seqfile_create(config->infiles[0], "r");
    if (fwdsf == NULL) {
        qes_log_format_fatal(config->logger,
                             "process_file -- Couldn't open seqfile %s\n",
                             config->infiles[0]);
        goto exit;
    }
    if (fwdsf->format == FASTA_FMT) {
        qes_log_format_fatal(config->logger,
                "process_file -- %s is FASTA, but only FASTQ input is "
                "supported\n", config->infiles[0]);
        goto exit;
    }
    switch (config->in_mode) {
    case READS_SINGLE:
    case READS_INTERLEAVED:
        break;
    case READS_PAIRED:
        revsf = qes_seqfile_create(config->infiles[1], "r");
        if (revsf == NULL) {
            qes_log_format_fatal(config->logger,
                                 "process_file -- Couldn't open seqfile %s\n",
                                 config->infiles[1]);
            goto exit;
        }
        break;
    case READS_UNKNOWN:
    default:
        qes_log_format_fatal(config->logger,
                             "process_file_single -- Bad infile mode %u\n",
                             config->in_mode);
        goto exit;
    }
    retval = axe_iterate_reads(config, fwdsf, revsf, config->in_mode,
                               process_read_pair_single);

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

    r1_ret = axe_match_read(config, &bcd1, config->fwd_trie, seq1);
    r2_ret = axe_match_read(config, &bcd2, config->rev_trie, seq2);
    increment_reads_print_progress(config);
    if (r1_ret != 0 || r2_ret != 0) {
        /* No match */
        config->reads_failed++;
        return write_unknown(config, seq1, seq2);
    }
    /* Found a match */
    if (bcd1 < 0 || bcd2 < 0 || (size_t)bcd1 >= config->n_barcodes_1 ||
            (size_t)bcd2 >= config->n_barcodes_2) {
        qes_log_format_fatal(config->logger,
                             "process_file -- Bad barcode index %zd,%zd\n",
                             (ssize_t)bcd1, (ssize_t)bcd2);
        return 1;
    }
    barcode_pair_index = config->barcode_lookup[bcd1][bcd2];
    if (barcode_pair_index < 0) {
        /* Invalid match */
        config->reads_failed++;
        return write_unknown(config, seq1, seq2);
    }
    config->reads_demultiplexed++;
    outfile = config->outputs[barcode_pair_index];
    bcd1_len = config->barcodes[barcode_pair_index]->len1;
    bcd2_len = config->barcodes[barcode_pair_index]->len2;
    config->barcodes[barcode_pair_index]->count++;
    return write_matched_reads(config, outfile, seq1, seq2, bcd1_len,
                               bcd2_len);
}


static int
process_file_combo(struct axe_config *config)
{
    struct qes_seqfile *fwdsf = NULL;
    struct qes_seqfile *revsf = NULL;
    int retval = 1;

    if (!axe_config_ok(config)) {
        return -1;
    }
    fwdsf = qes_seqfile_create(config->infiles[0], "r");
    if (fwdsf == NULL) {
        qes_log_format_fatal(config->logger,
                             "process_file -- Couldn't open seqfile %s\n",
                             config->infiles[0]);
        goto exit;
    }
    if (fwdsf->format == FASTA_FMT) {
        qes_log_format_fatal(config->logger,
                "process_file -- %s is FASTA, but only FASTQ input is "
                "supported\n", config->infiles[0]);
        goto exit;
    }
    switch (config->in_mode) {
    case READS_INTERLEAVED:
        break;
    case READS_PAIRED:
        revsf = qes_seqfile_create(config->infiles[1], "r");
        if (revsf == NULL) {
            qes_log_format_fatal(config->logger,
                                 "process_file -- Couldn't open seqfile %s\n",
                                 config->infiles[1]);
            goto exit;
        }
        break;
    case READS_SINGLE:
    case READS_UNKNOWN:
    default:
        qes_log_format_fatal(config->logger,
                             "process_file_combo -- Bad infile mode %u\n",
                             config->in_mode);
        goto exit;
    }
    retval = axe_iterate_reads(config, fwdsf, revsf, config->in_mode,
                               process_read_pair_combo);

exit:
    qes_seqfile_destroy(fwdsf);
    qes_seqfile_destroy(revsf);
    return retval;
}


/* Close an output, reporting whether the final flush/close succeeded. This
 * matters for gzipped output, where the last block is written on close. */
static int
axe_close_output_file(struct qes_seqfile *sf)
{
    int res = 0;

    if (sf == NULL || sf->qf == NULL || sf->qf->fp == NULL) {
        return 0;
    }
    res = QES_ZCLOSE(sf->qf->fp);
    sf->qf->fp = NULL;
    return res != 0;
}

static int
axe_close_outputs(struct axe_config *config)
{
    size_t iii = 0;
    int failed = 0;
    struct axe_output *out = NULL;

    for (iii = 0; iii <= config->n_barcode_pairs; iii++) {
        if (iii < config->n_barcode_pairs) {
            out = config->outputs != NULL ? config->outputs[iii] : NULL;
        } else {
            out = config->unknown_output;
        }
        if (out == NULL) {
            continue;
        }
        if (axe_close_output_file(out->fwd_file) ||
                axe_close_output_file(out->rev_file)) {
            failed = 1;
            qes_log_message_fatal(config->logger,
                    "process_file -- Failed to finish writing an output "
                    "file. Is the disk full?\n");
        }
    }
    return failed;
}

int
axe_process_file(struct axe_config *config)
{
    int ret = 0;
    struct timespec start = {0, 0};
    struct timespec end = {0, 0};

    if (!axe_config_ok(config)) {
        return -1;
    }
    clock_gettime(CLOCK_MONOTONIC, &start);
    if (config->verbosity >= 0) {
        axe_format_bold(config->logger,
                        "process_file -- (%s) Starting demultiplexing\n",
                        nowstr());
    }
    if (config->match_combo) {
        ret = process_file_combo(config);
    } else {
        ret = process_file_single(config);
    }
    if (axe_close_outputs(config) != 0) {
        ret = 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    config->time_taken = (float)(end.tv_sec - start.tv_sec) +
                         (float)(end.tv_nsec - start.tv_nsec) / 1e9f;
    if (config->verbosity >= 0) {
        /* Jump to new line so we don't clobber the progress bar */
        fprintf(stderr, "\n");
        axe_format_bold(config->logger,
                        "process_file -- (%s) Finished demultiplexing\n",
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

/* Generate all strings at *exactly* ``dist`` substitutions from ``str``, over
 * the alphabet ACGT. No string is returned twice. If ``keep_original`` is
 * set, ``str`` itself is also returned (once, first). Returns NULL on error,
 * or if ``dist`` > ``len``. */
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
    uintptr_t *alphabet_indicies;
    int alpha_ret = 0;
    gsl_combination *mut_idx_comb;

    if (n_results_o == NULL || str == NULL || len < 1 || dist < 1 ||
            dist > len || dist > n_letters) {
        return NULL;
    }
    result = qes_malloc(results_alloced * sizeof(*result));
    if (keep_original) {
        result[results++] = strndup(str, len);
    }
    alphabet_indicies = qes_calloc(dist, sizeof(*alphabet_indicies));
    mut_idx_comb = gsl_combination_calloc(len, dist);
    if (mut_idx_comb == NULL) {
        qes_free(alphabet_indicies);
        qes_free(result);
        return NULL;
    }
    do {
        while ((alpha_ret = product(n_letters, dist, alphabet_indicies,
                                    !alpha_ret)) == 1) {
            int is_exact = 1;

            tmp = strndup(str, len);
            for (iii = 0; iii < dist; iii++) {
                char replacement = alphabet[alphabet_indicies[iii]];
                size_t mut_idx = gsl_combination_get(mut_idx_comb, iii);
                if (tmp[mut_idx] == replacement) {
                    /* Not a substitution, so this has < dist mismatches. It
                     * is generated by a lower ``dist``. */
                    is_exact = 0;
                    break;
                }
                tmp[mut_idx] = replacement;
            }
            if (!is_exact) {
                qes_free(tmp);
                continue;
            }
            if (results + 1 > results_alloced) {
                results_alloced = qes_roundupz(results_alloced);
                result = qes_realloc(result,
                                     results_alloced * sizeof(*result));
            }
            result[results++] = tmp;
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

/* Find the barcode that is a prefix of ``seq`` (the longest, if more than one),
 * and store its trie value in ``*value``. Returns 0 on a match, 1 if there is
 * none, and -1 on error. ``*value`` is -1 unless a match is found. */
int
axe_match_read (struct axe_config *config, intptr_t *value,
                struct axe_trie *trie, const struct qes_seq *seq)
{
    TrieState *iter = NULL;
    TrieState *last_good = NULL;
    size_t seq_pos = 0;
    int ret = 1;

    if (value == NULL || !axe_trie_ok(trie) || !qes_seq_ok(seq)) {
        return -1;
    }
    *value = -1;
    iter = trie_root(trie->trie);
    last_good = trie_root(trie->trie);
    if (iter == NULL || last_good == NULL) {
        qes_log_message_fatal(config->logger,
                              "match_read -- trie_root() returned NULL!\n");
        ret = -1;
        goto exit;
    }
    /* Consume the read for as long as it follows a path in the trie, and no
     * further than its end. Whenever we are at the end of a barcode, remember
     * that state. */
    for (seq_pos = 0; seq_pos < seq->seq.len; seq_pos++) {
        if (!trie_state_walk(iter, seq->seq.str[seq_pos])) {
            break;
        }
        if (trie_state_is_terminal(iter)) {
            trie_state_copy(last_good, iter);
            ret = 0;
        }
    }
    if (ret == 0) {
        intptr_t data = -1;

        /* Step through the terminator to reach the leaf holding the data. */
        if (trie_state_walk(last_good, '\0')) {
            data = (intptr_t)trie_state_get_data(last_good);
        }
        if (data < 0) {
            ret = 1;
        } else {
            *value = data;
        }
    }

exit:
    trie_state_free(iter);
    trie_state_free(last_good);
    return ret;
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
        qes_log_format_fatal(config->logger, "write_table -- ERROR: Could not open %s\n%s\n",
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
        qes_log_format_error(config->logger,
                             "[write_table] Couldn't close tab file %s\n%s\n",
                              config->table_file, strerror(errno));
        return 1;
    }
    return 0;
}

/* Print a count as e.g. "12", "3.40K" or "1.20M" */
static const char *
axe_human_count(uint64_t n, char *buf, size_t bufsz)
{
    if (n >= 1000000) {
        snprintf(buf, bufsz, "%.2fM", (double)n / 1e6);
    } else if (n >= 1000) {
        snprintf(buf, bufsz, "%.2fK", (double)n / 1e3);
    } else {
        snprintf(buf, bufsz, "%" PRIu64, n);
    }
    return buf;
}

int
axe_print_summary(const struct axe_config *config)
{
    const char *tmp;
    char processed[32];
    char demuxed[32];
    char failed[32];
    double rate = 0.0;
    double pct_failed = 0.0;

    if (!axe_config_ok(config)) {
        return -1;
    }
    if (config->verbosity < 0) {
        /* Say nothing if we're being quiet */
        return 0;
    }
    if (config->time_taken > 0) {
        rate = (double)config->reads_processed / 1000.0 / config->time_taken;
    }
    if (config->reads_processed > 0) {
        pct_failed = 100.0 * (double)config->reads_failed /
                     (double)config->reads_processed;
    }
    axe_message_bold(config->logger, "\nRun Summary:\n");
    if (config->verbosity > 1) {
        qes_log_message_debug(config->logger,
                              "Being verbose (not that you'll notice)\n");
    }
    tmp = config->out_mode == READS_SINGLE ? "reads" : "read pairs";
    axe_format_bold(config->logger,
            "Processed %s %s in %0.1f seconds (%0.1fK %s/sec)\n",
            axe_human_count(config->reads_processed, processed,
                            sizeof(processed)),
            tmp, config->time_taken, rate, tmp);
    axe_format_bold(config->logger, "%s %s contained valid barcodes\n",
            axe_human_count(config->reads_demultiplexed, demuxed,
                            sizeof(demuxed)), tmp);
    axe_format_bold(config->logger,
            "%s %s could not be demultiplexed (%0.1f%%)\n",
            axe_human_count(config->reads_failed, failed, sizeof(failed)),
            tmp, pct_failed);
    return 0;
}
