/**
 * @file btree.c
 * @brief B 树实现（CLRS 风格: 插入先裂后降, 删除保富余下降）
 */

#include <stdlib.h>
#include <string.h>

#include "btree.h"


static BTNode *new_node(int is_leaf)
{
    BTNode *p = malloc(sizeof *p);
    if (p != NULL) {
        p->n = 0;
        p->is_leaf = is_leaf;
    }
    return p;
}

/* 在结点 x 中线性找第一个 keys[i] >= key 的下标（结点小, 线性即可） */
static size_t locate(BTNode *x, int key)
{
    size_t i = 0;
    while (i < x->n && x->keys[i] < key) {
        ++i;
    }
    return i;
}

/* ---------- 插入（自顶向下, 先分裂再下降） ---------- */

/* 分裂 x 的第 i 个满孩子 y 成 y/新 z 两半, 中间键上移给 x */
static void split_child(BTNode *x, size_t i)
{
    BTNode *y = x->child[i];
    BTNode *z = new_node(y->is_leaf);
    size_t j = 0;
    size_t half = BTREE_MAX_KEYS / 2; /* (2t-1)/2 = t-1 键留在 y */

    /* z 拿走 y 的后半键与孩子 */
    for (j = 0; j < half; ++j) {
        z->keys[j] = y->keys[j + BTREE_T];
    }
    z->n = half;
    if (!y->is_leaf) {
        for (j = 0; j <= half; ++j) {
            z->child[j] = y->child[j + BTREE_T];
        }
    }
    y->n = BTREE_T - 1;

    /* 中间键 keys[t-1] 上移到 x, 孩子指针后移 */
    for (j = x->n + 1; j > i + 1; --j) {
        x->child[j] = x->child[j - 1];
    }
    x->child[i + 1] = z;
    for (j = x->n; j > i; --j) {
        x->keys[j] = x->keys[j - 1];
    }
    x->keys[i] = y->keys[BTREE_T - 1];
    x->n++;
}

/* 向一个"未满"结点 x 插入 key（自顶向下递归） */
static DsResult insert_nonfull(BTNode *x, int key)
{
    size_t i = locate(x, key);

    if (i < x->n && x->keys[i] == key) {
        return (DsResult){DS_ERROR, "关键字已存在"}; /* 重复 */
    }
    if (x->is_leaf) {
        size_t j = x->n;
        while (j > i) {
            x->keys[j] = x->keys[j - 1];
            --j;
        }
        x->keys[i] = key;
        x->n++;
        return (DsResult){DS_OK, "插入成功"};
    }
    if (x->child[i]->n == BTREE_MAX_KEYS) { /* 孩子满: 先分裂再下降 */
        split_child(x, i);
        if (x->keys[i] == key) {
            return (DsResult){DS_ERROR, "关键字已存在"};
        }
        if (x->keys[i] < key) {
            ++i;
        }
    }
    return insert_nonfull(x->child[i], key);
}

DsResult btree_insert(BTree *root, int key)
{
    BTree r = NULL;
    BTree s = NULL;

    if (root == NULL) {
        return (DsResult){DS_NULL_PTR, "根指针指针为空"};
    }
    if (*root == NULL) {                   /* 首键建根 */
        r = new_node(1);
        if (r == NULL) {
            return (DsResult){DS_OVERFLOW, "结点内存分配失败"};
        }
        r->keys[0] = key;
        r->n = 1;
        *root = r;
        return (DsResult){DS_OK, "B树创建成功"};
    }
    r = *root;
    if (r->n == BTREE_MAX_KEYS) {          /* 根满: 新根上浮, 树增一层 */
        s = new_node(0);
        if (s == NULL) {
            return (DsResult){DS_OVERFLOW, "结点内存分配失败"};
        }
        s->child[0] = r;
        split_child(s, 0);
        *root = s;
    }
    return insert_nonfull(*root, key);
}

/* ---------- 查找 ---------- */

DsResult btree_search(const BTree root, int key, BTNode **node, size_t *idx)
{
    BTNode *x = root;
    size_t i = 0;

    if (root == NULL) {
        return (DsResult){DS_NOT_FOUND, "树为空"};
    }
    while (x != NULL) {
        i = locate(x, key);
        if (i < x->n && x->keys[i] == key) {
            if (node != NULL) {
                *node = x;
            }
            if (idx != NULL) {
                *idx = i;
            }
            return (DsResult){DS_OK, "查找成功"};
        }
        if (x->is_leaf) {
            return (DsResult){DS_NOT_FOUND, "未找到目标关键字"};
        }
        x = x->child[i];
    }
    return (DsResult){DS_NOT_FOUND, "未找到目标关键字"};
}

