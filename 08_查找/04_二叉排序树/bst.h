#ifndef BST_H
#define BST_H

/**
 * @file bst.h
 * @brief 二叉排序树（二叉搜索树）接口定义
 *
 * 性质: 左子树所有结点 < 根 < 右子树所有结点（子树递归成立）
 *   中序遍历 = 升序序列
 * 三个基本操作:
 *   1. 插入: 沿比较路径到底，挂新叶子 —— 新结点必为叶子
 *   2. 查找: 与折半类似，但树形由插入顺序决定
 *   3. 删除: 三情况
 *      a. 叶子: 直接删
 *      b. 单孩子: 孩子顶上
 *      c. 双孩子: 用中序前驱（左子树最右）的值替换，再删前驱
 * 复杂度: 高度 h 时，插入/查找/删除均 O(h)
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

/* 遍历回调（与链表 traverse 一致的回调风格） */
typedef void (*BSTVisit)(int value);

/* 二叉链表结点 */
typedef struct BSTNode {
    int              data;
    struct BSTNode  *left;
    struct BSTNode  *right;
} BSTNode;

typedef BSTNode *BSTree; /* 根指针，空树为 NULL */

DsResult bst_insert(BSTree *root, int key);
DsResult bst_search(BSTree root, int key, BSTNode **out);
DsResult bst_delete(BSTree *root, int key);
DsResult bst_clear(BSTree *root);
DsResult bst_destroy(BSTree *root);
size_t bst_count(BSTree root);
size_t bst_height(BSTree root);
DsResult bst_inorder(BSTree root, BSTVisit visit);
DsResult bst_print(BSTree root);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* BST_H */
