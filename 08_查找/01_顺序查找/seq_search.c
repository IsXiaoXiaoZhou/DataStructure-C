/**
 * @file seq_search.c
 * @brief 顺序查找实现（朴素版 + 哨兵版）
 */

#include "seq_search.h"


DsResult seq_search(const int a[], size_t n, int key, size_t *pos)
{
    size_t i = 0;

    if (a == NULL || pos == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (n == 0) {
        return (DsResult){DS_EMPTY, "查找表为空"};
    }

    for (i = 0; i < n; ++i) {
        if (a[i] == key) {
            *pos = i;
            return (DsResult){DS_OK, "查找成功"};
        }
    }
    return (DsResult){DS_NOT_FOUND, "未找到目标元素"};
}

/* 哨兵版: 表尾预埋 key，循环少判 i<n —— 命中位置即比较次数-1 */
DsResult seq_search_sentinel(int a[], size_t n, int key, size_t *pos)
{
    size_t i = 0;

    if (a == NULL || pos == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (n == 0) {
        return (DsResult){DS_EMPTY, "查找表为空"};
    }

    a[n] = key;                     /* 哨兵: 兜底保证比到 n 必命中 */
    for (i = 0; a[i] != key; ++i) { /* 循环体为空，仅以比较驱动前进 */
        /* 空循环体: 命中或走到哨兵即停止 */
    }
    if (i < n) {                    /* 在数据区命中 */
        *pos = i;
        return (DsResult){DS_OK, "查找成功"};
    }
    return (DsResult){DS_NOT_FOUND, "未找到目标元素"};            /* 命中的是哨兵 → 未找到 */
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "位置/下标越界";
        case DS_OVERFLOW:    return "空间已满或内存分配失败";
        case DS_EMPTY:       return "查找表为空";
        case DS_NOT_FOUND:   return "未找到目标元素";
        default:             return "未知状态码";
    }
}
