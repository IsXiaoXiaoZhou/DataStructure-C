/**
 * @file optimal_merge_tree.c
 * @brief k 叉最佳归并树实现（哈夫曼思想 + 补虚段）
 */

#include <stdlib.h>

#include "optimal_merge_tree.h"


/* ---------- long long 最小堆 ---------- */
typedef struct {
    long long *a;
    size_t     n;
    size_t     cap;
} Lp;

static DsStatus lp_push_one(Lp *h, long long v)
{
    size_t i = 0;
    if (h->n == h->cap) {
        size_t nc = (h->cap == 0) ? 16u : h->cap * 2u;
        long long *p = malloc(nc * sizeof *p);
        size_t j = 0;
        if (p == NULL) return DS_OVERFLOW;
        for (j = 0; j < h->n; ++j) p[j] = h->a[j];
        free(h->a);
        h->a = p;
        h->cap = nc;
    }
    h->a[h->n] = v;
    i = h->n;
    ++h->n;
    while (i > 0) {
        size_t par = (i - 1) / 2;
        if (h->a[par] <= h->a[i]) break;
        { long long t = h->a[par]; h->a[par] = h->a[i]; h->a[i] = t; }
        i = par;
    }
    return DS_OK;
}

static long long lp_pop_min(Lp *h)
{
    long long top = h->a[0];
    size_t n = --h->n;
    size_t i = 0;
    if (n > 0) h->a[0] = h->a[n];
    for (;;) {
        size_t l = 2 * i + 1;
        if (l >= n) break;
        size_t r = l + 1;
        size_t j = (r < n && h->a[r] < h->a[l]) ? r : l;
        if (h->a[j] >= h->a[i]) break;
        { long long t = h->a[i]; h->a[i] = h->a[j]; h->a[j] = t; }
        i = j;
    }
    return top;
}

DsResult opt_merge_dummy_needed(size_t m, unsigned k, size_t *count)
{
    if (count == NULL) {
        return (DsResult){DS_NULL_PTR, "出参指针为空"};
    }
    if (k < 2) {
        return (DsResult){DS_OUT_OF_RANGE, "归并路数须 >= 2"};
    }
    if (m == 0) {
        *count = 0;
        return (DsResult){DS_OK, "无需补虚段"};
    }
    *count = (size_t)((k - 1u) - ((unsigned)(((size_t)(m - 1)) % (size_t)(k - 1u)))) % (size_t)(k - 1u);
    return (DsResult){DS_OK, "补虚段计算完成"};
}

DsResult opt_merge_wpl(const int len[], size_t m, unsigned k, long long *wpl)
{
    Lp heap = { NULL, 0, 0 };
    size_t d = 0;
    size_t i = 0;
    long long sum = 0;

    if (len == NULL || wpl == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (k < 2) {
        return (DsResult){DS_OUT_OF_RANGE, "归并路数须 >= 2"};
    }
    if (m == 0) {
        *wpl = 0;
        return (DsResult){DS_OK, "空段WPL为0"};
    }
    if (opt_merge_dummy_needed(m, k, &d).code != DS_OK) {
        return (DsResult){DS_ERROR, "补虚段计算失败"};
    }

    for (i = 0; i < m; ++i) {
        if (lp_push_one(&heap, (long long)len[i]) != DS_OK) {
            free(heap.a);
            return (DsResult){DS_OVERFLOW, "堆内存分配失败"};
        }
    }
    for (i = 0; i < d; ++i) {
        if (lp_push_one(&heap, 0LL) != DS_OK) {
            free(heap.a);
            return (DsResult){DS_OVERFLOW, "堆内存分配失败"};
        }
    }

    while (heap.n > 1) {
        long long v = 0;
        for (i = 0; i < (size_t)k; ++i) {
            v += lp_pop_min(&heap);
        }
        sum += v;
        if (lp_push_one(&heap, v) != DS_OK) {
            free(heap.a);
            return (DsResult){DS_OVERFLOW, "堆内存分配失败"};
        }
    }

    free(heap.a);
    *wpl = sum;
    return (DsResult){DS_OK, "WPL计算成功"};
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "位置/下标越界";
        case DS_OVERFLOW:    return "堆内存分配失败";
        case DS_EMPTY:       return "对空结构取元素";
        case DS_NOT_FOUND:   return "查找未命中";
        default:             return "未知状态码";
    }
}
