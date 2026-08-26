/**
 * @file main.c
 * @brief LSD 基数排序断言测试
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "radix_sort.h"

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

static unsigned test_bases[] = { 2u, 10u, 16u, 100u };

int main(void)
{
    enum { N = 800, VALMAX = 1000 };
    int a[N];
    int b[N];
    int expected[N];
    size_t i = 0;
    int rounds = 0;
    size_t bt = 0;

    printf("===== 基数排序 radix_sort (LSD) =====\n");

    assert(radix_sort(NULL, 3, 10).code == DS_NULL_PTR);
    assert(radix_sort(a, 0, 10).code == DS_OK);
    a[0] = 7;
    assert(radix_sort(a, 1, 10).code == DS_OK && a[0] == 7);
    assert(radix_sort(a, 1, 1).code == DS_OUT_OF_RANGE);
    a[0] = -3;
    assert(radix_sort(a, 1, 10).code == DS_OUT_OF_RANGE);

    {
        static const int seq[] = { 13, 245, 9, 73, 821, 45, 520, 6, 37, 99 };
        for (i = 0; i < sizeof(seq) / sizeof(seq[0]); ++i) {
            a[i] = seq[i];
            expected[i] = seq[i];
        }
        assert(radix_sort(a, sizeof(seq) / sizeof(seq[0]), 10).code == DS_OK);
        stable_oracle(expected, sizeof(seq) / sizeof(seq[0]));
        for (i = 0; i < sizeof(seq) / sizeof(seq[0]); ++i) {
            assert(a[i] == expected[i]);
        }
        assert(is_asc(a, sizeof(seq) / sizeof(seq[0])));
    }

    srand(20260825u);
    for (rounds = 0; rounds < 2000; ++rounds) {
        size_t n = (size_t)(rand() % N) + 1;
        for (i = 0; i < n; ++i) {
            int v = rand() % VALMAX;
            a[i] = v;
            expected[i] = v;
        }
        bt = (size_t)((unsigned)rand() % (unsigned)(sizeof(test_bases) / sizeof(test_bases[0])));
        assert(radix_sort(a, n, test_bases[bt]).code == DS_OK);
        assert(is_asc(a, n));
        stable_oracle(expected, n);
        for (i = 0; i < n; ++i) {
            assert(a[i] == expected[i]);
        }
    }

    for (rounds = 0; rounds < 200; ++rounds) {
        size_t n = (size_t)(rand() % N) + 1;
        for (i = 0; i < n; ++i) {
            a[i] = rand() % VALMAX;
            b[i] = a[i];
        }
        assert(radix_sort(a, n, 10).code == DS_OK);
        assert(radix_sort(b, n, 2).code == DS_OK);
        for (i = 0; i < n; ++i) {
            assert(a[i] == b[i]);
        }
    }

    printf("全部测试通过\n");
    return 0;
}
