/**
 * @file insert_sort.c
 * @brief 直接插入排序实现
 */

#include "insert_sort.h"


DsResult insert_sort(int a[], size_t n)
{
    size_t i = 0;
    size_t j = 0;
    int key = 0;

    if (a == NULL) {
        return (DsResult){DS_NULL_PTR, "数组指针为空"};
    }
    if (n <= 1) {
        return (DsResult){DS_OK, "排序完成"};
    }

    for (i = 1; i < n; ++i) {
        key = a[i];
        j = i;
        while (j > 0 && key < a[j - 1]) {
            a[j] = a[j - 1];
            --j;
        }
        a[j] = key;
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
        case DS_OVERFLOW:    return "空间已满或内存分配失败";
        case DS_EMPTY:       return "对空结构取元素";
        case DS_NOT_FOUND:   return "查找未命中";
        default:             return "未知状态码";
    }
}
