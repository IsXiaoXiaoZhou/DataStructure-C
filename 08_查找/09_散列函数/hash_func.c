/**
 * @file hash_func.c
 * @brief 五种散列函数实现 + 分布均匀性统计
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "hash_func.h"


/* ========== 内部工具 ========== */

static int is_prime(int n)
{
    int i = 0;
    if (n < 2) return 0;
    if (n < 4) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    for (i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return 0;
    }
    return 1;
}

static int prev_prime(int n)
{
    while (n > 2 && !is_prime(n)) --n;
    return n;
}

static int digit_at(int key, int d)
{
    int i = 0;
    for (i = 0; i < d; ++i) key /= 10;
    return key % 10;
}

/* ========== 五种散列函数 ========== */

int hf_direct(int key, int m)
{
    int h = key % m;
    if (h < 0) h += m;
    return h;
}

int hf_division(int key, int m)
{
    int p = prev_prime(m);
    int h = key % p;
    if (h < 0) h += p;
    return h;
}

int hf_digit_analysis(int key, int m)
{
    int d1 = digit_at(key, 1);
    int d2 = digit_at(key, 2);
    int h = (d1 * 10 + d2) % m;
    if (h < 0) h += m;
    return h;
}

int hf_mid_square(int key, int m)
{
    long long sq = (long long)key * (long long)key;
    int nd = 0;
    int mid = 0;
    int i = 0;
    int take = 0;

    if (sq < 0) sq = -sq;

    {
        long long tmp = sq;
        nd = 0;
        if (tmp == 0) nd = 1;
        while (tmp > 0) { ++nd; tmp /= 10; }
    }

    take = nd < 3 ? nd : 3;
    {
        int skip = (nd - take) / 2;
        for (i = 0; i < skip; ++i) sq /= 10;
    }
    mid = (int)(sq % 1000);
    return mid % m;
}

int hf_folding(int key, int m)
{
    int sum = 0;
    if (key < 0) key = -key;
    while (key > 0) {
        sum += key % 1000;
        key /= 1000;
    }
    return sum % m;
}

/* ========== 分布均匀性统计 ========== */

DsResult hf_distribution(HashFunc hf, const int keys[], size_t n,
                         int m, int buckets[], double *chi2)
{
    size_t i = 0;
    double expected = 0.0;
    double chi2_val = 0.0;
    int *buf = NULL;
    int need_free = 0;

    if (hf == NULL || keys == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (n == 0 || m <= 0) {
        return (DsResult){DS_EMPTY, "关键字数或表长为0"};
    }

    if (buckets == NULL) {
        buf = (int *)malloc((size_t)m * sizeof(int));
        if (buf == NULL) return (DsResult){DS_OVERFLOW, "内存分配失败"};
        need_free = 1;
        buckets = buf;
    }

    memset(buckets, 0, (size_t)m * sizeof(int));

    for (i = 0; i < n; ++i) {
        int h = hf(keys[i], m);
        if (h >= 0 && h < m) {
            buckets[h]++;
        }
    }

    expected = (double)n / (double)m;
    if (expected > 0.0) {
        int j = 0;
        for (j = 0; j < m; ++j) {
            double diff = (double)buckets[j] - expected;
            chi2_val += diff * diff / expected;
        }
    }

    if (chi2 != NULL) {
        *chi2 = chi2_val;
    }
    if (need_free) {
        free(buf);
    }
    return (DsResult){DS_OK, "分布统计完成"};
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "位置/下标越界";
        case DS_OVERFLOW:    return "内存分配失败";
        case DS_EMPTY:       return "参数为空";
        default:             return "未知状态码";
    }
}
