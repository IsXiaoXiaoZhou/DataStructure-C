#ifndef AVL_H
#define AVL_H

/**
 * @file avl.h
 * @brief 平衡二叉树（AVL）接口定义
 *
 * 平衡条件: 任意结点的平衡因子 BF = 左子树高 - 右子树高 ∈ {-1,0,1}
 * 插入四旋转（在失衡结点 x 上）:
 *   LL: 插在 x 左孩子的左子树 → 右单旋
 *   RR: 插在 x 右孩子的右子树 → 左单旋
 *   LR: 插在 x 左孩子的右子树 → 左旋左孩子 + 右旋
 *   RL: 插在 x 右孩子的左子树 → 右旋右孩子 + 左旋
 * 复杂度: 插入/删除/查找均 O(log n)
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

/* 遍历回调（同 bst 风格） */
typedef void (*AVLVisit)(int value);

/* 结点: data + 子树高度（叶高 1）+ 左右孩子 */
typedef struct AVLNode {
    int              data;
    int              height;   /* 以本结点为根的子树高度，叶=1 */
    struct AVLNode  *left;
    struct AVLNode  *right;
} AVLNode;

typedef AVLNode *AVLTree; /* 根指针，空树为 NULL */

DsResult avl_insert(AVLTree *root, int key);
DsResult avl_delete(AVLTree *root, int key);
DsResult avl_search(AVLTree root, int key, AVLNode **out);
DsResult avl_verify(AVLTree root, size_t *violations);
DsResult avl_clear(AVLTree *root);
DsResult avl_destroy(AVLTree *root);
size_t avl_count(AVLTree root);
size_t avl_height(AVLTree root);
DsResult avl_inorder(AVLTree root, AVLVisit visit);
DsResult avl_print(AVLTree root);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* AVL_H */
