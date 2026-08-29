#ifndef REPLACEMENT_SELECTION_H
#define REPLACEMENT_SELECTION_H

/**
 * @file replacement_selection.h
 * @brief 置换选择排序（生成初始归并段）接口定义
 *
 * 用容量为 m 的最小堆作内存工作区，从输入序列逐记录生成若干已有序的初始归并段。
 * 归并段平均长度约 2m（雪堆效应），比逐块排序产生更少的初始归并段。
 */

#include <stddef.h>
#include <stdio.h>

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

/** @brief 置换选择生成的报告：段数 / 记录总数 */
typedef struct {
    size_t runs;   /* 生成的初始归并段数 */
    size_t total;  /* 输出的记录总数（不含段头长度） */
} RunReport;

DsResult replacement_selection(FILE *in, FILE *out, int *work, size_t m, RunReport *rep);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* REPLACEMENT_SELECTION_H */
