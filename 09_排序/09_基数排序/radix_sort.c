/**
 * @file radix_sort.c
 * @brief LSD 基数排序实现
 *
 * 每位调用稳定计数排序：频次 -> 前缀和 -> 从右往左回填。
 */

#include <stdlib.h>

#include "radix_sort.h"


DsResult radix_sort(int a[], size_t n, unsigned base)
{
    int maxv = 0;
    unsigned long long cur = 1;
    unsigned digits = 0;
    size_t *count = NULL;
    int *out = NULL;
    size_t i = 0;
    unsigned d = 0;

    if (a == NULL) {
        return (DsResult){DS_NULL_PTR, "数组指针为空"};
    }
    if (base < 2) {
        return (DsResult){DS_OUT_OF_RANGE, "基数须 >= 2"};
    }
    if (n == 0) {
        return (DsResult){DS_OK, "排序完成"};
    }
    if (n == 1) {
        return (a[0] < 0) ? (DsResult){DS_OUT_OF_RANGE, "数组含负值"}
                          : (DsResult){DS_OK, "排序完成"};
    }

    /* 求最大值，确定位数与负值检查 */
    maxv = a[0];
    if (maxv < 0) {
        return (DsResult){DS_OUT_OF_RANGE, "数组含负值"};
    }
    for (i = 1; i < n; ++i) {
        if (a[i] > maxv) maxv = a[i];
        if (a[i] < 0) {
            return (DsResult){DS_OUT_OF_RANGE, "数组含负值"};
        }
    }

    {
        unsigned long long m = (unsigned long long)maxv;
        digits = 1;
        while (m >= (unsigned long long)base) {
            m /= (unsigned long long)base;
            ++digits;
        }
    }

    count = calloc((size_t)base, sizeof *count);
    out = malloc(n * sizeof *out);
    if (count == NULL || out == NULL) {
        free(count);
        free(out);
        return (DsResult){DS_OVERFLOW, "辅助空间分配失败"};
    }

    for (d = 0; d < digits; ++d) {
        for (i = 0; i < (size_t)base; ++i) {
            count[i] = 0;
        }
        for (i = 0; i < n; ++i) {
            size_t digit = (size_t)(((unsigned long long)a[i] / cur) % (unsigned long long)base);
            ++count[digit];
        }
        for (i = 1; i < (size_t)base; ++i) {
            count[i] += count[i - 1];
        }
        for (i = n; i > 0; --i) {
            size_t digit = (size_t)(((unsigned long long)a[i - 1] / cur) % (unsigned long long)base);
            out[count[digit] - 1] = a[i - 1];
            --count[digit];
        }
        for (i = 0; i < n; ++i) {
            a[i] = out[i];
        }
        cur *= (unsigned long long)base;
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
