/**
 * @file thread_tree.c
 * @brief 线索二叉树（中序线索化）接口实现
 *
 * 设计要点:
 *   1. 线索化与中序递归遍历同构，仅多两步"挂线索"；
 *      pre 指针经参数传递（避免全局变量，多次线索化安全）
 *   2. destroy 只走 tag==0 分支 —— 线索会把树连成环，
 *      必须靠 tag 区分"结构边"与"线索边"
 * 复杂度: 线索化/遍历/销毁均 O(n)；遍历空间 O(1)
 */

#include <stdlib.h>

#include "thread_tree.h"


/* ---- 建树 ---- */

static ThreadNode *build_node(const int arr[], size_t n, size_t *pos)
{
    ThreadNode *node = NULL;

    if (*pos >= n || arr[*pos] == THREAD_NULL_FLAG) {
        (*pos)++;
        return NULL;
    }

    node = (ThreadNode *)malloc(sizeof(ThreadNode));
    if (node == NULL) {
        return NULL;
    }
    node->data = arr[(*pos)++];
    node->ltag = CHILD_PTR;
    node->rtag = CHILD_PTR;
    node->left = build_node(arr, n, pos);
    node->right = build_node(arr, n, pos);
    return node;
}

DsResult tt_create(const int arr[], size_t n, ThreadTree *t)
{
    size_t pos = 0;

    if (arr == NULL || t == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *t = build_node(arr, n, &pos);
    if (*t == NULL && n > 0 && arr[0] != THREAD_NULL_FLAG) {
        return (DsResult){DS_ERROR, "建树分配失败"};
    }
    return (DsResult){DS_OK, "线索树创建成功"};
}

/* ---- 线索化 ---- */

/* 中序线索化递归核心: pre 为当前中序序列中的直接前驱 */
static void thread_visit(ThreadNode *p, ThreadNode **pre)
{
    if (p->left == NULL) {
        p->ltag = THREAD_PTR;
        p->left = *pre; /* 左空 -> 前驱线索（首结点挂 NULL） */
    }
    if (*pre != NULL && (*pre)->right == NULL) {
        (*pre)->rtag = THREAD_PTR;
        (*pre)->right = p; /* 前驱右空 -> 指向我的后继线索 */
    }
    *pre = p;
}

static void in_thread(ThreadNode *p, ThreadNode **pre)
{
    if (p == NULL) {
        return;
    }

    in_thread(p->left, pre); /* 仅沿孩子指针下降（此时尚无线索） */
    thread_visit(p, pre);
    in_thread(p->right, pre);
}

DsResult tt_inthreading(ThreadTree t)
{
    ThreadNode *pre = NULL;

    if (t == NULL) {
        return (DsResult){DS_OK, "空树线索化为空操作"}; /* 空树线索化为空操作，合法 */
    }

    in_thread(t, &pre);
    if (pre != NULL) {
        pre->rtag = THREAD_PTR; /* 中序末结点: 后继线索置空 */
        pre->right = NULL;
    }
    return (DsResult){DS_OK, "中序线索化成功"};
}

/* ---- 前驱后继 ---- */

ThreadNode *tt_first(ThreadTree t)
{
    if (t == NULL) {
        return NULL;
    }

    /* 最左结点: 沿左孩子（tag==0）走到头 */
    while (t->ltag == CHILD_PTR) {
        t = t->left;
    }
    return t;
}

ThreadNode *tt_last(ThreadTree t)
{
    if (t == NULL) {
        return NULL;
    }

    while (t->rtag == CHILD_PTR) {
        t = t->right;
    }
    return t;
}

ThreadNode *tt_next(const ThreadNode *node)
{
    if (node == NULL) {
        return NULL;
    }

    if (node->rtag == THREAD_PTR) {
        return node->right; /* 后继线索直达（末结点返回 NULL） */
    }
    /* 有右孩子: 后继是右子树的最左结点 */
    {
        ThreadNode *p = node->right;

        while (p->ltag == CHILD_PTR) {
            p = p->left;
        }
        return p;
    }
}

ThreadNode *tt_prev(const ThreadNode *node)
{
    if (node == NULL) {
        return NULL;
    }

    if (node->ltag == THREAD_PTR) {
        return node->left; /* 前驱线索直达（首结点返回 NULL） */
    }
    /* 有左孩子: 前驱是左子树的最右结点 */
    {
        ThreadNode *p = node->left;

        while (p->rtag == CHILD_PTR) {
            p = p->right;
        }
        return p;
    }
}

/* ---- 线索遍历 ---- */

DsResult tt_inorder(ThreadTree t, int out[], size_t *n)
{
    ThreadNode *p = NULL;

    if (out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *n = 0;
    for (p = tt_first(t); p != NULL; p = tt_next(p)) {
        out[(*n)++] = p->data;
    }
    return (DsResult){DS_OK, "线索中序遍历完成"};
}

DsResult tt_inorder_reverse(ThreadTree t, int out[], size_t *n)
{
    ThreadNode *p = NULL;

    if (out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *n = 0;
    for (p = tt_last(t); p != NULL; p = tt_prev(p)) {
        out[(*n)++] = p->data;
    }
    return (DsResult){DS_OK, "线索逆中序遍历完成"};
}

/* ---- 销毁 ---- */

static void free_threaded(ThreadNode *node)
{
    if (node == NULL) {
        return;
    }

    /*
     * 只沿结构边（tag==0）递归: 线索边会把中序相邻结点
     * 连成环，跟随会死循环甚至 double free
     */
    if (node->ltag == CHILD_PTR) {
        free_threaded(node->left);
    }
    if (node->rtag == CHILD_PTR) {
        free_threaded(node->right);
    }
    free(node);
}

DsResult tt_destroy(ThreadTree *t)
{
    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }

    free_threaded(*t);
    *t = NULL;
    return (DsResult){DS_OK, "销毁成功"};
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
