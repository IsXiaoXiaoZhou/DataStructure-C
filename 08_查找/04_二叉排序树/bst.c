/**
 * @file bst.c
 * @brief 二叉排序树实现（插入/查找/删除/清空/遍历）
 */

#include <stdio.h>
#include <stdlib.h>

#include "bst.h"


/* ---------- 基础工具 ---------- */

/* 新建结点（data 已定，左右孩子置空） */
static BSTNode *new_node(int key)
{
    BSTNode *p = malloc(sizeof *p);
    if (p != NULL) {
        p->data = key;
        p->left = NULL;
        p->right = NULL;
    }
    return p;
}

DsResult bst_insert(BSTree *root, int key)
{
    BSTNode *p = NULL;

    if (root == NULL) {
        return (DsResult){DS_NULL_PTR, "根指针指针为空"};
    }
    if (*root == NULL) {            /* 空树/空子树: 挂新叶子 */
        p = new_node(key);
        if (p == NULL) {
            return (DsResult){DS_OVERFLOW, "结点内存分配失败"};
        }
        *root = p;
        return (DsResult){DS_OK, "插入成功"};
    }
    if (key < (*root)->data) {
        return bst_insert(&(*root)->left, key);
    }
    if (key > (*root)->data) {
        return bst_insert(&(*root)->right, key);
    }
    return (DsResult){DS_ERROR, "关键字已存在，不允许重复插入"};                /* 重复: 不插入 */
}

DsResult bst_search(BSTree root, int key, BSTNode **out)
{
    if (out != NULL) {
        *out = NULL;
    }
    while (root != NULL) {
        if (key == root->data) {
            if (out != NULL) {
                *out = root;
            }
            return (DsResult){DS_OK, "查找成功"};
        }
        root = (key < root->data) ? root->left : root->right;
    }
    return (DsResult){DS_NOT_FOUND, "未找到目标关键字"};
}

/* 删除 *p 指向的结点并修正结构（p 指向双亲的孩子指针或根指针） */
static DsStatus del_node(BSTNode **p)
{
    BSTNode *q = NULL;
    BSTNode *s = NULL;

    if (*p == NULL) {
        return DS_ERROR;
    }
    if ((*p)->left == NULL) {       /* 无左子树: 右孩子(或空)顶上 */
        q = *p;
        *p = q->right;
        free(q);
        return DS_OK;
    }
    if ((*p)->right == NULL) {      /* 无右子树: 左孩子顶上 */
        q = *p;
        *p = q->left;
        free(q);
        return DS_OK;
    }
    /* 双孩子: 找中序前驱（左子树最右结点）替换值 */
    q = *p;
    s = (*p)->left;
    while (s->right != NULL) {
        q = s;
        s = s->right;
    }
    (*p)->data = s->data;           /* 前驱值覆盖被删结点 */
    if (q != *p) {
        q->right = s->left;         /* 前驱是其父的右孩子: 左子树顶上 */
    } else {
        q->left = s->left;          /* 前驱是根的左孩子直接 */
    }
    free(s);
    return DS_OK;
}

DsResult bst_delete(BSTree *root, int key)
{
    if (root == NULL) {
        return (DsResult){DS_NULL_PTR, "根指针指针为空"};
    }
    if (*root == NULL) {
        return (DsResult){DS_NOT_FOUND, "树为空，无法删除"};
    }
    if (key == (*root)->data) {
        return (del_node(root) == DS_OK) ? (DsResult){DS_OK, "删除成功"}
                                         : (DsResult){DS_ERROR, "删除失败"};
    }
    if (key < (*root)->data) {
        return bst_delete(&(*root)->left, key);
    }
    return bst_delete(&(*root)->right, key);
}

DsResult bst_clear(BSTree *root)
{
    if (root == NULL) {
        return (DsResult){DS_NULL_PTR, "根指针指针为空"};
    }
    if (*root == NULL) {
        return (DsResult){DS_OK, "已是空树"};
    }
    (void)bst_clear(&(*root)->left);
    (void)bst_clear(&(*root)->right);
    free(*root);
    *root = NULL;
    return (DsResult){DS_OK, "清空成功"};
}

DsResult bst_destroy(BSTree *root)
{
    return bst_clear(root);
}

size_t bst_count(BSTree root)
{
    if (root == NULL) {
        return 0;
    }
    return 1 + bst_count(root->left) + bst_count(root->right);
}

size_t bst_height(BSTree root)
{
    size_t hl = 0;
    size_t hr = 0;

    if (root == NULL) {
        return 0;
    }
    hl = bst_height(root->left);
    hr = bst_height(root->right);
    return (hl > hr ? hl : hr) + 1;
}

DsResult bst_inorder(BSTree root, BSTVisit visit)
{
    if (visit == NULL) {
        return (DsResult){DS_NULL_PTR, "回调函数为空"};
    }
    if (root == NULL) {
        return (DsResult){DS_OK, "空树遍历完成"};
    }
    (void)bst_inorder(root->left, visit);
    visit(root->data);
    (void)bst_inorder(root->right, visit);
    return (DsResult){DS_OK, "中序遍历完成"};
}

/* ---------- 树形打印（右子树在上，逆时针旋转 90 度） ---------- */

static void print_recur(BSTNode *p, int depth)
{
    int d = depth;

    if (p == NULL) {
        return;
    }
    if (p->right != NULL) {
        print_recur(p->right, depth + 1);
    }
    while (d-- > 0) {
        printf("    ");
    }
    printf("%d\n", p->data);
    if (p->left != NULL) {
        print_recur(p->left, depth + 1);
    }
}

DsResult bst_print(BSTree root)
{
    if (root == NULL) {
        return (DsResult){DS_NULL_PTR, "树为空"};
    }
    print_recur(root, 0);
    return (DsResult){DS_OK, "打印完成"};
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "位置/下标越界";
        case DS_OVERFLOW:    return "内存分配失败";
        case DS_EMPTY:       return "树为空";
        case DS_NOT_FOUND:   return "未找到目标关键字";
        default:             return "未知状态码";
    }
}
