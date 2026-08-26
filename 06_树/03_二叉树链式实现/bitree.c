/**
 * @file bitree.c
 * @brief 二叉树的链式实现接口实现
 *
 * 设计要点:
 *   1. 非递归三序共用"沿左链下降"骨架，差异仅在访问时机:
 *      先序=入栈前访问，中序=出栈时访问，后序=出栈且右已毕时访问
 *   2. 层序用环形数组队列（容量 n+1 留空位区分满/空），
 *      与 03_队列 模块的循环队列同型
 * 复杂度: 全部遍历与统计 O(n)，空间 O(h) 或 O(w)
 */

#include <stdio.h>
#include <stdlib.h>

#include "bitree.h"


/* ---- 建树 ---- */

/* 递归消费数组辅助: *pos 为当前消费位置 */
static BiTNode *build_node(const int arr[], size_t n, size_t *pos)
{
    BiTNode *node = NULL;

    if (*pos >= n || arr[*pos] == BITREE_NULL_FLAG) {
        (*pos)++; /* 空标记也要消费掉，保证兄弟子树位置正确 */
        return NULL;
    }

    node = (BiTNode *)malloc(sizeof(BiTNode));
    if (node == NULL) {
        return NULL;
    }
    node->data = arr[(*pos)++];
    node->left = build_node(arr, n, pos);
    node->right = build_node(arr, n, pos);
    return node;
}

DsResult bitree_create(const int arr[], size_t n, BiTree *t)
{
    size_t pos = 0;

    if (arr == NULL || t == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *t = build_node(arr, n, &pos);
    if (*t == NULL && pos < n && arr[0] != BITREE_NULL_FLAG) {
        return (DsResult){DS_ERROR, "首结点分配失败"};
    }
    if (*t == NULL) {
        /* 首元素即空标记 -> 空树；或根分配失败。区分: pos==1 表示正常空树 */
        return (pos == 1 || n == 0) ? (DsResult){DS_OK, "空树创建成功"}
                                    : (DsResult){DS_ERROR, "建树失败"};
    }
    return (DsResult){DS_OK, "二叉树创建成功"};
}

DsResult bitree_destroy(BiTree *t)
{
    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }
    if (*t == NULL) {
        return (DsResult){DS_OK, "空树无需销毁"};
    }

    /* 后序: 先递归释放两棵子树，再释放自身 */
    bitree_destroy(&(*t)->left);
    bitree_destroy(&(*t)->right);
    free(*t);
    *t = NULL;
    return (DsResult){DS_OK, "销毁成功"};
}

/* ---- 统计 ---- */

size_t bitree_count(BiTree t)
{
    if (t == NULL) {
        return 0;
    }
    return 1 + bitree_count(t->left) + bitree_count(t->right);
}

size_t bitree_leaf_count(BiTree t)
{
    if (t == NULL) {
        return 0;
    }
    if (t->left == NULL && t->right == NULL) {
        return 1; /* 叶子 */
    }
    return bitree_leaf_count(t->left) + bitree_leaf_count(t->right);
}

size_t bitree_depth(BiTree t)
{
    size_t dl = 0;
    size_t dr = 0;

    if (t == NULL) {
        return 0;
    }

    dl = bitree_depth(t->left);
    dr = bitree_depth(t->right);
    return (dl > dr ? dl : dr) + 1;
}

/* ---- 递归遍历 ---- */

/* 先序递归辅助 */
static void pre_rec(const BiTNode *node, int out[], size_t *n)
{
    if (node == NULL) {
        return;
    }

    out[(*n)++] = node->data;
    pre_rec(node->left, out, n);
    pre_rec(node->right, out, n);
}

/* 中序递归辅助 */
static void in_rec(const BiTNode *node, int out[], size_t *n)
{
    if (node == NULL) {
        return;
    }

    in_rec(node->left, out, n);
    out[(*n)++] = node->data;
    in_rec(node->right, out, n);
}

/* 后序递归辅助 */
static void post_rec(const BiTNode *node, int out[], size_t *n)
{
    if (node == NULL) {
        return;
    }

    post_rec(node->left, out, n);
    post_rec(node->right, out, n);
    out[(*n)++] = node->data;
}

