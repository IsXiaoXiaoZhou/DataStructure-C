/**
 * @file avl.c
 * @brief 平衡二叉树实现（四旋转 + 插入删除自平衡）
 */

#include <stdio.h>
#include <stdlib.h>

#include "avl.h"


/* ---------- 高度/旋转基础 ---------- */

static int h(AVLNode *p)
{
    return p == NULL ? 0 : p->height;
}

static int max2(int a, int b)
{
    return a > b ? a : b;
}

static void upd_height(AVLNode *p)
{
    p->height = 1 + max2(h(p->left), h(p->right));
}

static AVLNode *new_node(int key)
{
    AVLNode *p = malloc(sizeof *p);
    if (p != NULL) {
        p->data = key;
        p->height = 1;
        p->left = NULL;
        p->right = NULL;
    }
    return p;
}

/* LL 形态: 右单旋。x 的左孩子 y 升为根, x 挂到 y 右侧 */
static AVLNode *rot_right(AVLNode *x)
{
    AVLNode *y = x->left;
    x->left = y->right;
    y->right = x;
    upd_height(x);
    upd_height(y);
    return y;
}

/* RR 形态: 左单旋。x 的右孩子 y 升为根, x 挂到 y 左侧 */
static AVLNode *rot_left(AVLNode *x)
{
    AVLNode *y = x->right;
    x->right = y->left;
    y->left = x;
    upd_height(x);
    upd_height(y);
    return y;
}

/* 针对失衡结点 x 按四种形态做平衡调整，返回新子树根 */
static AVLNode *rebalance(AVLNode *x)
{
    int bf = h(x->left) - h(x->right);

    if (bf > 1) {                         /* 左重: LL 或 LR */
        if (h(x->left->left) >= h(x->left->right)) {
            return rot_right(x);          /* LL: 直接右旋 */
        }
        x->left = rot_left(x->left);      /* LR: 左旋变 LL 再右旋 */
        return rot_right(x);
    }
    if (bf < -1) {                        /* 右重: RR 或 RL */
        if (h(x->right->right) >= h(x->right->left)) {
            return rot_left(x);           /* RR: 直接左旋 */
        }
        x->right = rot_right(x->right);   /* RL: 右旋变 RR 再左旋 */
        return rot_left(x);
    }
    return x;
}

/* ---------- 插入 ---------- */

static AVLNode *insert_rec(AVLNode *p, int key, DsStatus *st)
{
    if (p == NULL) {
        p = new_node(key);
        if (p == NULL) {
            *st = DS_OVERFLOW;
        }
        return p;
    }
    if (key < p->data) {
        p->left = insert_rec(p->left, key, st);
    } else if (key > p->data) {
        p->right = insert_rec(p->right, key, st);
    } else {
        *st = DS_ERROR;                  /* 重复, 结构不变 */
        return p;
    }
    upd_height(p);
    if (*st == DS_OK) {
        p = rebalance(p);
    }
    return p;
}

DsResult avl_insert(AVLTree *root, int key)
{
    DsStatus st = DS_OK;

    if (root == NULL) {
        return (DsResult){DS_NULL_PTR, "根指针指针为空"};
    }
    *root = insert_rec(*root, key, &st);
    return (st == DS_OK) ? (DsResult){DS_OK, "插入成功"}
         : (st == DS_ERROR) ? (DsResult){DS_ERROR, "关键字已存在，不允许重复插入"}
         : (DsResult){DS_OVERFLOW, "结点内存分配失败"};
}

/* ---------- 删除 ---------- */

static AVLNode *del_rec(AVLNode *p, int key, DsStatus *st)
{
    AVLNode *child = NULL;
    AVLNode *old = NULL;
    AVLNode *prev = NULL;

    if (p == NULL) {
        *st = DS_ERROR;                  /* 未命中 */
        return NULL;
    }
    if (key < p->data) {
        p->left = del_rec(p->left, key, st);
    } else if (key > p->data) {
        p->right = del_rec(p->right, key, st);
    } else {
        /* 命中: 双孩子用中序前驱（左子树最右）值替换，删前驱 */
        if (p->left != NULL && p->right != NULL) {
            prev = p->left;
            while (prev->right != NULL) {
                prev = prev->right;
            }
            p->data = prev->data;
            p->left = del_rec(p->left, prev->data, st);
        } else {
            child = (p->left != NULL) ? p->left : p->right;
            old = p;
            p = child;                   /* 孩子顶上（可能为 NULL） */
            free(old);
        }
    }
    if (p == NULL) {
        return NULL;
    }
    upd_height(p);
    if (*st == DS_OK) {                  /* 确实删掉了才可能失衡 */
        p = rebalance(p);
    }
    return p;
}

