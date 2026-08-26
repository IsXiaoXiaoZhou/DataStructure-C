/**
 * @file main.c
 * @brief 败者树 k 路归并断言测试
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loser_tree.h"

typedef struct {
    int   *p;
    size_t n;
    size_t i;
} RG;

static int cmp_int(const void *x, const void *y)
{
    int a = *(const int *)x;
    int b = *(const int *)y;
    return (a > b) - (a < b);
}

static int is_asc(const int a[], size_t n)
{
    size_t i = 0;
    for (i = 1; i < n; ++i) {
        if (a[i - 1] > a[i]) return 0;
    }
    return 1;
}

static void do_merge(const RG *runs, size_t k, LoserTree *lt,
                     int *tree_buf, int *seg, int *out, size_t *outn)
{
    size_t i = 0;
    size_t used = 0;

    assert(lt_init(lt, tree_buf, seg, k).code == DS_OK);
    for (i = 0; i < k; ++i) {
        seg[i] = (runs[i].n > 0) ? runs[i].p[0] : INT_MAX;
    }
    assert(lt_rebuild(lt).code == DS_OK);

    for (;;) {
        size_t widx = 0;
        int wval = 0;
        assert(lt_winner(lt, &widx, &wval).code == DS_OK);
        if (wval == INT_MAX) break;
        out[used++] = wval;
        assert(runs[widx].i < runs[widx].n);
        ++(((RG *)runs)[widx].i);
        if (((RG *)runs)[widx].i < runs[widx].n) {
            seg[widx] = runs[widx].p[((RG *)runs)[widx].i];
        } else {
            assert(lt_exhaust(lt, widx).code == DS_OK);
            continue;
        }
        assert(lt_adjust(lt, widx).code == DS_OK);
    }
    *outn = used;
}

static void run_fixed_case(const int *vals[], const size_t lens[], size_t k)
{
    RG runs[16];
    size_t size = 0;
    size_t total = 0;
    size_t i = 0;
    size_t outn = 0;
    int *tree_buf = NULL;
    int *seg = NULL;
    int *out = NULL;
    int *all = NULL;
    LoserTree lt;
    size_t q = 0;

    assert(k <= 16u && k >= 1u);
    size = lt_size_needed(k);
    for (i = 0; i < k; ++i) {
        runs[i].p = (int *)vals[i];
        runs[i].n = lens[i];
        runs[i].i = 0;
        total += lens[i];
    }
    tree_buf = malloc(size * 2u * sizeof *tree_buf);
    seg      = malloc(size * sizeof *seg);
    out      = malloc((total + (k ? 1u : 0u)) * sizeof *out);
    all      = malloc(total * sizeof *all);
    assert(tree_buf && seg && out && all);

    q = 0;
    for (i = 0; i < k; ++i) {
        memcpy(&all[q], runs[i].p, runs[i].n * sizeof *all);
        q += runs[i].n;
    }

    do_merge(runs, k, &lt, tree_buf, seg, out, &outn);

    assert(outn == total);
    assert(is_asc(out, outn));
    qsort(all, total, sizeof *all, cmp_int);
    for (i = 0; i < total; ++i) {
        assert(out[i] == all[i]);
    }

    free(tree_buf);
    free(seg);
    free(out);
    free(all);
}

int main(void)
{
    {
        LoserTree lt;
        int seg[8];
        int tb[16];
        size_t idx = 0;
        int v = 0;
        assert(lt_size_needed(1) == 1);
        assert(lt_size_needed(5) == 8);
        assert(lt_init(&lt, tb, seg, 0).code == DS_OUT_OF_RANGE);
        assert(lt_init(NULL, tb, seg, 2).code == DS_NULL_PTR);
        assert(lt_init(&lt, NULL, seg, 2).code == DS_NULL_PTR);
        assert(lt_init(&lt, tb, NULL, 2).code == DS_NULL_PTR);
        assert(lt_winner(NULL, &idx, &v).code == DS_NULL_PTR);
    }

    {
        static const int a1[] = { 3, 7, 8, 12, 20 };
        const size_t l1 = sizeof(a1) / sizeof(a1[0]);
        const int *vals[] = { a1 };
        const size_t lens[] = { l1 };
        run_fixed_case(vals, lens, 1);
    }

    {
        static const size_t ks[] = { 2, 3, 4, 5, 7 };
        size_t ki = 0;
        for (ki = 0; ki < sizeof(ks) / sizeof(ks[0]); ++ki) {
            size_t k = ks[ki];
            const int *vals[9];
            size_t lens[9];
            size_t i = 0;
            for (i = 0; i < k; ++i) {
                static int buf[9][32];
                size_t j = 0;
                size_t len = 3 + (i * 5) % 12;
                for (j = 0; j < len; ++j) {
                    buf[i][j] = (int)(i * 100 + j * 7 + (ki * 13));
                }
                lens[i] = len;
                vals[i] = buf[i];
            }
            run_fixed_case(vals, lens, k);
        }
    }

    srand(20260825u);
    {
        int rounds = 0;
        for (rounds = 0; rounds < 800; ++rounds) {
            size_t k = (size_t)(rand() % 9) + 1;
            const int *vals[9];
            size_t lens[9];
            size_t i = 0;
            for (i = 0; i < k; ++i) {
                static int dbuf[9][40];
                size_t len = (size_t)(rand() % 13);
                size_t j = 0;
                for (j = 0; j < len; ++j) {
                    dbuf[i][j] = rand() % 1000;
                }
                qsort(&dbuf[i][0], len, sizeof(int), cmp_int);
                vals[i] = dbuf[i];
                lens[i] = len;
            }
            run_fixed_case(vals, lens, k);
        }
    }

    printf("全部测试通过\n");
    return 0;
}