DsResult bitree_preorder(BiTree t, int out[], size_t *n)
{
    if (out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    *n = 0;
    pre_rec(t, out, n);
    return (DsResult){DS_OK, "先序遍历完成"};
}

DsResult bitree_inorder(BiTree t, int out[], size_t *n)
{
    if (out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    *n = 0;
    in_rec(t, out, n);
    return (DsResult){DS_OK, "中序遍历完成"};
}

DsResult bitree_postorder(BiTree t, int out[], size_t *n)
{
    if (out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    *n = 0;
    post_rec(t, out, n);
    return (DsResult){DS_OK, "后序遍历完成"};
}

/* ---- 非递归遍历 ---- */

/* 栈元素: 结点指针 */
typedef struct {
    const BiTNode *items[BITREE_MAX_NODES];
    size_t top; /* 栈顶下标，0 表空 */
} NodeStack;

static void stack_push(NodeStack *st, const BiTNode *node)
{
    st->items[st->top++] = node;
}

static const BiTNode *stack_pop(NodeStack *st)
{
    return st->items[--st->top];
}

DsResult bitree_preorder_iter(BiTree t, int out[], size_t *n)
{
    /* static: 栈数组约 800KB，放静态区避免吃爆机器栈（教学模块单线程使用） */
    static NodeStack st;
    const BiTNode *p = t;

    if (out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *n = 0;
    st.top = 0;
    while (p != NULL || st.top > 0) {
        if (p != NULL) {
            out[(*n)++] = p->data; /* 先序: 首次到达即访问 */
            stack_push(&st, p);
            p = p->left;
        } else {
            p = stack_pop(&st);
            p = p->right; /* 转向右子树 */
        }
    }
    return (DsResult){DS_OK, "非递归先序遍历完成"};
}

DsResult bitree_inorder_iter(BiTree t, int out[], size_t *n)
{
    static NodeStack st;
    const BiTNode *p = t;

    if (out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *n = 0;
    st.top = 0;
    while (p != NULL || st.top > 0) {
        if (p != NULL) {
            stack_push(&st, p); /* 暂不访问，沿左链下降 */
            p = p->left;
        } else {
            p = stack_pop(&st);
            out[(*n)++] = p->data; /* 中序: 左子树完毕(弹出)才访问 */
            p = p->right;
        }
    }
    return (DsResult){DS_OK, "非递归中序遍历完成"};
}

DsResult bitree_postorder_iter(BiTree t, int out[], size_t *n)
{
    static NodeStack st;
    const BiTNode *p = t;
    const BiTNode *last_visited = NULL;

    if (out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *n = 0;
    st.top = 0;
    while (p != NULL || st.top > 0) {
        if (p != NULL) {
            stack_push(&st, p);
            p = p->left;
        } else {
            const BiTNode *top = st.items[st.top - 1];

            if (top->right != NULL && top->right != last_visited) {
                p = top->right; /* 右子树未访问: 转右，自身再等等 */
            } else {
                out[(*n)++] = top->data; /* 右子树已毕(或无右): 访问自身 */
                last_visited = top;
                stack_pop(&st);
            }
        }
    }
    return (DsResult){DS_OK, "非递归后序遍历完成"};
}

/* ---- 层序遍历 ---- */

DsResult bitree_level_order(BiTree t, int out[], size_t *n)
{
    /* static: 队列数组约 800KB，放静态区避免吃爆机器栈（教学模块单线程使用） */
    static const BiTNode *queue[BITREE_MAX_NODES];
    size_t head = 0;
    size_t tail = 0;

    if (out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *n = 0;
    if (t == NULL) {
        return (DsResult){DS_OK, "空树层序遍历完成"};
    }

    queue[tail++ % BITREE_MAX_NODES] = t; /* 根入队 */
    while (head < tail) {
        const BiTNode *node = queue[head++ % BITREE_MAX_NODES];

        out[(*n)++] = node->data;
        if (node->left != NULL) {
            queue[tail++ % BITREE_MAX_NODES] = node->left;
        }
        if (node->right != NULL) {
            queue[tail++ % BITREE_MAX_NODES] = node->right;
        }
    }
    return (DsResult){DS_OK, "层序遍历完成"};
}

/* ---- 打印 ---- */

/* 逆时针 90 度: 先右后左打印，缩进随深度增加 */
static void print_rotated(const BiTNode *node, size_t depth)
{
    size_t i = 0;

    if (node == NULL) {
        return;
    }

    print_rotated(node->right, depth + 1);
    for (i = 0; i < depth; i++) {
        printf("      ");
    }
    printf("%d\n", node->data);
    print_rotated(node->left, depth + 1);
}

DsResult bitree_print(BiTree t)
{
    if (t == NULL) {
        printf("(空树)\n");
        return (DsResult){DS_OK, "空树打印完成"};
    }

    print_rotated(t, 0);
    return (DsResult){DS_OK, "打印完成"};
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OVERFLOW:    return "容量不足";
        default:             return "未知状态码";
    }
}
