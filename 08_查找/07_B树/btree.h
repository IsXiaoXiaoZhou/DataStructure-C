#ifndef BTREE_H
#define BTREE_H

/**
 * @file btree.h
 * @brief B 树接口定义（多路平衡查找树，磁盘索引的经典结构）
 *
 * 定义: 一棵 m 阶 B 树（此处用最小度数 t 表述, 即每结点至多 2t-1 个
 *   关键字、至少 t-1 个；等价 m=2t）
 * 性质:
 *   1. 根关键字数 ∈ [1, 2t-1]，内结点 ∈ [t-1, 2t-1]
 *   2. 每个内结点有 m+1 个孩子（m 为关键字数）
 *   3. 结点内关键字升序，孩子子树间也保持有序（多路 BST）
 *   4. 所有叶子在同一层（叶子等深 → 查找路径等长，磁盘 IO 均衡）
 * 复杂度: 插入/删除/查找均 O(t·log_t n)
 */

#include <stddef.h>

/* 六值状态码（与全工程逐字一致） */
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

/* B 树最小度数: 结点关键字数 ∈ [t-1, 2t-1] */
#define BTREE_T 3
#define BTREE_MAX_KEYS (2 * BTREE_T - 1)  /* 每结点最多关键字 */
#define BTREE_MIN_KEYS (BTREE_T - 1)      /* 内结点最少关键字(根除外) */

/* 遍历回调（与全工程风格一致） */
typedef void (*BTreeVisit)(int value);

typedef struct BTNode {
    size_t           n;          /* 本结点关键字个数 */
    int              keys[BTREE_MAX_KEYS];
    struct BTNode   *child[BTREE_MAX_KEYS + 1];
    int              is_leaf;    /* 1=叶子(无孩子) */
} BTNode;

typedef BTNode *BTree; /* 根指针，空树为 NULL */

DsResult btree_insert(BTree *root, int key);
DsResult btree_search(const BTree root, int key, BTNode **node, size_t *idx);
DsResult btree_delete(BTree *root, int key);
size_t btree_key_count(const BTree root);
size_t btree_height(const BTree root);
DsResult btree_verify(const BTree root, size_t *violations);
DsResult btree_inorder(const BTree root, BTreeVisit visit);
DsResult btree_destroy(BTree *root);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* BTREE_H */
