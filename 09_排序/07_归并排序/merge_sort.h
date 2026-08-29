#ifndef MERGE_SORT_H
#define MERGE_SORT_H

/**
 * @file merge_sort.h
 * @brief 二路归并排序接口定义
 *
 * 提供两种实现:
 *   A. merge_sort          递归版（自顶向下）
 *   B. merge_sort_iter     迭代版（自底向上）
 * 两者共用 merge 辅助，稳定，O(n log n)。
 * 需要 O(n) 辅助空间。
 */

#include <stddef.h>

/* 七值状态码（与全工程逐字一致） */
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

DsResult merge_sort(int a[], size_t n);
DsResult merge_sort_iter(int a[], size_t n);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* MERGE_SORT_H */
