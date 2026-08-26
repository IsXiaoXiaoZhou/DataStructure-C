/**
 * @file counting_sort.c
 * @brief 稳定计数排序实现
 *
 * 关键点: 三段走 - 统计频次 -> 前缀和 -> 从右往左回填保证稳定。
 */

#include <stdlib.h>

#include "counting_sort.h"


DsResult counting_sort(int a[], size_t n, unsigned max_key)
{
    size_t k = (size_t)max_key + 1;
    size_t *count = NULL;
    int *out = NULL;
    size_t i = 0;

    if (a == NULL) {
        return (DsResult){DS_NULL_PTR, "数组指针为空"};
    }
    if (n == 0) {
        return (DsResult){DS_OK, "排序完成"};
    }
    if (n == 1) {
        return (a[0] < 0 || (unsigned)a[0] > max_key)
            ? (DsResult){DS_OUT_OF_RANGE, "元素值超出范围"}
            : (DsResult){DS_OK, "排序完成"};
    }

    for (i = 0; i < n; ++i) {
        if (a[i] < 0 || (unsigned)a[i] > max_key) {
            return (DsResult){DS_OUT_OF_RANGE, "元素值超出范围"};
        }
    }

    count = calloc(k, sizeof *count);
    out = malloc(n * sizeof *out);
    if (count == NULL || out == NULL) {
        free(count);
        free(out);
        return (DsResult){DS_OVERFLOW, "辅助空间分配失败"};
    }

    /* 1. 频次统计 */
    for (i = 0; i < n; ++i) {
        ++count[(unsigned)a[i]];
    }
    /* 2. 前缀和 */
    for (i = 1; i < k; ++i) {
        count[i] += count[i - 1];
    }
    /* 3. 从右往左回填，保证稳定 */
    for (i = n; i > 0; --i) {
        size_t v = (unsigned)a[i - 1];
        out[count[v] - 1] = a[i - 1];
        --count[v];
    }
    for (i = 0; i < n; ++i) {
        a[i] = out[i];
    }

    free(count);
    free(out);
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