DsResult avl_delete(AVLTree *root, int key)
{
    DsStatus st = DS_OK;

    if (root == NULL) {
        return (DsResult){DS_NULL_PTR, "根指针指针为空"};
    }
    *root = del_rec(*root, key, &st);
    return (st == DS_OK) ? (DsResult){DS_OK, "删除成功"}
                         : (DsResult){DS_NOT_FOUND, "未找到目标关键字"};
}

/* ---------- 查找/统计/遍历 ---------- */

DsResult avl_search(AVLTree root, int key, AVLNode **out)
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

DsResult avl_clear(AVLTree *root)
{
    if (root == NULL) {
        return (DsResult){DS_NULL_PTR, "根指针指针为空"};
    }
    if (*root == NULL) {
        return (DsResult){DS_OK, "已是空树"};
    }
    (void)avl_clear(&(*root)->left);
    (void)avl_clear(&(*root)->right);
    free(*root);
    *root = NULL;
    return (DsResult){DS_OK, "清空成功"};
}

DsResult avl_destroy(AVLTree *root)
{
    return avl_clear(root);
}

size_t avl_count(AVLTree root)
{
    if (root == NULL) {
        return 0;
    }
    return 1 + avl_count(root->left) + avl_count(root->right);
}

size_t avl_height(AVLTree root)
{
    return root == NULL ? 0 : (size_t)root->height;
}

DsResult avl_inorder(AVLTree root, AVLVisit visit)
{
    if (visit == NULL) {
        return (DsResult){DS_NULL_PTR, "回调函数为空"};
    }
    if (root == NULL) {
        return (DsResult){DS_OK, "空树遍历完成"};
    }
    (void)avl_inorder(root->left, visit);
    visit(root->data);
    (void)avl_inorder(root->right, visit);
    return (DsResult){DS_OK, "中序遍历完成"};
}

/* ---------- 结构自检验证器 ---------- */

/* 返回真实的高度；违规计数入参累加 */
static int verify_rec(AVLNode *p, size_t *violations)
{
    int hl = 0;
    int hr = 0;
    int hh = 0;

    if (p == NULL) {
        return 0;
    }
    hl = verify_rec(p->left, violations);
    hr = verify_rec(p->right, violations);
    if (hl - hr > 1 || hr - hl > 1) {           /* BF 越界 */
        (*violations)++;
    }
    hh = 1 + max2(hl, hr);
    if (p->height != hh) {                      /* 缓存高度与真实不符 */
        (*violations)++;
    }
    return hh;
}

DsResult avl_verify(AVLTree root, size_t *violations)
{
    size_t cnt = 0;

    if (violations != NULL) {
        *violations = 0;
    }
    (void)verify_rec(root, &cnt);
    if (violations != NULL) {
        *violations = cnt;
    }
    return cnt == 0 ? (DsResult){DS_OK, "AVL结构验证通过"}
                    : (DsResult){DS_ERROR, "AVL结构验证失败"};
}

/* ---------- 树形打印 ---------- */

static void print_rec(AVLNode *p, int depth)
{
    int d = depth;

    if (p == NULL) {
        return;
    }
    if (p->right != NULL) {
        print_rec(p->right, depth + 1);
    }
    while (d-- > 0) {
        printf("    ");
    }
    printf("%d\n", p->data);
    if (p->left != NULL) {
        print_rec(p->left, depth + 1);
    }
}

DsResult avl_print(AVLTree root)
{
    if (root == NULL) {
        return (DsResult){DS_NULL_PTR, "树为空"};
    }
    print_rec(root, 0);
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
