/**
 * @file forest_bitree.c
 * @brief 森林与二叉树互转接口实现
 *
 * 设计要点:
 *   1. 复制式互转: copy_cs 递归复制孩子兄弟链（先孩子后兄弟），
 *      森林根链 <-> 二叉树右链的搬运在顶层完成
 *   2. 指纹 = 先根序列 + 孩子数序列，可唯一重建形态，
 *      是 roundtrip 一致性的判定基础
 * 复杂度: 互转/遍历/指纹均 O(n)
 */

#include <stdlib.h>

#include "forest_bitree.h"


/* 新建结点辅助 */
static CSNode *new_node(int data)
{
    CSNode *node = (CSNode *)malloc(sizeof(CSNode));

    if (node != NULL) {
        node->data = data;
        node->first_child = NULL;
        node->next_sibling = NULL;
    }
    return node;
}

/* 孩子兄弟链结点计数 */
static size_t count_cs(const CSNode *node)
{
    if (node == NULL) {
        return 0;
    }
    return 1 + count_cs(node->first_child) + count_cs(node->next_sibling);
}

/* 先根遍历一棵树（不跨树: 根链兄弟由 forest_* 顶层逐棵驱动） */
static void preorder_cs(const CSNode *node, int out[], size_t *n)
{
    const CSNode *c = NULL;

    if (node == NULL) {
        return;
    }

    out[(*n)++] = node->data; /* 根 */
    for (c = node->first_child; c != NULL; c = c->next_sibling) {
        preorder_cs(c, out, n); /* 依次先根遍历每棵子树 */
    }
}

/* 后根遍历一棵树: 全部子树完成后再访问根（整棵树连续） */
static void postorder_cs(const CSNode *node, int out[], size_t *n)
{
    const CSNode *c = NULL;

    if (node == NULL) {
        return;
    }

    for (c = node->first_child; c != NULL; c = c->next_sibling) {
        postorder_cs(c, out, n);
    }
    out[(*n)++] = node->data; /* 根最后 */
}

/* 深拷贝孩子兄弟链（整棵子树含其兄弟链），失败返回 NULL */
static CSNode *copy_cs(const CSNode *src)
{
    CSNode *dst = NULL;

    if (src == NULL) {
        return NULL;
    }

    dst = new_node(src->data);
    if (dst == NULL) {
        return NULL;
    }
    dst->first_child = copy_cs(src->first_child);
    dst->next_sibling = copy_cs(src->next_sibling);
    return dst;
}

/* ---- 森林操作 ---- */

DsResult forest_init(Forest *f)
{
    if (f == NULL) {
        return (DsResult){DS_NULL_PTR, "森林指针为空"};
    }

    f->roots = NULL;
    f->tree_count = 0;
    return (DsResult){DS_OK, "森林初始化成功"};
}

static void free_cs(CSNode *node)
{
    if (node == NULL) {
        return;
    }

    free_cs(node->first_child);
    free_cs(node->next_sibling);
    free(node);
}

DsResult forest_destroy(Forest *f)
{
    if (f == NULL) {
        return (DsResult){DS_NULL_PTR, "森林指针为空"};
    }

    free_cs(f->roots); /* 根链的 next_sibling 串起全部树根，一次递归全释放 */
    f->roots = NULL;
    f->tree_count = 0;
    return (DsResult){DS_OK, "森林销毁成功"};
}

DsResult forest_add_tree(Forest *f, int data, CSNode **root)
{
    CSNode *node = new_node(data);

    if (f == NULL) {
        return (DsResult){DS_NULL_PTR, "森林指针为空"};
    }
    if (node == NULL) {
        return (DsResult){DS_ERROR, "结点分配失败"};
    }

    /* 尾挂根链，保持树序稳定 */
    if (f->roots == NULL) {
        f->roots = node;
    } else {
        CSNode *p = f->roots;

        while (p->next_sibling != NULL) {
            p = p->next_sibling;
        }
        p->next_sibling = node;
    }
    f->tree_count++;
    if (root != NULL) {
        *root = node;
    }
    return (DsResult){DS_OK, "添加树成功"};
}

