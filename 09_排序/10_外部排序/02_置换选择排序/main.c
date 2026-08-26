/**
 * @file main.c
 * @brief 置换选择排序断言测试
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "replacement_selection.h"

#define N 2000
#define WORKMAX 2500

static int cmp_int(const void *x, const void *y)
{
    int a = *(const int *)x;
    int b = *(const int *)y;
    return (a > b) - (a < b);
}

static void write_in(const char *path, const int in[], size_t n)
{
    FILE *f = fopen(path, "w");
    size_t i = 0;
    assert(f != NULL);
    for (i = 0; i < n; ++i) fprintf(f, "%d\n", in[i]);
    fclose(f);
}

static void run_round(const char *inpath, const char *outpath,
                      int in[], size_t n, size_t m, int work[], size_t workcap)
{
    FILE *fi = NULL;
    FILE *fo = NULL;
    FILE *fr = NULL;
    RunReport rep;
    int *merged = NULL;
    size_t i = 0, cnt = 0, runs = 0;

    write_in(inpath, in, n);

    fi = fopen(inpath, "r");
    fo = fopen(outpath, "w");
    assert(fi != NULL && fo != NULL);
    assert(workcap >= m);
    assert(replacement_selection(fi, fo, work, m, &rep).code == DS_OK);
    fclose(fi);
    fclose(fo);

    merged = malloc(n * sizeof *merged);
    assert(merged != NULL);

    fr = fopen(outpath, "r");
    assert(fr != NULL);
    for (;;) {
        int rn = 0, prev = 0;
        if (fscanf(fr, "%d", &rn) != 1) break;
        assert(rn >= 0);
        for (i = 0; i < (size_t)rn; ++i) {
            int v = 0;
            assert(fscanf(fr, "%d", &v) == 1);
            if (i > 0) assert(prev <= v);
            prev = v;
            merged[cnt++] = v;
        }
        ++runs;
    }
    fclose(fr);

    assert(cnt == rep.total);
    assert(runs == rep.runs);
    assert(cnt == n);

    qsort(in, n, sizeof *in, cmp_int);
    qsort(merged, cnt, sizeof *merged, cmp_int);
    for (i = 0; i < cnt; ++i) {
        assert(in[i] == merged[i]);
    }

    assert(runs >= 1);
    assert(runs <= (n + m - 1) / m);

    free(merged);
}

int main(void)
{
    static int was[N];
    static int sorted_in[N];
    int work[WORKMAX];
    const char *inpath = "rsel_in.txt";
    const char *outpath = "rsel_out.txt";
    int rounds = 0;

    printf("===== 置换选择排序 replacement_selection =====\n");

    {
        RunReport rep;
        int w[4];
        FILE *fi = fopen(inpath, "w");
        assert(fi != NULL);
        fprintf(fi, "1\n2\n");
        fclose(fi);
        fi = fopen(inpath, "r");
        assert(fi != NULL);
        FILE *fo = fopen(outpath, "w");
        assert(fo != NULL);
        assert(replacement_selection(NULL, NULL, w, 3, &rep).code == DS_NULL_PTR);
        assert(replacement_selection(fi, NULL, w, 3, &rep).code == DS_NULL_PTR);
        assert(replacement_selection(fi, fo, NULL, 3, &rep).code == DS_NULL_PTR);
        assert(replacement_selection(NULL, fo, w, 0, &rep).code == DS_NULL_PTR);
        assert(replacement_selection(fi, fo, w, 0, &rep).code == DS_OUT_OF_RANGE);
        fclose(fi);
        fclose(fo);
    }

    srand(20260825u);
    for (rounds = 0; rounds < 36; ++rounds) {
        size_t mtab[3] = { 3, 10, 50 };
        size_t m = mtab[rounds % 3];
        size_t i = 0;
        for (i = 0; i < N; ++i) {
            was[i] = rand() % 1000000;
        }
        memcpy(sorted_in, was, sizeof sorted_in);
        run_round(inpath, outpath, sorted_in, N, m, work, (size_t)WORKMAX);
    }

    {
        size_t i = 0;
        for (i = 0; i < N; ++i) sorted_in[i] = (int)i * 3 + 1;
        memcpy(was, sorted_in, sizeof was);
        run_round(inpath, outpath, was, N, 10, work, (size_t)WORKMAX);
    }

    {
        size_t i = 0;
        for (i = 0; i < N; ++i) {
            was[i] = rand() % 1000;
            sorted_in[i] = was[i];
        }
        qsort(sorted_in, N, sizeof *sorted_in, cmp_int);
        run_round(inpath, outpath, was, N, (size_t)WORKMAX - 10, work, (size_t)WORKMAX);
    }

    {
        size_t i = 0;
        for (i = 0; i < N; ++i) {
            was[i] = rand() % 1000000;
            sorted_in[i] = was[i];
        }
        run_round(inpath, outpath, sorted_in, N, 1, work, (size_t)WORKMAX);
    }

    remove(inpath);
    remove(outpath);

    printf("全部测试通过\n");
    return 0;
}
