#ifndef RADIX_SORT_H
#define RADIX_SORT_H

/**
 * @file radix_sort.h
 * @brief LSD 基数排序接口定义
 *
 * 从最低位到最高位逐位做"稳定计数排序"。
 * 要求: 元素为非负整数；base >= 2。
 * 复杂度: O(d(n + base))，d 为最大数的位数。
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

DsResult radix_sort(int a[], size_t n, unsigned base);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* RADIX_SORT_H */
