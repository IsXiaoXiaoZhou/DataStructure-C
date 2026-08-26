/**
 * @file main.c
 * @brief 归并排序（递归/迭代两版）断言测试
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "merge_sort.h"

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
        if (a[i - 1] > a[i]) {
            return 0;
        }
    }
    return 1;
}

int main(void)
{
    enum { N = 600 };
    int a[N];
    int b[N];
    int expected[N];
    size_t i = 0;
    int rounds = 0;

    printf("===== 归并排序 merge_sort / merge_sort_iter =====\n");

    assert(merge_sort(NULL, 3).code == DS_NULL_PTR);
    assert(merge_sort_iter(NULL, 3).code == DS_NULL_PTR);
    assert(merge_sort(a, 0).code == DS_OK);
    assert(merge_sort_iter(a, 0).code == DS_OK);
    a[0] = -4;
    assert(merge_sort(a, 1).code == DS_OK && a[0] == -4);

    {
        static const int seq[] = { 5, 3, 8, 3, 9, 1, 5, 2 };
        for (i = 0; i < sizeof(seq) / sizeof(seq[0]); ++i) {
            a[i] = seq[i];
            b[i] = seq[i];
        }
        assert(merge_sort(a, sizeof(seq) / sizeof(seq[0])).code == DS_OK);
        assert(merge_sort_iter(b, sizeof(seq) / sizeof(seq[0])).code == DS_OK);
        assert(is_asc(a, sizeof(seq) / sizeof(seq[0])));
        assert(is_asc(b, sizeof(seq) / sizeof(seq[0])));
        for (i = 0; i < sizeof(seq) / sizeof(seq[0]); ++i) {
            assert(a[i] == b[i]);
        }
    }

    srand(20260825u);
    for (rounds = 0; rounds < 2000; ++rounds) {
        size_t n = (size_t)(rand() % N) + 1;
        for (i = 0; i < n; ++i) {
            int v = rand() % 1000;
            a[i] = v;
            b[i] = v;
            expected[i] = v;
        }
        assert(merge_sort(a, n).code == DS_OK);
        assert(merge_sort_iter(b, n).code == DS_OK);
        qsort(expected, n, sizeof *expected, cmp_int);
        for (i = 0; i < n; ++i) {
            assert(a[i] == expected[i]);
            assert(b[i] == expected[i]);
        }
    }

    printf("全部测试通过\n");
    return 0;
}
