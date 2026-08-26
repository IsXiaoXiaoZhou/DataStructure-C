/**
 * @file main.c
 * @brief 稳定计数排序断言测试
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "counting_sort.h"

static void stable_oracle(int a[], size_t n)
{
    size_t i = 0;
    size_t j = 0;
    for (i = 1; i < n; ++i) {
        int key = a[i];
        j = i;
        while (j > 0 && key < a[j - 1]) {
            a[j] = a[j - 1];
            --j;
        }
        a[j] = key;
    }
}

static int is_asc(const int a[], size_t n)
{
    size_t i = 0;
    for (i = 1; i < n; ++i) {
        if (a[i - 1] > a[i]) return 0;
    }
    return 1;
}

int main(void)
{
    enum { N = 800, KEYMAX = 15 };
    int a[N];
    int expected[N];
    size_t i = 0;
    int rounds = 0;

    printf("===== 计数排序 counting_sort =====\n");

    assert(counting_sort(NULL, 3, 10).code == DS_NULL_PTR);
    assert(counting_sort(a, 0, 10).code == DS_OK);
    a[0] = 5;
    assert(counting_sort(a, 1, 10).code == DS_OK && a[0] == 5);
    a[0] = -1;
    assert(counting_sort(a, 1, 10).code == DS_OUT_OF_RANGE);

    {
        static const int seq[] = { 5, 2, 8, 2, 9, 1, 5, 2, 0, 7 };
        for (i = 0; i < sizeof(seq) / sizeof(seq[0]); ++i) {
            a[i] = seq[i];
            expected[i] = seq[i];
        }
        assert(counting_sort(a, sizeof(seq) / sizeof(seq[0]), 9).code == DS_OK);
        stable_oracle(expected, sizeof(seq) / sizeof(seq[0]));
        for (i = 0; i < sizeof(seq) / sizeof(seq[0]); ++i) {
            assert(a[i] == expected[i]);
        }
    }

    a[0] = 100;
    assert(counting_sort(a, 1, 9).code == DS_OUT_OF_RANGE);
    {
        int tmp[3] = { 1, -2, 3 };
        assert(counting_sort(tmp, 3, 9).code == DS_OUT_OF_RANGE);
    }

    srand(20260825u);
    for (rounds = 0; rounds < 2000; ++rounds) {
        size_t n = (size_t)(rand() % N) + 1;
        unsigned maxk = (unsigned)(rand() % KEYMAX) + 1;
        for (i = 0; i < n; ++i) {
            a[i] = rand() % (int)maxk;
            expected[i] = a[i];
        }
        assert(counting_sort(a, n, maxk).code == DS_OK);
        assert(is_asc(a, n));
        stable_oracle(expected, n);
        for (i = 0; i < n; ++i) {
            assert(a[i] == expected[i]);
        }
    }

    printf("全部测试通过\n");
    return 0;
}
