/*
 * ============================================================================
 *
 *       Filename:  test_comb.c
 *
 *    Description:  blah
 *
 *        Version:  1.0
 *        Created:  13/06/14 14:01:02
 *       Revision:  none
 *        License:  GPLv3+
 *       Compiler:  gcc, clang
 *
 *         Author:  Kevin Murray, spam@kdmurray.id.au
 *
 * ============================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

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
                /* fill forwards the incrementing. */
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
    size_t jjj;
    uint64_t *mut_indicies;

    if (str == NULL || len < 1 || dist < 1) {
        return NULL;
    }

    result = malloc(results_alloced * sizeof(*result));
    mut_indicies = calloc(dist, sizeof(*mut_indicies));
    while (combinations(len, dist, mut_indicies) != 0) {
        for (iii = 0; iii < dist; iii++) {
            for (jjj = 0; jjj < n_letters; jjj++) {
                tmp = strndup(str, len);
                tmp[mut_indicies[iii]] = alphabet[jjj];
                printf("% 4zu\t%s\n", results, tmp);
                if (results + 1 > results_alloced) {
                    results_alloced = (results_alloced << 1);
                    result = realloc(result,
                            results_alloced * sizeof(*result));
                }
                result[results++] = tmp;
            }
        }
    }
}

int
main (int argc, char *argv[])
{
    uint64_t n;
    uint64_t r;
    uint64_t *choices;
    int call_no = 0;

    if (argc != 3) {
        fprintf(stderr, "usage: test_comb n r\n");
        exit(EXIT_FAILURE);
    }

    n = atoll(argv[1]);
    r = atoll(argv[2]);
    choices = calloc(r, sizeof(*choices));

    while (combinations(n, r, choices) != 0) {
        size_t iii;
        printf("c % 3d:\t", ++call_no);
        for (iii = 0; iii < r; iii++) {
            printf("%llu ", choices[iii]);
        }
        printf("\n");
    }
    size_t a = 0;
    //hamming_mutate_dna(&a, "ACTCTA", 6, 1, 0);
}

#if 0
#include <stdio.h>
uintptr_t *
combinations64_(uint64_t pool, uint64_t choices, uint64_t need, uint64_t iter,
        uint64_t at)
{
    if (pool > 64 || pool < 1 || pool < choices) {
        return NULL;
    }
    if (need == 0) {
        size_t iii;
        uintptr_t *retval = calloc(choices, 
        for (iii = 0; iii < pool; iii++) {

        }
    }

}
#define combinations64(pool, choices) \
    combinations64_(pool, choices, choices, 0, 0, 0)
;

void comb(int m, int n, unsigned char *c)
{
	int i;
	for (i = 0; i < n; i++) c[i] = n - i;
 
	while (1) {
		for (i = n; i--;)
			printf("%d%c", c[i], i ? ' ': '\n');
 
		/* this check is not strictly necessary, but if m is not close to n,
		   it makes the whole thing quite a bit faster */
		if (c[i]++ < m) continue;
 
		for (i = 0; c[i] >= m - i;) if (++i >= n) return;
		for (c[i]++; i; i--) c[i-1] = c[i] + 1;
	}
}
/* Type marker stick: using bits to indicate what's chosen.  The stick can't
 * handle more than 32 items, but the idea is there; at worst, use array instead */
typedef unsigned long marker;
marker one = 1;
 
void comb(int pool, int need, marker chosen, int at)
{
	if (pool < need + at) return; /* not enough bits left */
 
	if (!need) {
		/* got all we needed; print the thing.  if other actions are
		 * desired, we could have passed in a callback function. */
		for (at = 0; at < pool; at++)
			if (chosen & (one << at)) printf("%d ", at);
		printf("\n");
		return;
	}
	/* if we choose the current item, "or" (|) the bit to mark it so. */
	comb(pool, need - 1, chosen | (one << at), at + 1);
	comb(pool, need, chosen, at + 1);  /* or don't choose it, go to next */
}
 
int main()
{
	comb(5, 3, 0, 0);
	return 0;
}
#endif
