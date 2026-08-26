/**
 * @file main.c
 * @brief 快速排序断言测试与随机对拍
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "quick_sort.h"

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
    enum { N = 500 };
    int a[N];
    int expected[N];
    size_t i = 0;
    int rounds = 0;

    printf("===== 快速排序 quick_sort =====\n");

    assert(quick_sort(NULL, 3).code == DS_NULL_PTR);
    assert(quick_sort(a, 0).code == DS_OK);
    a[0] = 7;
    assert(quick_sort(a, 1).code == DS_OK && a[0] == 7);

    a[0] = 5; a[1] = 2; a[2] = 8; a[3] = 2; a[4] = 9; a[5] = 1;
    assert(quick_sort(a, 6).code == DS_OK);
    assert(is_asc(a, 6));
    assert(a[0] == 1 && a[5] == 9);

    srand(20260825u);
    for (rounds = 0; rounds < 2000; ++rounds) {
        size_t n = (size_t)(rand() % N) + 1;
        for (i = 0; i < n; ++i) {
            a[i] = rand() % 1000;
            expected[i] = a[i];
        }
        assert(quick_sort(a, n).code == DS_OK);
        qsort(expected, n, sizeof *expected, cmp_int);
        for (i = 0; i < n; ++i) {
            assert(a[i] == expected[i]);
        }
        assert(is_asc(a, n));
    }

    printf("全部测试通过\n");
    return 0;
}
