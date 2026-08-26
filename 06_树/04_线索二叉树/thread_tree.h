#ifndef THREAD_TREE_H
#define THREAD_TREE_H

/**
 * @file thread_tree.h
 * @brief 线索二叉树（中序线索化）接口定义
 *
 * 存储结构: 二叉链表 + ltag/rtag 标志域
 *           tag==0 指针是孩子，tag==1 指针是线索（前驱/后继）
 * 设计要点:
 *   1. n+1 个空链域恰好可以存放 n-1 个前驱后继线索
 *      （中序首结点无前驱、尾结点无后继，线索置 NULL）
 *   2. 线索化后中序遍历无需栈无需递归:
 *      first(最左) -> 反复 next（线索直达或右子树最左）
 *   3. 先序/后序线索化思路相同（访问时机不同），
 *      但"求后继"仅中序线索可以纯线索完成且不加父指针，
 *      教学以中序线索为主（本模块即中序线索）
 * 复杂度: 线索化 O(n)；线索遍历 O(n) 且空间 O(1)（对比
 *         递归 O(h) 栈 / 非递归显式栈）
 */

#include <stddef.h>

/* 状态码（按本模块所需裁剪，语义见各枚举项注释） */
typedef enum {
    DS_OK = 0,   /* 操作成功 */
    DS_ERROR,    /* 一般性错误（无前驱/后继等） */
    DS_NULL_PTR  /* 空指针参数 */
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 遍历输出容量 */
#define THREAD_MAX_NODES 10000

/* 空子树标记 */
#define THREAD_NULL_FLAG (-1)

/* tag 取值 */
#define CHILD_PTR 0 /* 指针指向孩子 */
#define THREAD_PTR 1 /* 指针是线索 */

/* 线索二叉树结点 */
typedef struct ThreadNode {
    int data;
    struct ThreadNode *left;
    struct ThreadNode *right;
    int ltag; /* 0 孩子 / 1 前驱线索 */
    int rtag; /* 0 孩子 / 1 后继线索 */
} ThreadNode;

typedef ThreadNode *ThreadTree; /* 根指针 */

DsResult tt_create(const int arr[], size_t n, ThreadTree *t);
DsResult tt_inthreading(ThreadTree t);
ThreadNode *tt_first(ThreadTree t);
ThreadNode *tt_last(ThreadTree t);
ThreadNode *tt_next(const ThreadNode *node);
ThreadNode *tt_prev(const ThreadNode *node);
DsResult tt_inorder(ThreadTree t, int out[], size_t *n);
DsResult tt_inorder_reverse(ThreadTree t, int out[], size_t *n);
DsResult tt_destroy(ThreadTree *t);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* THREAD_TREE_H */
