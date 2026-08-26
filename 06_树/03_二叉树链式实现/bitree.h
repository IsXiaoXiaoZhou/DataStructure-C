#ifndef BITREE_H
#define BITREE_H

/**
 * @file bitree.h
 * @brief 二叉树的链式实现（二叉链表）接口定义
 *
 * 存储结构: 每结点 data + left + right，根指针为空即空树
 * 设计要点:
 *   1. 遍历是二叉树的灵魂: 先/中/后序递归版 + 先/中/后序
 *      非递归栈版 + 层序队列版共 7 种，递归与非递归对拍验证
 *   2. 建树采用"带空标记的先序数组"（-1 表示空子树），
 *      这是考研/教材标准输入形式
 *   3. n 结点有 n+1 个空链域 —— 线索化的起点（见 04 模块）
 * 复杂度: 遍历/统计类 O(n)；深度 O(n)
 */

#include <stddef.h>

/* 状态码（按本模块所需裁剪，语义见各枚举项注释） */
typedef enum {
    DS_OK = 0,    /* 操作成功 */
    DS_ERROR,     /* 一般性错误 */
    DS_NULL_PTR,  /* 空指针参数 */
    DS_OVERFLOW   /* 栈/队列容量不足（防御性） */
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 遍历输出数组最大容量（测试对拍用） */
#define BITREE_MAX_NODES 100000

/* 空子树标记（建树数组中 -1 表示空） */
#define BITREE_NULL_FLAG (-1)

/* 二叉链表结点 */
typedef struct BiTNode {
    int data;
    struct BiTNode *left;
    struct BiTNode *right;
} BiTNode;

typedef BiTNode *BiTree; /* 根指针，空树为 NULL */

DsResult bitree_create(const int arr[], size_t n, BiTree *t);
DsResult bitree_destroy(BiTree *t);
size_t bitree_count(BiTree t);
size_t bitree_leaf_count(BiTree t);
size_t bitree_depth(BiTree t);
DsResult bitree_preorder(BiTree t, int out[], size_t *n);
DsResult bitree_inorder(BiTree t, int out[], size_t *n);
DsResult bitree_postorder(BiTree t, int out[], size_t *n);
DsResult bitree_preorder_iter(BiTree t, int out[], size_t *n);
DsResult bitree_inorder_iter(BiTree t, int out[], size_t *n);
DsResult bitree_postorder_iter(BiTree t, int out[], size_t *n);
DsResult bitree_level_order(BiTree t, int out[], size_t *n);
DsResult bitree_print(BiTree t);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* BITREE_H */
