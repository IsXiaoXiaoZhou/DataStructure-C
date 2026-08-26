/**
 * @file seq_binary_tree.c
 * @brief 二叉树的顺序存储（数组实现）接口实现
 *
 * 设计要点:
 *   1. 全部父子定位只依赖编号关系式 2i / 2i+1 / i/2，
 *      不额外存指针 —— 顺序存储的本质
 *   2. 遍历递归以编号为参数，编号超过 last 即空子树
 * 复杂度: 父子定位 O(1)；遍历 O(n)
 */

#include <stdio.h>

#include "seq_binary_tree.h"


DsResult sbt_init(SeqBinaryTree *t)
{
    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }

    t->last = 0;
    return (DsResult){DS_OK, "顺序二叉树初始化成功"};
}

DsResult sbt_insert_level(SeqBinaryTree *t, int data, size_t *idx)
{
    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }
    if (t->last >= SEQ_BT_MAX) {
        return (DsResult){DS_OVERFLOW, "顺序二叉树已满"};
    }

    t->last++;
    t->data[t->last] = data;
    if (idx != NULL) {
        *idx = t->last;
    }
    return (DsResult){DS_OK, "层序追加成功"};
}

size_t sbt_count(const SeqBinaryTree *t)
{
    return (t == NULL) ? 0 : t->last;
}

int sbt_is_empty(const SeqBinaryTree *t)
{
    return (t == NULL) || (t->last == 0);
}

DsResult sbt_parent(const SeqBinaryTree *t, size_t i, size_t *parent)
{
    if (t == NULL || parent == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (i < 1 || i > t->last) {
        return (DsResult){DS_OUT_OF_RANGE, "结点编号越界"};
    }
    if (i == 1) {
        return (DsResult){DS_ERROR, "根结点无双亲"};
    }

    *parent = i / 2; /* 整数除法自动下取整 */
    return (DsResult){DS_OK, "取双亲成功"};
}

DsResult sbt_left_child(const SeqBinaryTree *t, size_t i, size_t *child)
{
    if (t == NULL || child == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (i < 1 || i > t->last) {
        return (DsResult){DS_OUT_OF_RANGE, "结点编号越界"};
    }
    if (2 * i > t->last) {
        return (DsResult){DS_ERROR, "无左孩子"};
    }

    *child = 2 * i;
    return (DsResult){DS_OK, "取左孩子成功"};
}

DsResult sbt_right_child(const SeqBinaryTree *t, size_t i, size_t *child)
{
    if (t == NULL || child == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (i < 1 || i > t->last) {
        return (DsResult){DS_OUT_OF_RANGE, "结点编号越界"};
    }
    if (2 * i + 1 > t->last) {
        return (DsResult){DS_ERROR, "无右孩子"};
    }

    *child = 2 * i + 1;
    return (DsResult){DS_OK, "取右孩子成功"};
}

DsResult sbt_get(const SeqBinaryTree *t, size_t i, int *value)
{
    if (t == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (i < 1 || i > t->last) {
        return (DsResult){DS_OUT_OF_RANGE, "结点编号越界"};
    }

    *value = t->data[i];
    return (DsResult){DS_OK, "取数据成功"};
}

size_t sbt_depth(const SeqBinaryTree *t)
{
    size_t last = 0;
    size_t d = 0;

    if (t == NULL || t->last == 0) {
        return 0;
    }

    /* 深度 = floor(log2(last)) + 1，循环除 2 计算位数 */
    last = t->last;
    while (last > 0) {
        d++;
        last /= 2;
    }
    return d;
}

/* 先序递归辅助: 编号 i 子树 */
static void preorder_at(const SeqBinaryTree *t, size_t i, int out[], size_t *n)
{
    if (i > t->last) {
        return;
    }

    out[(*n)++] = t->data[i];
    preorder_at(t, 2 * i, out, n);
    preorder_at(t, 2 * i + 1, out, n);
}

DsResult sbt_preorder(const SeqBinaryTree *t, int out[], size_t *n)
{
    if (t == NULL || out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *n = 0;
    preorder_at(t, 1, out, n);
    return (DsResult){DS_OK, "先序遍历完成"};
}

/* 中序递归辅助 */
static void inorder_at(const SeqBinaryTree *t, size_t i, int out[], size_t *n)
{
    if (i > t->last) {
        return;
    }

    inorder_at(t, 2 * i, out, n);
    out[(*n)++] = t->data[i];
    inorder_at(t, 2 * i + 1, out, n);
}

DsResult sbt_inorder(const SeqBinaryTree *t, int out[], size_t *n)
{
    if (t == NULL || out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *n = 0;
    inorder_at(t, 1, out, n);
    return (DsResult){DS_OK, "中序遍历完成"};
}

/* 后序递归辅助 */
static void postorder_at(const SeqBinaryTree *t, size_t i, int out[], size_t *n)
{
    if (i > t->last) {
        return;
    }

    postorder_at(t, 2 * i, out, n);
    postorder_at(t, 2 * i + 1, out, n);
    out[(*n)++] = t->data[i];
}

DsResult sbt_postorder(const SeqBinaryTree *t, int out[], size_t *n)
{
    if (t == NULL || out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *n = 0;
    postorder_at(t, 1, out, n);
    return (DsResult){DS_OK, "后序遍历完成"};
}

DsResult sbt_level_order(const SeqBinaryTree *t, int out[], size_t *n)
{
    size_t i = 0;

    if (t == NULL || out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    /* 完全二叉树连续存放: 层序即物理序 */
    *n = 0;
    for (i = 1; i <= t->last; i++) {
        out[(*n)++] = t->data[i];
    }
    return (DsResult){DS_OK, "层序遍历完成"};
}

DsResult sbt_print(const SeqBinaryTree *t)
{
    size_t i = 0;

    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }

    printf("顺序二叉树(%zu 结点, 深度 %zu):\n", t->last, sbt_depth(t));
    for (i = 1; i <= t->last; i++) {
        size_t p = 0;
        size_t l = 0;
        size_t r = 0;
        int has_p = (sbt_parent(t, i, &p).code == DS_OK);
        int has_l = (sbt_left_child(t, i, &l).code == DS_OK);
        int has_r = (sbt_right_child(t, i, &r).code == DS_OK);

        printf("  [%2zu]=%-4d 双亲:%-3zu 左:%-3zu 右:%-3zu\n",
               i, t->data[i],
               has_p ? p : 0,
               has_l ? l : 0,
               has_r ? r : 0);
    }
    return (DsResult){DS_OK, "打印完成"};
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "下标越界";
        case DS_OVERFLOW:    return "容量已满";
        default:             return "未知状态码";
    }
}
