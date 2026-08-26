#ifndef EXTERNAL_MERGE_H
#define EXTERNAL_MERGE_H

/**
 * @file external_merge.h
 * @brief 外部 k 路归并接口定义
 *
 * 把 k 个已升序排好的输入文件用"败者树"做 k 路选择，
 * 依次取全局最小写入输出文件，得到一个全局升序的大文件。
 * 复杂度：O(n log k)，n=总记录数。内存开销 O(k)。
 */

#include <stddef.h>
#include <stdio.h>

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

DsResult external_merge(FILE **in, size_t k, FILE *out);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* EXTERNAL_MERGE_H */