/* ---------- 删除（自顶向下, 保证路径孩子富余） ---------- */

/* 把父结点 x 的 keys[idx] 及右孩子并入左孩子 child[idx] */
static void merge_children(BTNode *x, size_t idx)
{
    BTNode *left = x->child[idx];
    BTNode *right = x->child[idx + 1];
    size_t j = 0;

    left->keys[left->n] = x->keys[idx];    /* 父键下沉 */
    for (j = 0; j < right->n; ++j) {       /* 右孩子键续接 */
        left->keys[left->n + 1 + j] = right->keys[j];
    }
    if (!left->is_leaf) {
        for (j = 0; j <= right->n; ++j) {
            left->child[left->n + 1 + j] = right->child[j];
        }
    }
    left->n += right->n + 1;               /* 吞并父键与右孩子 */
    free(right);
    for (j = idx; j < x->n - 1; ++j) {     /* 父结点收拢 */
        x->keys[j] = x->keys[j + 1];
    }
    for (j = idx + 1; j < x->n; ++j) {
        x->child[j] = x->child[j + 1];
    }
    x->n--;
}

/* 保证 x->child[idx] 至少 t 个关键字: 不足则借左/右兄弟或合并 */
static void borrow_or_merge(BTNode *x, size_t idx)
{
    BTNode *c = x->child[idx];

    if (idx > 0 && x->child[idx - 1]->n > BTREE_MIN_KEYS) {
        /* 借左兄弟最右键 -> 父键 -> 孩子最左 */
        BTNode *left = x->child[idx - 1];
        size_t j = c->n;
        while (j > 0) {
            c->keys[j] = c->keys[j - 1];
            --j;
        }
        c->keys[0] = x->keys[idx - 1];
        x->keys[idx - 1] = left->keys[left->n - 1];
        if (!c->is_leaf) {
            for (j = c->n + 1; j > 0; --j) {
                c->child[j] = c->child[j - 1];
            }
            c->child[0] = left->child[left->n];
        }
        c->n++;
        left->n--;
    } else if (idx < x->n && x->child[idx + 1]->n > BTREE_MIN_KEYS) {
        /* 借右兄弟最左键 -> 父键 -> 孩子最右 */
        BTNode *right = x->child[idx + 1];
        c->keys[c->n] = x->keys[idx];
        x->keys[idx] = right->keys[0];
        if (!c->is_leaf) {
            c->child[c->n + 1] = right->child[0];
        }
        c->n++;
        {
            size_t j = 0;
            while (j < right->n - 1) {
                right->keys[j] = right->keys[j + 1];
                ++j;
            }
            if (!right->is_leaf) {
                for (j = 0; j < right->n; ++j) {
                    right->child[j] = right->child[j + 1];
                }
            }
            right->n--;
        }
    } else {
        /* 兄弟都不富余: 与左兄弟(或右兄弟)+父键合并 */
        if (idx > 0) {
            merge_children(x, idx - 1);
        } else {
            merge_children(x, idx);
        }
    }
}

/* 在结点 x 上删除 key（x 保证满足: 非根则 n >= t） */
static DsResult del_rec(BTNode *x, int key)
{
    size_t i = locate(x, key);

    if (i < x->n && x->keys[i] == key) {   /* 命中 */
        if (x->is_leaf) {                  /* 叶: 直接删键 */
            size_t j = i;
            while (j < x->n - 1) {
                x->keys[j] = x->keys[j + 1];
                ++j;
            }
            x->n--;
            return (DsResult){DS_OK, "删除成功"};
        }
        /* 内部结点: 前驱/后继键顶替 */
        if (x->child[i]->n >= BTREE_T) {   /* 左孩子富余: 前驱顶替 */
            BTNode *c = x->child[i];
            while (!c->is_leaf) {
                c = c->child[c->n];
            }
            x->keys[i] = c->keys[c->n - 1];
            return del_rec(x->child[i], x->keys[i]);
        }
        if (x->child[i + 1]->n >= BTREE_T) {/* 右孩子富余: 后继顶替 */
            BTNode *c = x->child[i + 1];
            while (!c->is_leaf) {
                c = c->child[0];
            }
            x->keys[i] = c->keys[0];
            return del_rec(x->child[i + 1], x->keys[i]);
        }
        /* 两孩子都不足: 合并后递归删 */
        merge_children(x, i);
        return del_rec(x->child[i], key);
    }

    /* 未命中: 进入 child[i] 前保证其富余 */
    if (x->is_leaf) {
        return (DsResult){DS_NOT_FOUND, "未找到目标关键字"};
    }
    if (x->child[i]->n == BTREE_MIN_KEYS) {
        borrow_or_merge(x, i);
        i = locate(x, key);
    }
    return del_rec(x->child[i], key);
}

