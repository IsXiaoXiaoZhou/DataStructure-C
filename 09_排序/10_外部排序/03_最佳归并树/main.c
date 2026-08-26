/**
 * @file main.c
 * @brief k 叉最佳归并树断言测试
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "optimal_merge_tree.h"

static long long ref_wpl(const int len[], size_t m, unsigned k)
{
    long long a[4096];
    long long b[4096];
    size_t cnt = 0;
    size_t i = 0;
    size_t d = 0;
    long long wpl = 0;

    assert(opt_merge_dummy_needed(m, k, &d).code == DS_OK);
    assert(m + d <= 4096u);
    for (i = 0; i < m; ++i) a[cnt++] = len[i];
    for (i = 0; i < d; ++i) a[cnt++] = 0LL;

    while (cnt > 1) {
        long long sum = 0;
        size_t si = 0;
        size_t t = 0;
        size_t j = 0;
        for (t = 0; t < (size_t)k; ++t) {
            size_t p = 0;
            unsigned first = 1;
            size_t q = 0;
            for (q = 0; q < cnt; ++q) {
                if (first || a[q] < a[p]) { p = q; first = 0; }
            }
            sum += a[p];
            a[p] = 0x7FFFFFFF0LL;
        }
        wpl += sum;
        for (j = 0; j < cnt; ++j) {
            if (a[j] != 0x7FFFFFFF0LL) b[si++] = a[j];
        }
        b[si++] = sum;
        memcpy(a, b, si * sizeof *a);
        cnt = si;
    }
    return wpl;
}

int main(void)
{
    printf("===== k 叉最佳归并树 optimal_merge_tree =====\n");

    {
        size_t d = 999;
        assert(opt_merge_dummy_needed(0, 3, &d).code == DS_OK && d == 0);
        assert(opt_merge_dummy_needed(1, 3, &d).code == DS_OK && d == 0);
        assert(opt_merge_dummy_needed(2, 3, &d).code == DS_OK && d == 1);
        assert(opt_merge_dummy_needed(3, 3, &d).code == DS_OK && d == 0);
        assert(opt_merge_dummy_needed(4, 3, &d).code == DS_OK && d == 1);
        assert(opt_merge_dummy_needed(6, 3, &d).code == DS_OK && d == 1);
        assert(opt_merge_dummy_needed(6, 2, &d).code == DS_OK && d == 0);
        assert(opt_merge_dummy_needed(6, 4, &d).code == DS_OK && d == 1);
        assert(opt_merge_dummy_needed(5, 3, NULL).code == DS_NULL_PTR);
        assert(opt_merge_dummy_needed(5, 1, &d).code == DS_OUT_OF_RANGE);
        assert(opt_merge_dummy_needed(5, 0, &d).code == DS_OUT_OF_RANGE);
    }

    {
        long long wpl = 0;
        const int l1[] = { 40, 32, 20, 18, 10, 8, 5, 3 };
        assert(opt_merge_wpl(l1, 8, 3, &wpl).code == DS_OK);
        assert(wpl == 234);
        assert(ref_wpl(l1, 8, 3) == 234);

        const int l2[] = { 5, 7, 9 };
        assert(opt_merge_wpl(l2, 3, 2, &wpl).code == DS_OK);
        assert(wpl == 33);
        assert(ref_wpl(l2, 3, 2) == 33);
    }

    {
        long long wpl = -1;
        const int l1[] = { 7 };
        const int l2[] = { 2, 3 };
        assert(opt_merge_wpl(NULL, 3, 2, &wpl).code == DS_NULL_PTR);
        assert(opt_merge_wpl(l1, 1, 3, &wpl).code == DS_OK);
        assert(wpl == 0);
        assert(opt_merge_wpl(l1, 0, 3, &wpl).code == DS_OK && wpl == 0);
        assert(opt_merge_wpl(l2, 2, 1, &wpl).code == DS_OUT_OF_RANGE);
        assert(opt_merge_wpl(l2, 2, 0, &wpl).code == DS_OUT_OF_RANGE);
    }

    srand(20260825u);
    {
        int rounds = 0;
        for (rounds = 0; rounds < 2000; ++rounds) {
            int len[20];
            size_t m = (size_t)(rand() % 18) + 1;
            unsigned k = (unsigned)(rand() % 5) + 2;
            size_t i = 0;
            long long w1 = 0, w2 = 0;
            for (i = 0; i < m; ++i) len[i] = (rand() % 200) + 1;
            assert(opt_merge_wpl(len, m, k, &w1).code == DS_OK);
            w2 = ref_wpl(len, m, k);
            assert(w1 == w2);
        }
    }

    printf("全部测试通过\n");
    return 0;
}
