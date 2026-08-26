#ifndef BIN_SEARCH_H
#define BIN_SEARCH_H

/**
 * @file bin_search.h
 * @brief 有序表三档折半式查找接口定义（要求 a 升序）
 *
 * 三个版本（同属"按值选址"的折半家族）:
 *   1. bin_search     折半查找: mid 恒取区间中点
 *   2. interp_search  插值查找: mid 按 key 在值域的占比插值
 *   3. fib_search     斐波那契查找: mid 按斐波那契数列分割
 */

#include <stddef.h>

/* 六值状态码（与全工程逐字一致） */
typedef enum {
    DS_OK = 0,          /* 操作成功 */
    DS_ERROR,           /* 一般性错误（未命中） */
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

DsResult bin_search(const int a[], size_t n, int key, size_t *pos);
DsResult interp_search(const int a[], size_t n, int key, size_t *pos);
DsResult fib_search(const int a[], size_t n, int key, size_t *pos);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* BIN_SEARCH_H */