DsResult btree_delete(BTree *root, int key)
{
    BTree r = NULL;

    if (root == NULL) {
        return (DsResult){DS_NULL_PTR, "根指针指针为空"};
    }
    r = *root;
    if (r == NULL) {
        return (DsResult){DS_NOT_FOUND, "树为空"};
    }
    if (r->n == 1 && r->is_leaf) {         /* 单键根: 直接清零 */
        if (r->keys[0] == key) {
            r->n = 0;
            free(r);
            *root = NULL;
            return (DsResult){DS_OK, "删除成功"};
        }
        return (DsResult){DS_NOT_FOUND, "未找到目标关键字"};
    }
    {
        DsResult st = del_rec(r, key);
        /* 删除引起根键数为 0: 把唯一孩子提升为新根（树减一层） */
        if (st.code == DS_OK && r->n == 0) {
            BTree child = r->child[0];
            free(r);
            *root = child;
        }
        return st;
    }
}

/* ---------- 统计/高度/遍历/销毁 ---------- */

size_t btree_key_count(const BTree root)
{
    size_t i = 0;
    size_t cnt = 0;

    if (root == NULL) {
        return 0;
    }
    cnt = root->n;
    if (!root->is_leaf) {
        for (i = 0; i <= root->n; ++i) {
            cnt += btree_key_count(root->child[i]);
        }
    }
    return cnt;
}

size_t btree_height(const BTree root)
{
    size_t h = 0;
    BTree x = root;

    while (x != NULL) {
        ++h;
        x = x->is_leaf ? NULL : x->child[0];
    }
    return h;
}

static void visit_rec(const BTNode *x, BTreeVisit visit)
{
    size_t i = 0;
    if (x == NULL) {
        return;
    }
    for (i = 0; i < x->n; ++i) {
        if (!x->is_leaf) {
            visit_rec(x->child[i], visit);
        }
        visit(x->keys[i]);
    }
    if (!x->is_leaf) {
        visit_rec(x->child[x->n], visit);
    }
}

DsResult btree_inorder(const BTree root, BTreeVisit visit)
{
    if (visit == NULL) {
        return (DsResult){DS_NULL_PTR, "回调函数为空"};
    }
    visit_rec(root, visit);
    return (DsResult){DS_OK, "中序遍历完成"};
}

static void destroy_rec(BTNode *x)
{
    size_t i = 0;
    if (x == NULL) {
        return;
    }
    if (!x->is_leaf) {
        for (i = 0; i <= x->n; ++i) {
            destroy_rec(x->child[i]);
        }
    }
    free(x);
}

DsResult btree_destroy(BTree *root)
{
    if (root == NULL) {
        return (DsResult){DS_NULL_PTR, "根指针指针为空"};
    }
    destroy_rec(*root);
    *root = NULL;
    return (DsResult){DS_OK, "B树销毁成功"};
}

/* ---------- 结构自检 ---------- */

static void verify_rec(const BTNode *x, int depth, long *leaf_depth,
                       size_t *viol)
{
    size_t i = 0;

    if (x == NULL) {
        (*viol)++;
        return;
    }
    if (x->n == 0 || x->n > BTREE_MAX_KEYS) {
        (*viol)++;
        return;
    }
    if (!x->is_leaf && x->n < BTREE_MIN_KEYS && depth != 0) {
        (*viol)++;
    }
    for (i = 0; i + 1 < x->n; ++i) {
        if (x->keys[i] >= x->keys[i + 1]) {
            (*viol)++;
        }
    }
    if (x->is_leaf) {
        if (*leaf_depth < 0) {
            *leaf_depth = depth;
        } else if (*leaf_depth != depth) {
            (*viol)++;
        }
        return;
    }
    for (i = 0; i <= x->n; ++i) {
        verify_rec(x->child[i], depth + 1, leaf_depth, viol);
    }
}

DsResult btree_verify(const BTree root, size_t *violations)
{
    size_t cnt = 0;
    long leaf_depth = -1;

    if (root == NULL) {
        return (DsResult){DS_EMPTY, "B树为空"};
    }
    verify_rec(root, 0, &leaf_depth, &cnt);
    if (violations != NULL) {
        *violations = cnt;
    }
    return cnt == 0 ? (DsResult){DS_OK, "B树结构验证通过"}
                    : (DsResult){DS_ERROR, "B树结构验证失败"};
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "位置/下标越界";
        case DS_OVERFLOW:    return "内存分配失败";
        case DS_EMPTY:       return "B树为空";
        case DS_NOT_FOUND:   return "未找到目标关键字";
        default:             return "未知状态码";
    }
}
