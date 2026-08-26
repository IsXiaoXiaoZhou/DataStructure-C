#ifndef FOREST_BITREE_H
#define FOREST_BITREE_H

/**
 * @file forest_bitree.h
 * @brief 森林与二叉树的相互转换接口定义
 *
 * 存储结构: 森林 = 孩子兄弟链的根经 next_sibling 串联；
 *           二叉树 = 同型结点（first_child->left, next_sibling->right）
 * 转换规则（教材标准）:
 *   森林 -> 二叉树:
 *     第一棵树的根为二叉树根；第 i+1 棵树根挂第 i 棵树根的右链
 *     树内: first_child -> 左孩子, next_sibling -> 右孩子
 *   二叉树 -> 森林: 上述过程的逆（沿根右链拆分）
 * 重要性质:
 *   森林先序遍历 == 转换后二叉树的先序遍历
 *   森林后序遍历 == 转换后二叉树的中序遍历
 * 设计要点:
 *   转换采用"复制重建"（不共享结点），转换后源与目标
 *   同时可读可销毁 —— 便于 roundtrip 对拍验证
 * 复杂度: 两个方向均 O(n)
 */

#include <stddef.h>

/* 状态码（按本模块所需裁剪，语义见各枚举项注释） */
typedef enum {
    DS_OK = 0,   /* 操作成功 */
    DS_ERROR,    /* 一般性错误（分配失败/空森林） */
    DS_NULL_PTR  /* 空指针参数 */
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 输出容量 */
#define FOREST_MAX_NODES 10000

/* 孩子兄弟链结点（森林与二叉树共用结点型） */
typedef struct CSNode {
    int data;
    struct CSNode *first_child;
    struct CSNode *next_sibling;
} CSNode;

/* 森林: 根链（第一棵树根，其 next_sibling 串其余树根） */
typedef struct {
    CSNode *roots;     /* 根链头；空森林为 NULL */
    size_t tree_count; /* 树的棵数 */
} Forest;

/* 二叉树: 即 CSNode 树（left=first_child, right=next_sibling） */
typedef CSNode *CSBinaryTree;

DsResult forest_init(Forest *f);
DsResult forest_destroy(Forest *f);
DsResult forest_add_tree(Forest *f, int data, CSNode **root);
DsResult forest_attach(Forest *f, CSNode *parent, CSNode *child);
size_t forest_count(const Forest *f);
size_t forest_tree_count(const Forest *f);
DsResult forest_preorder(const Forest *f, int out[], size_t *n);
DsResult forest_postorder(const Forest *f, int out[], size_t *n);
DsResult forest_to_bitree(const Forest *f, CSBinaryTree *bt);
DsResult bitree_to_forest(const CSBinaryTree bt, Forest *f);
DsResult bitree_destroy(CSBinaryTree *bt);
DsResult bt_preorder(const CSBinaryTree bt, int out[], size_t *n);
DsResult bt_inorder(const CSBinaryTree bt, int out[], size_t *n);
size_t bt_count(const CSBinaryTree bt);
DsResult forest_fingerprint(const Forest *f, int datas[], int fanouts[], size_t *n);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* FOREST_BITREE_H */