DsResult forest_attach(Forest *f, CSNode *parent, CSNode *child)
{
    (void)f; /* 孩子数经指纹统计，无需维护额外域 */

    if (parent == NULL || child == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    if (parent->first_child == NULL) {
        parent->first_child = child;
    } else {
        CSNode *p = parent->first_child;

        while (p->next_sibling != NULL) {
            p = p->next_sibling;
        }
        p->next_sibling = child;
    }
    return (DsResult){DS_OK, "挂载子树成功"};
}

size_t forest_count(const Forest *f)
{
    if (f == NULL) {
        return 0;
    }

    /* 根链的 next_sibling 串起全部树，从 roots 起一次统计整片森林 */
    return count_cs(f->roots);
}

size_t forest_tree_count(const Forest *f)
{
    return (f == NULL) ? 0 : f->tree_count;
}

DsResult forest_preorder(const Forest *f, int out[], size_t *n)
{
    const CSNode *root = NULL;

    if (f == NULL || out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *n = 0;
    for (root = f->roots; root != NULL; root = root->next_sibling) {
        preorder_cs(root, out, n);
    }
    return (DsResult){DS_OK, "森林先序遍历完成"};
}

DsResult forest_postorder(const Forest *f, int out[], size_t *n)
{
    const CSNode *root = NULL;

    if (f == NULL || out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *n = 0;
    for (root = f->roots; root != NULL; root = root->next_sibling) {
        postorder_cs(root, out, n);
    }
    return (DsResult){DS_OK, "森林后序遍历完成"};
}

/* ---- 互转 ---- */

DsResult forest_to_bitree(const Forest *f, CSBinaryTree *bt)
{
    const CSNode *root = NULL;
    CSNode *result = NULL;
    CSNode *tail = NULL;

    if (f == NULL || bt == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *bt = NULL;
    if (f->roots == NULL) {
        return (DsResult){DS_OK, "空森林转空二叉树"}; /* 空森林 -> 空二叉树 */
    }

    /* 逐棵复制：第 i+1 棵挂到第 i 棵的右链（next_sibling -> right） */
    for (root = f->roots; root != NULL; root = root->next_sibling) {
        CSNode *copy = copy_cs(root);

        if (copy == NULL) {
            free_cs(result);
            return (DsResult){DS_ERROR, "复制结点分配失败"};
        }
        copy->next_sibling = NULL; /* 复制体作为二叉树右孩子语义，切断原根链 */
        if (result == NULL) {
            result = copy;
        } else {
            tail->next_sibling = copy;
        }
        tail = copy;
    }

    *bt = result;
    return (DsResult){DS_OK, "森林转二叉树成功"};
}

DsResult bitree_to_forest(const CSBinaryTree bt, Forest *f)
{
    const CSNode *root = NULL;

    if (f == NULL) {
        return (DsResult){DS_NULL_PTR, "森林指针为空"};
    }

    forest_init(f);
    if (bt == NULL) {
        return (DsResult){DS_OK, "空二叉树转空森林"};
    }

    /* 沿二叉树根右链拆出每棵树；树内 left/right 语义由 copy 原样搬运 */
    for (root = bt; root != NULL; root = root->next_sibling) {
        CSNode *copy = copy_cs(root);

        if (copy == NULL) {
            forest_destroy(f);
            return (DsResult){DS_ERROR, "复制结点分配失败"};
        }
        copy->next_sibling = NULL;
        if (f->roots == NULL) {
            f->roots = copy;
        } else {
            CSNode *p = f->roots;

            while (p->next_sibling != NULL) {
                p = p->next_sibling;
            }
            p->next_sibling = copy;
        }
        f->tree_count++;
    }
    return (DsResult){DS_OK, "二叉树转森林成功"};
}

DsResult bitree_destroy(CSBinaryTree *bt)
{
    if (bt == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }

    free_cs(*bt);
    *bt = NULL;
    return (DsResult){DS_OK, "销毁成功"};
}

/* ---- 二叉树遍历（先/中序，性质对拍用） ---- */

static void bt_pre_rec(const CSNode *node, int out[], size_t *n)
{
    if (node == NULL) {
        return;
    }

    out[(*n)++] = node->data;
    bt_pre_rec(node->first_child, out, n); /* left */
    bt_pre_rec(node->next_sibling, out, n); /* right */
}

DsResult bt_preorder(const CSBinaryTree bt, int out[], size_t *n)
{
    if (out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *n = 0;
    bt_pre_rec(bt, out, n);
    return (DsResult){DS_OK, "二叉树先序遍历完成"};
}

static void bt_in_rec(const CSNode *node, int out[], size_t *n)
{
    if (node == NULL) {
        return;
    }

    bt_in_rec(node->first_child, out, n);
    out[(*n)++] = node->data;
    bt_in_rec(node->next_sibling, out, n);
}

DsResult bt_inorder(const CSBinaryTree bt, int out[], size_t *n)
{
    if (out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *n = 0;
    bt_in_rec(bt, out, n);
    return (DsResult){DS_OK, "二叉树中序遍历完成"};
}

size_t bt_count(const CSBinaryTree bt)
{
    if (bt == NULL) {
        return 0;
    }
    return 1 + bt_count(bt->first_child) + bt_count(bt->next_sibling);
}

/* ---- 指纹 ---- */

static size_t child_count(const CSNode *node)
{
    size_t c = 0;
    const CSNode *p = NULL;

    for (p = node->first_child; p != NULL; p = p->next_sibling) {
        c++;
    }
    return c;
}

static void fingerprint_cs(const CSNode *node, int datas[], int fanouts[], size_t *n)
{
    if (node == NULL) {
        return;
    }

    datas[*n] = node->data;
    fanouts[*n] = (int)child_count(node);
    (*n)++;
    fingerprint_cs(node->first_child, datas, fanouts, n); /* 先根序: 孩子 */
    fingerprint_cs(node->next_sibling, datas, fanouts, n); /* 后兄弟 */
}

DsResult forest_fingerprint(const Forest *f, int datas[], int fanouts[], size_t *n)
{
    if (f == NULL || datas == NULL || fanouts == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *n = 0;
    fingerprint_cs(f->roots, datas, fanouts, n);
    return (DsResult){DS_OK, "指纹计算成功"};
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        default:             return "未知状态码";
    }
}
