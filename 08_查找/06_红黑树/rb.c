/**
 * @file rb.c
 * @brief 红黑树实现（CLRS 风格: 新结点染红、插入叔红变色叔黑旋转、
 *        删除双黑修复），NULL 即黑色叶
 */

#include <stdlib.h>

#include "rb.h"


/* ---------- 基础工具 ---------- */

void rb_init(RBTree *t)
{
    if (t != NULL) {
        t->root = NULL;
    }
}

static RBNode *new_node(int key)
{
    RBNode *p = malloc(sizeof *p);
    if (p != NULL) {
        p->data = key;
        p->color = RB_RED;      /* 新结点必红: 不破坏黑高 */
        p->left = NULL;
        p->right = NULL;
        p->parent = NULL;
    }
    return p;
}

static RBNode *rb_find(const RBTree *t, int key)
{
    RBNode *x = t->root;
    while (x != NULL) {
        if (key == x->data) {
            return x;
        }
        x = (key < x->data) ? x->left : x->right;
    }
    return NULL;
}

/* 左旋（以 x 为支点, 其右孩子 y 上移） */
static void rot_left(RBTree *t, RBNode *x)
{
    RBNode *y = x->right;
    x->right = y->left;
    if (y->left != NULL) {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == NULL) {
        t->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

/* 右旋（对称） */
static void rot_right(RBTree *t, RBNode *x)
{
    RBNode *y = x->left;
    x->left = y->right;
    if (y->right != NULL) {
        y->right->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == NULL) {
        t->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->right = x;
    x->parent = y;
}

/* ---------- 插入 ---------- */

/* 新结点 z 染红可能破坏性质2(根红)/4(父红): 父红叔红→变色上移,
   父红叔黑→旋转（与 AVL 的四种形态同构） */
static void insert_fixup(RBTree *t, RBNode *z)
{
    while (z != t->root && z->parent->color == RB_RED) {
        RBNode *g = z->parent->parent;   /* 父红 ⇒ 父非根 ⇒ 祖父必在 */
        if (z->parent == g->left) {
            RBNode *uncle = g->right;
            if (uncle != NULL && uncle->color == RB_RED) {
                z->parent->color = RB_BLACK;  /* 叔红: 父叔变黑、祖父变红, 上移 */
                uncle->color = RB_BLACK;
                g->color = RB_RED;
                z = g;
            } else {
                if (z == z->parent->right) {  /* LR 折线: 先左旋成 LL 直线 */
                    z = z->parent;
                    rot_left(t, z);
                }
                z->parent->color = RB_BLACK;  /* LL: 父染黑、祖父染红、右旋 */
                g->color = RB_RED;
                rot_right(t, g);
            }
        } else {                              /* 对称: parent 是右孩子 */
            RBNode *uncle = g->left;
            if (uncle != NULL && uncle->color == RB_RED) {
                z->parent->color = RB_BLACK;
                uncle->color = RB_BLACK;
                g->color = RB_RED;
                z = g;
            } else {
                if (z == z->parent->left) {   /* RL 折线: 先右旋成 RR 直线 */
                    z = z->parent;
                    rot_right(t, z);
                }
                z->parent->color = RB_BLACK;  /* RR: 父染黑、祖父染红、左旋 */
                g->color = RB_RED;
                rot_left(t, g);
            }
        }
    }
    t->root->color = RB_BLACK;                /* 性质2: 根恒黑 */
}

DsResult rb_insert(RBTree *t, int key)
{
    RBNode *z = NULL;
    RBNode *y = NULL;
    RBNode *x = NULL;

    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }
    if (rb_find(t, key) != NULL) {
        return (DsResult){DS_ERROR, "关键字已存在，不允许重复插入"};
    }
    z = new_node(key);
    if (z == NULL) {
        return (DsResult){DS_OVERFLOW, "结点内存分配失败"};
    }
    y = NULL;
    x = t->root;
    while (x != NULL) {                      /* 常规 BST 插叶 */
        y = x;
        x = (key < x->data) ? x->left : x->right;
    }
    z->parent = y;
    if (y == NULL) {
        t->root = z;
    } else if (key < y->data) {
        y->left = z;
    } else {
        y->right = z;
    }
    insert_fixup(t, z);
    return (DsResult){DS_OK, "插入成功"};
}

/* ---------- 删除 ---------- */

/* 用 v 顶替 u（不释放 u） */
static void transplant(RBTree *t, RBNode *u, RBNode *v)
{
    if (u->parent == NULL) {
        t->root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    if (v != NULL) {
        v->parent = u->parent;
    }
}

/* 双黑修复: x(double-black) 与它的父 xp */
static void delete_fixup(RBTree *t, RBNode *x, RBNode *xp)
{
    while (x != t->root && (x == NULL || x->color == RB_BLACK)) {
        if (x == xp->left) {
            RBNode *w = xp->right;           /* 兄弟 */
            if (w == NULL) {
                x = xp;
                xp = xp == NULL ? NULL : xp->parent;
                continue;
            }
            if (w->color == RB_RED) {        /* 情况1: 兄弟红 → 染黑+左旋 */
                w->color = RB_BLACK;
                xp->color = RB_RED;
                rot_left(t, xp);
                w = xp->right;
            }
            if ((w->left == NULL || w->left->color == RB_BLACK) &&
                (w->right == NULL || w->right->color == RB_BLACK)) { /* 情况2 */
                w->color = RB_RED;           /* 兄弟双黑子: 兄弟染红, 双黑上移 */
                x = xp;
                xp = xp->parent;
            } else {
                if (w->right == NULL || w->right->color == RB_BLACK) {
                    /* 情况3: 兄弟左红右黑 → 先右旋兄弟成情况4 */
                    if (w->left != NULL) {
                        w->left->color = RB_BLACK;
                    }
                    w->color = RB_RED;
                    rot_right(t, w);
                    w = xp->right;
                }
                /* 情况4: 兄弟右红 → 换色+左旋, 双黑消除 */
                w->color = xp->color;
                xp->color = RB_BLACK;
                if (w->right != NULL) {
                    w->right->color = RB_BLACK;
                }
                rot_left(t, xp);
                x = t->root;
            }
        } else {                             /* 对称: x 是右孩子 */
            RBNode *w = xp->left;
            if (w == NULL) {
                x = xp;
                xp = xp == NULL ? NULL : xp->parent;
                continue;
            }
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                xp->color = RB_RED;
                rot_right(t, xp);
                w = xp->left;
            }
            if ((w->left == NULL || w->left->color == RB_BLACK) &&
                (w->right == NULL || w->right->color == RB_BLACK)) {
                w->color = RB_RED;
                x = xp;
                xp = xp->parent;
            } else {
                if (w->left == NULL || w->left->color == RB_BLACK) {
                    if (w->right != NULL) {
                        w->right->color = RB_BLACK;
                    }
                    w->color = RB_RED;
                    rot_left(t, w);
                    w = xp->left;
                }
                w->color = xp->color;
                xp->color = RB_BLACK;
                if (w->left != NULL) {
                    w->left->color = RB_BLACK;
                }
                rot_right(t, xp);
                x = t->root;
            }
        }
    }
    if (x != NULL) {
        x->color = RB_BLACK;
    }
}

DsResult rb_delete(RBTree *t, int key)
{
    RBNode *z = NULL;
    RBNode *x = NULL;
    RBNode *xp = NULL;
    RBColor zc = RB_BLACK;

    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }
    z = rb_find(t, key);
    if (z == NULL) {
        return (DsResult){DS_NOT_FOUND, "未找到目标关键字"};
    }

    /* 双孩子: 用中序前驱(左子树最右)的值覆盖, 转删前驱 */
    if (z->left != NULL && z->right != NULL) {
        RBNode *pred = z->left;
        while (pred->right != NULL) {
            pred = pred->right;
        }
        z->data = pred->data;      /* 值覆盖: 红黑性质只看结构, 与值无关 */
        z = pred;                  /* 目标换成前驱: 前驱必无右孩子 */
    }

    /* z 至多一个孩子: 孩子 x 顶替, 记录其父 xp */
    x = (z->left != NULL) ? z->left : z->right;
    xp = z->parent;
    zc = z->color;
    transplant(t, z, x);
    free(z);

    if (zc == RB_BLACK && xp != NULL) {
        delete_fixup(t, x, xp);
    } else if (zc == RB_BLACK && xp == NULL && x != NULL) {
        x->color = RB_BLACK;       /* 删的是根: 新根染黑 */
    }
    return (DsResult){DS_OK, "删除成功"};
}

/* ---------- 查询/统计/遍历 ---------- */

DsResult rb_search(const RBTree *t, int key, RBNode **out)
{
    RBNode *z = NULL;

    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }
    z = rb_find(t, key);
    if (out != NULL) {
        *out = z;
    }
    return z == NULL ? (DsResult){DS_NOT_FOUND, "未找到目标关键字"}
                     : (DsResult){DS_OK, "查找成功"};
}

