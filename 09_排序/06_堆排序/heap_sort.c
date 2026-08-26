/**
 * @file heap_sort.c
 * @brief 大根堆（建堆/插入/删除）与堆排序实现
 *
 * 堆 ADT 元素存于 buf[1..size]（1 基），父 = i/2，子 = 2i / 2i+1。
 * heap_sort 数组为 0 基，内部用 0 基下沉辅助。
 */

#include "heap_sort.h"


/* 1 基上浮: 将 buf[i] 上浮到合适位置 */
static void sift_up_1(int *buf, size_t i)
{
    while (i > 1) {
        size_t parent = i / 2;
        if (buf[parent] >= buf[i]) break;
        int t = buf[parent];
        buf[parent] = buf[i];
        buf[i] = t;
        i = parent;
    }
}

/* 1 基下沉: 将 buf[i] 下沉到合适位置，堆规模为 size */
static void sift_down_1(int *buf, size_t size, size_t i)
{
    while (1) {
        size_t left = 2 * i;
        size_t right = left + 1;
        size_t largest = i;

        if (left <= size && buf[left] > buf[largest]) {
            largest = left;
        }
        if (right <= size && buf[right] > buf[largest]) {
            largest = right;
        }
        if (largest == i) break;
        int t = buf[i];
        buf[i] = buf[largest];
        buf[largest] = t;
        i = largest;
    }
}

/* 0 基下沉: 将 a[i] 下沉到合适位置，堆规模为 n */
static void sift_down_0(int *a, size_t n, size_t i)
{
    while (1) {
        size_t left = 2 * i + 1;
        size_t right = left + 1;
        size_t largest = i;

        if (left < n && a[left] > a[largest]) {
            largest = left;
        }
        if (right < n && a[right] > a[largest]) {
            largest = right;
        }
        if (largest == i) break;
        int t = a[i];
        a[i] = a[largest];
        a[largest] = t;
        i = largest;
    }
}

DsResult heap_create(DsHeap *h, int *buf, size_t cap)
{
    if (h == NULL || buf == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    h->buf = buf;
    h->cap = cap;
    h->size = 0;
    return (DsResult){DS_OK, "堆创建成功"};
}

DsResult heap_insert(DsHeap *h, int value)
{
    if (h == NULL || h->buf == NULL) {
        return (DsResult){DS_NULL_PTR, "堆指针为空"};
    }
    if (h->size >= h->cap) {
        return (DsResult){DS_OVERFLOW, "堆已满，无法插入"};
    }
    ++h->size;
    h->buf[h->size] = value;
    sift_up_1(h->buf, h->size);
    return (DsResult){DS_OK, "插入成功"};
}

DsResult heap_peek(const DsHeap *h, int *out)
{
    if (h == NULL || h->buf == NULL || out == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (h->size == 0) {
        return (DsResult){DS_EMPTY, "堆为空，无法取堆顶"};
    }
    *out = h->buf[1];
    return (DsResult){DS_OK, "取堆顶成功"};
}

DsResult heap_delete_top(DsHeap *h, int *out)
{
    if (h == NULL || h->buf == NULL || out == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (h->size == 0) {
        return (DsResult){DS_EMPTY, "堆为空，无法删除"};
    }
    *out = h->buf[1];
    h->buf[1] = h->buf[h->size];
    --h->size;
    if (h->size > 0) {
        sift_down_1(h->buf, h->size, 1);
    }
    return (DsResult){DS_OK, "删除成功"};
}

void heap_clear(DsHeap *h)
{
    if (h != NULL) {
        h->size = 0;
    }
}

DsResult heap_sort(int a[], size_t n)
{
    size_t i = 0;
    size_t heap = 0;

    if (a == NULL) {
        return (DsResult){DS_NULL_PTR, "数组指针为空"};
    }
    if (n <= 1) {
        return (DsResult){DS_OK, "排序完成"};
    }

    /* 自底向上建大根堆 */
    i = n / 2;
    while (i > 0) {
        --i;
        sift_down_0(a, n, i);
    }

    /* 排序：反复把堆顶换到末尾 */
    heap = n;
    while (heap > 1) {
        int t = a[0];
        a[0] = a[heap - 1];
        a[heap - 1] = t;
        --heap;
        sift_down_0(a, heap, 0);
    }
    return (DsResult){DS_OK, "排序完成"};
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "位置/下标越界";
        case DS_OVERFLOW:    return "堆已满";
        case DS_EMPTY:       return "堆为空";
        case DS_NOT_FOUND:   return "查找未命中";
        default:             return "未知状态码";
    }
}
