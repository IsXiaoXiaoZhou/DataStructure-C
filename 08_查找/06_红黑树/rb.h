#ifndef RB_H
#define RB_H

/**
 * @file rb.h
 * @brief 红黑树接口定义
 *
 * 五性质（红黑树是"弱平衡"的 BST，黑高一致换来较少的旋转）:
 *   1. 每个结点非红即黑
 *   2. 根是黑色
 *   3. 每个叶结点(NULL)是黑色
 *   4. 红结点的两个子结点均为黑色 —— 等价于无相邻红结点
 *   5. 任一结点到所有后代叶的路径含相同个数的黑结点（黑高相等）
 * 推论: 高度 ≤ 2·log2(n+1)（红结点至多占一半）
 */

#include <stddef.h>

/* 七值状态码（与全工程逐字一致） */
typedef enum {
    DS_OK = 0,          /* 操作成功 */
    DS_ERROR,           /* 一般性错误（查找/删除未命中、插入重复） */
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

typedef enum { RB_RED = 0, RB_BLACK = 1 } RBColor;

typedef struct RBNode {
    int              data;
    RBColor          color;
    struct RBNode   *left;
    struct RBNode   *right;
    struct RBNode   *parent;  /* 根父为 NULL；NULL 视为黑色叶 */
} RBNode;

/* 红黑树（栈对象: 结构体本体放调用方栈上） */
typedef struct {
    RBNode *root;   /* 空树为 NULL */
} RBTree;

void rb_init(RBTree *t);
DsResult rb_insert(RBTree *t, int key);
DsResult rb_delete(RBTree *t, int key);
DsResult rb_search(const RBTree *t, int key, RBNode **out);
DsResult rb_verify(const RBTree *t, size_t *violations);
DsResult rb_clear(RBTree *t);
DsResult rb_destroy(RBTree *t);
size_t rb_count(const RBTree *t);
size_t rb_height(const RBTree *t);
DsResult rb_inorder(const RBTree *t, void (*visit)(int value));

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* RB_H */
