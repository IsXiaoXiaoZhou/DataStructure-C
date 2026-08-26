/**
 * @file main.c
 * @brief 散列函数测试 + 分布均匀性统计演示
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hash_func.h"

enum { N_KEYS = 1000, TABLE_LEN = 97, BUCKET_CAP = 128 };

typedef int (*HF)(int, int);

static const HF funcs[] = {
    hf_direct, hf_division, hf_digit_analysis, hf_mid_square, hf_folding
};
static const char *names[] = {
    "直接定址", "除留余数", "数字分析", "平方取中", "折叠法"
};

static void print_histogram(const int buckets[], int m, const char *name, double chi2)
{
    int j = 0;
    int maxc = 0;
    printf("  %s (卡方=%.1f): ", name, chi2);
    for (j = 0; j < m; ++j) {
        if (buckets[j] > maxc) maxc = buckets[j];
    }
    for (j = 0; j < m && j < 30; ++j) {
        int bar = maxc > 0 ? buckets[j] * 10 / maxc : 0;
        printf("%d", bar);
    }
    printf("...\n");
}

int main(void)
{
    int keys[N_KEYS];
    int buckets[BUCKET_CAP];
    size_t i = 0;
    size_t f = 0;

    printf("===== 散列函数（五法集合 + 分布均匀性） =====\n");

    assert(hf_direct(0, 1) == 0);
    assert(hf_division(0, 1) == 0);

    for (i = 0; i < N_KEYS; ++i) {
        keys[i] = (int)i;
    }

    for (f = 0; f < sizeof(funcs) / sizeof(funcs[0]); ++f) {
        HF hf = funcs[f];
        double chi2 = 0.0;
        DsResult st;

        for (i = 0; i < N_KEYS; ++i) {
            int h = hf(keys[i], TABLE_LEN);
            assert(h >= 0 && h < TABLE_LEN);
        }

        st = hf_distribution(hf, keys, N_KEYS, TABLE_LEN, buckets, &chi2);
        assert(st.code == DS_OK);
        print_histogram(buckets, TABLE_LEN, names[f], chi2);
    }

    printf("\n--- 随机关键字 (srand=20260825) ---\n");
    srand(20260825u);
    for (i = 0; i < N_KEYS; ++i) {
        keys[i] = rand() % 10000;
    }

    for (f = 0; f < sizeof(funcs) / sizeof(funcs[0]); ++f) {
        HF hf = funcs[f];
        double chi2 = 0.0;

        for (i = 0; i < N_KEYS; ++i) {
            int h = hf(keys[i], TABLE_LEN);
            assert(h >= 0 && h < TABLE_LEN);
        }
        hf_distribution(hf, keys, N_KEYS, TABLE_LEN, buckets, &chi2);
        print_histogram(buckets, TABLE_LEN, names[f], chi2);
    }

    assert(hf_distribution(NULL, keys, N_KEYS, TABLE_LEN, buckets, NULL).code
           == DS_NULL_PTR);
    assert(hf_distribution(hf_direct, NULL, N_KEYS, TABLE_LEN, buckets, NULL).code
           == DS_NULL_PTR);
    assert(hf_distribution(hf_direct, keys, 0, TABLE_LEN, buckets, NULL).code
           == DS_EMPTY);

    printf("全部测试通过\n");
    return 0;
}
