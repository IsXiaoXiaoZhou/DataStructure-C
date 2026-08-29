#ifndef SEQ_SEARCH_H
#define SEQ_SEARCH_H

/**
 * @file seq_search.h
 * @brief 顺序查找接口定义
 *
 * 适用: 顺序表/链表上的查找，不要求有序
 * 两个版本:
 *   1. seq_search           朴素版: 每轮先判 i<n 再判 key，至多两次比较/轮
 *   2. seq_search_sentinel  哨兵版: 预埋 a[n]=key，省去每轮边界判断
 *      找到即停；成功比较次数 = 位置+1，均摊省掉 ~n 次边界比较
 * ASL 分析（成功）: (1+2+...+n)/n = (n+1)/2 次比较
 *   失败需 n+1 次比较（朴素）/ n+1 次（哨兵，多算一次哨兵命中）
 * 补充价值: 哨兵是"去循环不变量"的经典手法，
 *   与快排哨兵、KMP 的 next 思想同源
 */

#include <stddef.h>

/* 七值状态码（与全工程逐字一致） */
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

DsResult seq_search(const int a[], size_t n, int key, size_t *pos);
DsResult seq_search_sentinel(int a[], size_t n, int key, size_t *pos);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* SEQ_SEARCH_H */