static void free_rec(RBNode *n)
{
    if (n == NULL) {
        return;
    }
    free_rec(n->left);
    free_rec(n->right);
    free(n);
}

DsResult rb_clear(RBTree *t)
{
    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }
    free_rec(t->root);
    t->root = NULL;
    return (DsResult){DS_OK, "清空成功"};
}

DsResult rb_destroy(RBTree *t)
{
    return rb_clear(t);
}

static size_t count_rec(const RBNode *n)
{
    if (n == NULL) {
        return 0;
    }
    return 1 + count_rec(n->left) + count_rec(n->right);
}

size_t rb_count(const RBTree *t)
{
    if (t == NULL) {
        return 0;
    }
    return count_rec(t->root);
}

static size_t height_rec(const RBNode *n)
{
    size_t hl = 0;
    size_t hr = 0;

    if (n == NULL) {
        return 0;
    }
    hl = height_rec(n->left);
    hr = height_rec(n->right);
    return (hl > hr ? hl : hr) + 1;
}

size_t rb_height(const RBTree *t)
{
    if (t == NULL) {
        return 0;
    }
    return height_rec(t->root);
}

static void inorder_rec(const RBNode *n, void (*visit)(int))
{
    if (n == NULL) {
        return;
    }
    inorder_rec(n->left, visit);
    visit(n->data);
    inorder_rec(n->right, visit);
}

