/**
 * @file merge_sort.c
 * @brief 递归 / 迭代二路归并排序实现
 */

#include <stdlib.h>

#include "merge_sort.h"


/* 归并 a[lo..mid] 与 a[mid+1..hi]，经 tmp 回写 */
static void merge_range(int a[], int *tmp, size_t lo, size_t mid, size_t hi)
{
    size_t i = lo;
    size_t j = mid + 1;
    size_t k = lo;

    while (i <= mid && j <= hi) {
        if (a[i] <= a[j]) {
            tmp[k++] = a[i++];
        } else {
            tmp[k++] = a[j++];
        }
    }
    while (i <= mid) {
        tmp[k++] = a[i++];
    }
    while (j <= hi) {
        tmp[k++] = a[j++];
    }
    for (k = lo; k <= hi; ++k) {
        a[k] = tmp[k];
    }
}

/* 递归主体 */
static void merge_range_rec(int a[], int *tmp, size_t lo, size_t hi)
{
    size_t mid = 0;
    if (lo >= hi) {
        return;
    }
    mid = lo + (hi - lo) / 2;
    merge_range_rec(a, tmp, lo, mid);
    merge_range_rec(a, tmp, mid + 1, hi);
    merge_range(a, tmp, lo, mid, hi);
}

DsResult merge_sort(int a[], size_t n)
{
    int *tmp = NULL;
    if (a == NULL) {
        return (DsResult){DS_NULL_PTR, "数组指针为空"};
    }
    if (n <= 1) {
        return (DsResult){DS_OK, "排序完成"};
    }
    tmp = malloc(n * sizeof *tmp);
    if (tmp == NULL) {
        return (DsResult){DS_OVERFLOW, "辅助空间分配失败"};
    }
    merge_range_rec(a, tmp, 0, n - 1);
    free(tmp);
    return (DsResult){DS_OK, "排序完成"};
}

DsResult merge_sort_iter(int a[], size_t n)
{
    int *tmp = NULL;
    size_t width = 0;

    if (a == NULL) {
        return (DsResult){DS_NULL_PTR, "数组指针为空"};
    }
    if (n <= 1) {
        return (DsResult){DS_OK, "排序完成"};
    }
    tmp = malloc(n * sizeof *tmp);
    if (tmp == NULL) {
        return (DsResult){DS_OVERFLOW, "辅助空间分配失败"};
    }

    for (width = 1; width < n; width *= 2) {
        size_t lo = 0;
        for (lo = 0; lo + width < n; lo += 2 * width) {
            size_t mid = lo + width - 1;
            size_t hi = 0;
            if (lo + 2 * width - 1 < n) {
                hi = lo + 2 * width - 1;
            } else {
                hi = n - 1;
            }
            merge_range(a, tmp, lo, mid, hi);
        }
    }
    free(tmp);
    return (DsResult){DS_OK, "排序完成"};
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "位置/下标越界";
        case DS_OVERFLOW:    return "辅助空间分配失败";
        case DS_EMPTY:       return "对空结构取元素";
        case DS_NOT_FOUND:   return "查找未命中";
        default:             return "未知状态码";
    }
}
