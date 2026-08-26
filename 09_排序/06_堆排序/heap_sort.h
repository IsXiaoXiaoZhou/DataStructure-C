#ifndef HEAP_SORT_H
#define HEAP_SORT_H

/**
 * @file heap_sort.h
 * @brief 大根堆 + 堆排序接口定义
 *
 * 本模块提供两部分能力:
 *   A. 大根堆 ADT（DsHeap，栈对象）
 *   B. heap_sort 原地堆排序（数组 0 基）
 * 复杂度: O(n log n)，原地，不稳定
 */

#include <stddef.h>

/* 六值状态码（与全工程逐字一致） */
typedef enum {
    DS_OK = 0,          /* 操作成功 */
    DS_ERROR,           /* 一般性错误 */
    DS_NULL_PTR,        /* 空指针参数 */
    DS_OUT_OF_RANGE,    /* 位置/下标越界 */
    DS_OVERFLOW,        /* 空间已满或内存分配失败 */
    DS_EMPTY,           // 对空结构取元素
    DS_NOT_FOUND        // 查找未命中（业务正常结果，非异常）
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 大根堆（栈对象，叠在调用方缓冲上）。元素存放于 buf[1..size]，buf[0] 留空 */
typedef struct {
    int *buf;     /* 调用方提供的缓冲，容量须 >= cap+1 个 int */
    size_t cap;   /* 最大元素个数（buf[1..cap] 可用） */
    size_t size;  /* 当前元素个数 */
} DsHeap;

DsResult heap_create(DsHeap *h, int *buf, size_t cap);
DsResult heap_insert(DsHeap *h, int value);
DsResult heap_peek(const DsHeap *h, int *out);
DsResult heap_delete_top(DsHeap *h, int *out);
void heap_clear(DsHeap *h);
DsResult heap_sort(int a[], size_t n);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* HEAP_SORT_H */
