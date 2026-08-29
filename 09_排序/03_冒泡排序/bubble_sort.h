#ifndef BUBBLE_SORT_H
#define BUBBLE_SORT_H

/**
 * @file bubble_sort.h
 * @brief 冒泡排序接口定义
 *
 * 适用: 小规模 / 基本有序数据；原地排序，稳定
 * 复杂度: O(n^2) 最坏/平均；O(n) 最好(已有序)
 * 稳定性: 稳定
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

DsResult bubble_sort(int a[], size_t n);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* BUBBLE_SORT_H */