DsResult rb_inorder(const RBTree *t, void (*visit)(int value))
{
    if (t == NULL || visit == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    inorder_rec(t->root, visit);
    return (DsResult){DS_OK, "中序遍历完成"};
}

/* ---------- 结构自检验证器 ---------- */

static size_t verify_rec(const RBNode *n, const RBNode *parent,
                         size_t depth, size_t *viol)
{
    size_t bl = 0;
    size_t br = 0;
    size_t bh = 0;

    if (depth > 5000) {
        (*viol) += 1000;
        return 1;
    }
    if (n == NULL) {
        return 1;                        /* 空叶算黑高 1 */
    }
    if (n->parent != parent) {
        (*viol)++;
    }
    if (parent != NULL &&
        n != parent->left && n != parent->right) {
        (*viol)++;
    }
    if (n->color == RB_RED && parent != NULL && parent->color == RB_RED) {
        (*viol)++;
    }
    bl = verify_rec(n->left, n, depth + 1, viol);
    br = verify_rec(n->right, n, depth + 1, viol);
    if (bl != br) {
        (*viol)++;
    }
    bh = n->color == RB_BLACK ? bl + 1 : bl;
    return bh;
}

DsResult rb_verify(const RBTree *t, size_t *violations)
{
    size_t cnt = 0;

    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }
    if (t->root != NULL && t->root->color != RB_BLACK) {
        cnt++;                           /* 性质2: 根黑 */
    }
    if (t->root != NULL && t->root->parent != NULL) {
        cnt++;                           /* 根的 parent 必须为 NULL */
    }
    (void)verify_rec(t->root, NULL, 0, &cnt);
    if (violations != NULL) {
        *violations = cnt;
    }
    return cnt == 0 ? (DsResult){DS_OK, "红黑树结构验证通过"}
                    : (DsResult){DS_ERROR, "红黑树结构验证失败"};
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
