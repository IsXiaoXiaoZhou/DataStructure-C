/**
 * @file replacement_selection.c
 * @brief 置换选择排序实现：用容量 m 的最小堆工作区生成"初始归并段"
 */

#include <stdlib.h>

#include "replacement_selection.h"


/* 可增长 int 数组 */
typedef struct {
    int   *a;
    size_t n;
    size_t cap;
} IAv;

/* ---------- 最小堆（0 基） ---------- */

static void heap_up(int *h, size_t i)
{
    int v = h[i];
    while (i > 0) {
        size_t p = (i - 1) / 2;
        if (h[p] <= v) break;
        h[i] = h[p];
        i = p;
    }
    h[i] = v;
}

static void heap_down(int *h, size_t n, size_t i)
{
    int v = h[i];
    for (;;) {
        size_t j = 2 * i + 1;
        if (j >= n) break;
        if (j + 1 < n && h[j + 1] < h[j]) ++j;
        if (h[j] >= v) break;
        h[i] = h[j];
        i = j;
    }
    h[i] = v;
}

static void heap_push(int *h, size_t *sz, int v)
{
    h[(*sz)++] = v;
    heap_up(h, *sz - 1);
}

static int heap_pop(int *h, size_t *sz)
{
    int top = h[0];
    --*sz;
    if (*sz > 0) {
        h[0] = h[*sz];
        heap_down(h, *sz, 0);
    }
    return top;
}

/* ---------- 可增长数组 ---------- */

static DsStatus ia_push(IAv *a, int v)
{
    if (a->n == a->cap) {
        size_t nc = (a->cap == 0) ? 8u : a->cap * 2u;
        size_t i = 0;
        int *p = malloc(nc * sizeof *p);
        if (p == NULL) return DS_OVERFLOW;
        for (i = 0; i < a->n; ++i) p[i] = a->a[i];
        free(a->a);
        a->a = p;
        a->cap = nc;
    }
    a->a[a->n++] = v;
    return DS_OK;
}

DsResult replacement_selection(FILE *in, FILE *out, int *heapbuf, size_t m, RunReport *rep)
{
    size_t hsize = 0;
    IAv deferred = { NULL, 0, 0 };
    IAv current = { NULL, 0, 0 };
    int x = 0;

    if (in == NULL || out == NULL || rep == NULL || heapbuf == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (m == 0) {
        return (DsResult){DS_OUT_OF_RANGE, "工作区容量须 >= 1"};
    }
    rep->runs = 0;
    rep->total = 0;

    while (hsize < m && fscanf(in, "%d", &x) == 1) {
        heap_push(heapbuf, &hsize, x);
    }

    while (hsize > 0 || deferred.n > 0) {
        size_t i = 0;
        int last = 0;

        current.n = 0;
        while (hsize > 0) {
            int v = heap_pop(heapbuf, &hsize);
            if (ia_push(&current, v) != DS_OK) goto fail;
            last = v;
            if (fscanf(in, "%d", &x) == 1) {
                if (x >= last) {
                    heap_push(heapbuf, &hsize, x);
                } else if (ia_push(&deferred, x) != DS_OK) {
                    goto fail;
                }
            }
        }

        if (current.n == 0) break;
        if (fprintf(out, "%d\n", (int)current.n) < 0) goto fail;
        for (i = 0; i < current.n; ++i) {
            if (fprintf(out, "%d\n", current.a[i]) < 0) goto fail;
        }
        ++rep->runs;
        rep->total += current.n;

        for (i = 0; i < deferred.n; ++i) {
            heap_push(heapbuf, &hsize, deferred.a[i]);
        }
        deferred.n = 0;
    }

    free(current.a);
    free(deferred.a);
    return (DsResult){DS_OK, "置换选择排序完成"};

fail:
    free(current.a);
    free(deferred.a);
    return (DsResult){DS_ERROR, "置换选择排序失败"};
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "位置/下标越界";
        case DS_OVERFLOW:    return "空间已满或内存分配失败";
        case DS_EMPTY:       return "对空结构取元素";
        case DS_NOT_FOUND:   return "查找未命中";
        default:             return "未知状态码";
    }
}
