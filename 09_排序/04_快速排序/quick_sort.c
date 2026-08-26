/**
 * @file quick_sort.c
 * @brief 快速排序实现（三数取中 + Lomuto 分区）
 */

#include "quick_sort.h"


/* 交换两个元素 */
static void swap(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

/* 三数取中: 将中位数放到 a[hi] 作为 pivot */
static void median_of_three(int a[], int lo, int hi)
{
    int mid = lo + (hi - lo) / 2;
    if (a[lo] > a[mid]) swap(&a[lo], &a[mid]);
    if (a[lo] > a[hi])  swap(&a[lo], &a[hi]);
    if (a[mid] > a[hi]) swap(&a[mid], &a[hi]);
    swap(&a[mid], &a[hi]); /* 中位数放到 hi 位置 */
}

/* Lomuto 分区: 返回 pivot 最终位置 */
static int partition(int a[], int lo, int hi)
{
    int pivot = a[hi];
    int i = lo;
    int j = lo;

    for (j = lo; j < hi; ++j) {
        if (a[j] <= pivot) {
            swap(&a[i], &a[j]);
            ++i;
        }
    }
    swap(&a[i], &a[hi]);
    return i;
}

/* 递归排序区间 [lo, hi] */
static void quick_range(int a[], int lo, int hi)
{
    if (lo >= hi) {
        return;
    }
    median_of_three(a, lo, hi);
    {
        int p = partition(a, lo, hi);
        quick_range(a, lo, p - 1);
        quick_range(a, p + 1, hi);
    }
}

DsResult quick_sort(int a[], size_t n)
{
    if (a == NULL) {
        return (DsResult){DS_NULL_PTR, "数组指针为空"};
    }
    if (n <= 1) {
        return (DsResult){DS_OK, "排序完成"};
    }
    quick_range(a, 0, (int)n - 1);
    return (DsResult){DS_OK, "排序完成"};
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
