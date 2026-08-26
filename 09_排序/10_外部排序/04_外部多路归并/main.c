/**
 * @file main.c
 * @brief 外部 k 路归并断言测试
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "external_merge.h"

#define MAXK 12
#define FILEMAX (MAXK + 1)

static char *in_name(char *buf, size_t bufsz, int idx)
{
    snprintf(buf, bufsz, "em_in%d.txt", idx);
    return buf;
}

static int cmp_int(const void *x, const void *y)
{
    int a = *(const int *)x;
    int b = *(const int *)y;
    return (a > b) - (a < b);
}

static size_t run_case(const size_t lens[], size_t k)
{
    int *all = NULL;
    int *got = NULL;
    size_t total = 0;
    size_t i = 0;
    FILE *in[MAXK];
    FILE *out = NULL;
    char path[32];
    size_t gcnt = 0;

    for (i = 0; i < k; ++i) total += lens[i];
    all = malloc((total > 0 ? total : 1u) * sizeof *all);
    got = malloc((total > 0 ? total : 1u) * sizeof *got);
    assert(all != NULL && got != NULL);

    {
        size_t acc = 0;
        for (i = 0; i < k; ++i) {
            FILE *f = NULL;
            size_t j = 0;
            int prev = 0;
            in_name(path, sizeof path, (int)i);
            f = fopen(path, "w");
            assert(f != NULL);
            for (j = 0; j < lens[i]; ++j) {
                int v = ((rand() % 1000) + (int)(1000u * i)) * (1 + (rand() % 4)) + prev + 1;
                prev = v;
                fprintf(f, "%d\n", v);
                all[acc++] = v;
            }
            fclose(f);
        }
        assert(acc == total);
    }

    for (i = 0; i < k; ++i) {
        in_name(path, sizeof path, (int)i);
        in[i] = fopen(path, "r");
        assert(in[i] != NULL);
    }
    out = fopen("em_out.txt", "w");
    assert(out != NULL);

    assert(external_merge(in, k, out).code == DS_OK);

    for (i = 0; i < k; ++i) fclose(in[i]);
    fclose(out);

    {
        int prev = 0;
        FILE *r = fopen("em_out.txt", "r");
        assert(r != NULL);
        for (;;) {
            int v = 0;
            if (fscanf(r, "%d", &v) != 1) break;
            if (gcnt > 0) assert(prev <= v);
            prev = v;
            got[gcnt++] = v;
        }
        fclose(r);
    }
    assert(gcnt == total);

    qsort(all, total, sizeof *all, cmp_int);
    for (i = 0; i < gcnt; ++i) {
        assert(all[i] == got[i]);
    }

    free(all);
    free(got);
    return gcnt;
}

int main(void)
{
    int rounds = 0;

    printf("===== 外部多路归并 external_merge =====\n");

    {
        FILE *f = fopen("em_out.txt", "w");
        assert(f != NULL);
        assert(external_merge(NULL, 2, f).code == DS_NULL_PTR);
        assert(external_merge(NULL, 0, NULL).code == DS_NULL_PTR);
        fclose(f);
        {
            FILE *g = fopen("em_out.txt", "w");
            assert(g != NULL);
            FILE **inp = calloc(1, sizeof *inp);
            assert(inp != NULL);
            assert(external_merge(inp, 0, g).code == DS_OUT_OF_RANGE);
            free(inp);
            fclose(g);
        }
    }

    srand(20260825u);

    {
        const size_t k2[2] = { 5, 9 };
        size_t k3[3] = { 7, 2, 4 };
        size_t k6[6] = { 3, 0, 8, 5, 6, 0 };
        size_t k1[1] = { 11 };
        run_case(k2, 2);
        run_case(k3, 3);
        run_case(k6, 6);
        run_case(k1, 1);
    }

    for (rounds = 0; rounds < 120; ++rounds) {
        size_t k = (size_t)(rand() % MAXK) + 1;
        size_t lens[MAXK];
        size_t i = 0;
        for (i = 0; i < k; ++i) {
            lens[i] = (size_t)(rand() % 9);
        }
        run_case(lens, k);
    }

    {
        char path[32];
        int i = 0;
        for (i = 0; i < FILEMAX; ++i) {
            in_name(path, sizeof path, i);
            remove(path);
        }
        remove("em_out.txt");
    }

    printf("全部测试通过\n");
    return 0;
}
