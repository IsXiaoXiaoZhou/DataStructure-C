#ifndef LOSER_TREE_H
#define LOSER_TREE_H

/**
 * @file loser_tree.h
 * @brief k 路归并"败者树"（最小关键字胜出）接口定义
 *
 * 败者树是一棵完全二叉树，内部结点存败者、根存胜者。
 * 复杂度：建树 O(k)；调整某段 O(log2(k))
 */

#include <stddef.h>
#include <limits.h>

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

/* k 路归并败者树（栈对象） */
typedef struct {
    int    *tree;  /* 树缓冲，容量 >= 2*size */
    int    *seg;   /* 各段当前关键字值，容量 >= size */
    size_t  size;  /* 补位后的规模：不小于 k 的最小 2 次幂 */
    size_t  k;     /* 真实归并段数 */
} LoserTree;

size_t lt_size_needed(size_t k);
DsResult lt_init(LoserTree *lt, int *tree_buf, int *seg, size_t k);
DsResult lt_rebuild(LoserTree *lt);
DsResult lt_winner(const LoserTree *lt, size_t *idx, int *val);
DsResult lt_adjust(LoserTree *lt, size_t i);
DsResult lt_exhaust(LoserTree *lt, size_t i);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* LOSER_TREE_H */
