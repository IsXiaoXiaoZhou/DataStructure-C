/**
 * @file bplus.c
 * @brief B+ 树实现（叶链 + 范围查询，插入/删除见头文件说明）
 */

#include <stdlib.h>

#include "bplus.h"

#ifdef BPT_LEAK_CHECK
/* 泄漏检查计数 wrapper（编译加 -DBPT_LEAK_CHECK 启用，见 bplus.h 说明）。
 * wrapper 定义处宏尚未生效，体内调用真实 malloc/free；
 * 定义完成后重定向，后续本模块全部 malloc/free 均被计数 */
static size_t bpt_lc_cnt_in = 0;
static size_t bpt_lc_cnt_out = 0;

void *bpt_lc_malloc(size_t size)
{
    void *p = malloc(size);
    if (p != NULL) {
        ++bpt_lc_cnt_in;
    }
    return p;
}

void bpt_lc_free(void *p)
{
    if (p != NULL) {
        ++bpt_lc_cnt_out;
    }
    free(p);
}

size_t bpt_lc_in(void)
{
    return bpt_lc_cnt_in;
}

size_t bpt_lc_out(void)
{
    return bpt_lc_cnt_out;
}

#define malloc bpt_lc_malloc
#define free   bpt_lc_free
#endif


/* ========== 内部工具 ========== */

static BPTNode *bp_new(int leaf)
{
    BPTNode *p = malloc(sizeof *p);
    if (p != NULL) {
        p->n = 0;
        p->leaf = leaf;
        p->parent = NULL;
        if (leaf) {
            p->ch[0] = NULL;              /* 叶: ch[0] = next 链指针 */
        }
    }
    return p;
}

/* 叶的 next 指针宏 (复用 ch[0]) */
#define LEAF_NEXT(nd)  ((nd)->ch[0])
#define LEAF_SET_NEXT(nd, v) ((nd)->ch[0] = (struct BPTNode *)(v))

/* 左most 叶 */
static BPTNode *leftmost(BPTNode *x)
{
    while (x != NULL && !x->leaf) {
        x = x->ch[0];
    }
    return x;
}

/* 内部/叶结点内定位: 第一个 keys[i] >= key 的下标 */
static int locate(const BPTNode *x, int key)
{
    int i = 0;
    while (i < x->n && x->keys[i] < key) {
        ++i;
    }
    return i;
}

/* 向叶结点有序插入 (假设未满) */
static void leaf_insert(BPTNode *nd, int key)
{
    int i = nd->n;
    while (i > 0 && nd->keys[i - 1] > key) {
        nd->keys[i] = nd->keys[i - 1];
        --i;
    }
    nd->keys[i] = key;
    nd->n++;
}

/* ========== 叶分裂 ========== */

static int split_leaf(BPTNode *nd, BPTNode **out_right)
{
    BPTNode *right = bp_new(1);
    int i = 0;

    nd->n = BPT_T;
    right->n = BPT_MAX - BPT_T;
    for (i = 0; i < right->n; ++i) {
        right->keys[i] = nd->keys[BPT_T + i];
    }
    right->parent = nd->parent;
    LEAF_SET_NEXT(right, LEAF_NEXT(nd));
    LEAF_SET_NEXT(nd, right);
    *out_right = right;
    return right->keys[0];
}

/* ========== 内部分裂 ========== */

static int split_inner(BPTNode *nd, BPTNode **out_right)
{
    BPTNode *right = bp_new(0);
    int mid = nd->keys[BPT_T];
    int i = 0;

    nd->n = BPT_T;
    right->n = BPT_T - 1;
    for (i = 0; i < right->n; ++i) {
        right->keys[i] = nd->keys[BPT_T + 1 + i];
    }
    for (i = 0; i <= right->n; ++i) {
        right->ch[i] = nd->ch[BPT_T + 1 + i];
        right->ch[i]->parent = right;
    }
    right->parent = nd->parent;
    *out_right = right;
    return mid;
}

/* ========== 插入 (核心) ========== */

void bpt_init(BPTree *t)
{
    if (t != NULL) {
        *t = NULL;
    }
}

/* 递归插入 */
static DsResult insert_rec(BPTNode *x, int key,
                           int *out_key, BPTNode **out_right)
{
    int i = 0;

    if (x->leaf) {
        for (i = 0; i < x->n; ++i) {
            if (x->keys[i] == key) {
                return (DsResult){DS_ERROR, "关键字已存在"};
            }
        }
        leaf_insert(x, key);
        if (x->n < BPT_MAX) {
            *out_key = 0;
            *out_right = NULL;
            return (DsResult){DS_OK, "插入成功"};
        }
        {
            BPTNode *right = NULL;
            int pk = split_leaf(x, &right);
            *out_key = pk;
            *out_right = right;
            return (DsResult){DS_OK, "叶分裂成功"};
        }
    }

    /* 内部结点 */
    {
        int ci = 0;
        int ckey = 0;
        BPTNode *cright = NULL;
        DsResult st;

        i = locate(x, key);
        ci = i;
        if (ci < x->n && x->keys[ci] == key) {
            ++ci;
        }

        st = insert_rec(x->ch[ci], key, &ckey, &cright);
        if (st.code != DS_OK) {
            return st;
        }
        if (ckey == 0) {
            *out_key = 0;
            *out_right = NULL;
            return (DsResult){DS_OK, "插入成功"};
        }
        {
            int j = 0;
            for (j = x->n; j > ci; --j) {
                x->keys[j] = x->keys[j - 1];
                x->ch[j + 1] = x->ch[j];
            }
            x->keys[ci] = ckey;
            x->ch[ci + 1] = cright;
            cright->parent = x;
            x->n++;
        }
        if (x->n < BPT_MAX) {
            *out_key = 0;
            *out_right = NULL;
            return (DsResult){DS_OK, "插入成功"};
        }
        {
            BPTNode *iright = NULL;
            int pk = split_inner(x, &iright);
            *out_key = pk;
            *out_right = iright;
            return (DsResult){DS_OK, "内部分裂成功"};
        }
    }
}

DsResult bpt_insert(BPTree *t, int key)
{
    BPTNode *root = NULL;
    BPTNode *new_root = NULL;
    int pk = 0;
    BPTNode *cright = NULL;
    DsResult st;

    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }
    if (*t == NULL) {
        root = bp_new(1);
        if (root == NULL) {
            return (DsResult){DS_OVERFLOW, "结点内存分配失败"};
        }
        root->keys[0] = key;
        root->n = 1;
        *t = root;
        return (DsResult){DS_OK, "B+树创建成功"};
    }
    root = *t;
    st = insert_rec(root, key, &pk, &cright);
    if (st.code != DS_OK) {
        return st;
    }
    if (pk != 0 && cright != NULL) {
        new_root = bp_new(0);
        if (new_root == NULL) {
            return (DsResult){DS_OVERFLOW, "结点内存分配失败"};
        }
        new_root->keys[0] = pk;
        new_root->ch[0] = root;
        new_root->ch[1] = cright;
        new_root->n = 1;
        root->parent = new_root;
        cright->parent = new_root;
        *t = new_root;
    }
    return (DsResult){DS_OK, "插入成功"};
}

/* ========== 查找 ========== */

DsResult bpt_search(BPTree t, int key)
{
    BPTNode *x = t;
    int i = 0;

    if (t == NULL) {
        return (DsResult){DS_NOT_FOUND, "树为空"};
    }
    while (!x->leaf) {
        i = locate(x, key);
        if (i < x->n && x->keys[i] == key) {
            ++i;
        }
        x = x->ch[i];
    }
    for (i = 0; i < x->n; ++i) {
        if (x->keys[i] == key) {
            return (DsResult){DS_OK, "查找成功"};
        }
    }
    return (DsResult){DS_NOT_FOUND, "未找到目标关键字"};
}

/* ========== 范围查询 ========== */

DsResult bpt_range(BPTree t, int lo, int hi, int *out, size_t cap, size_t *n)
{
    BPTNode *leaf = NULL;
    int i = 0;
    size_t cnt = 0;

    if (out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    *n = 0;
    if (t == NULL || lo > hi) {
        return (DsResult){DS_OK, "空树或无效范围"};
    }
    leaf = t;
    while (!leaf->leaf) {
        i = locate(leaf, lo);
        if (i < leaf->n && leaf->keys[i] == lo) {
            ++i;
        }
        leaf = leaf->ch[i];
    }
    while (leaf != NULL) {
        for (i = 0; i < leaf->n; ++i) {
            if (leaf->keys[i] >= lo && leaf->keys[i] <= hi) {
                if (cnt < cap) {
                    out[cnt] = leaf->keys[i];
                }
                ++cnt;
            } else if (leaf->keys[i] > hi) {
                *n = cnt;
                return (DsResult){DS_OK, "范围查询完成"};
            }
        }
        leaf = (BPTNode *)LEAF_NEXT(leaf);
    }
    *n = cnt;
    return (DsResult){DS_OK, "范围查询完成"};
}

/* ========== 删除 ========== */

DsResult bpt_delete(BPTree *t, int key)
{
    BPTNode *leaf = NULL;
    BPTNode *bottom = NULL;     /* 删除路径最底端幸存结点(收缩/重导起点) */
    int i = 0;
    int j = 0;

    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }
    if (*t == NULL) {
        return (DsResult){DS_NOT_FOUND, "树为空"};
    }
    leaf = *t;
    while (!leaf->leaf) {
        i = locate(leaf, key);
        if (i < leaf->n && leaf->keys[i] == key) {
            ++i;
        }
        leaf = leaf->ch[i];
    }
    i = locate(leaf, key);
    if (i >= leaf->n || leaf->keys[i] != key) {
        return (DsResult){DS_NOT_FOUND, "未找到目标关键字"};
    }
    for (j = i; j < leaf->n - 1; ++j) {
        leaf->keys[j] = leaf->keys[j + 1];
    }
    leaf->n--;
    /* 修正祖先路由键 —— 双保险: 旧趟覆盖非删除路径的常规修正, 新趟兜底收缩与空叶场景 */
    {
        BPTNode *x = leaf;
        while (x->parent != NULL) {
            BPTNode *p = x->parent;
            int ci = 0;
            for (ci = 0; ci <= p->n; ++ci) {
                if (p->ch[ci] == x) {
                    break;
                }
            }
            if (ci > 0 && ci <= p->n) {
                BPTNode *minleaf = p->ch[ci];
                while (minleaf != NULL && !minleaf->leaf) {
                    minleaf = minleaf->ch[0];
                }
                if (minleaf != NULL && minleaf->n > 0) {
                    p->keys[ci - 1] = minleaf->keys[0];
                }
            }
            x = p;
        }
    }
    /* 空叶摘除 */
    if (leaf->n == 0) {
        BPTNode *p = leaf->parent;
        if (p == NULL) {
            free(leaf);
            *t = NULL;
            return (DsResult){DS_OK, "删除成功"};
        }
        {
            BPTNode *prev = leftmost(*t);
            while (prev != NULL && (BPTNode *)LEAF_NEXT(prev) != leaf) {
                prev = (BPTNode *)LEAF_NEXT(prev);
            }
            if (prev != NULL) {
                LEAF_SET_NEXT(prev, LEAF_NEXT(leaf));
            }
        }
        {
            int ci = 0;
            int found = -1;
            for (ci = 0; ci <= p->n; ++ci) {
                if (p->ch[ci] == leaf) {
                    found = ci;
                    break;
                }
            }
            if (found >= 0) {
                for (j = found; j < p->n; ++j) {
                    p->ch[j] = p->ch[j + 1];
                }
                {
                    int ki = found > 0 ? found - 1 : 0;
                    for (j = ki; j < p->n - 1; ++j) {
                        p->keys[j] = p->keys[j + 1];
                    }
                }
                p->n--;
                free(leaf);
                bottom = p;
                if (p->n == 0 && p->parent == NULL) {
                    *t = p->ch[0];
                    if (*t != NULL) {
                        (*t)->parent = NULL;
                    }
                    free(p);
                    bottom = *t;
                }
            }
        }
    }
    if (bottom == NULL) {
        bottom = leaf;
    }
    /* 收缩 + 路由键重导: 自底向上沿删除路径单趟处理
     * 1) 收缩: 删除不借不并, 内部结点键数可降至 0(仅剩 ch[0] 一个
     *    孩子指针, 无路由键, 树高虚高)。此时用唯一孩子 ch[0] 原位
     *    顶替该结点(父结点孩子指针直接改指孩子)并释放之: 子树内容
     *    一键未变, 故父结点路由键所指"右子树最小键"不变, 无需调整;
     *    顶替后孩子自身可能同为 0 键内部结点, 继续在同层级联收缩;
     *    根结点退化则由唯一孩子取代, 树高减一。
     * 2) 路由键重导: 每层重导父路由键 = 对应右子树当前最小键 ——
     *    空叶摘除可能使祖先子树最小键变化(如被摘叶是其父子树的
     *    最左叶时, 既有修正逻辑会跳过空叶而漏更祖父层) */
    {
        BPTNode *x = bottom;
        while (x != NULL) {
            BPTNode *p = x->parent;
            if (!x->leaf && x->n == 0) {
                BPTNode *c = x->ch[0];      /* 内部结点必有孩子 */
                if (p == NULL) {
                    *t = c;                 /* 根退化: 树高减一 */
                    if (c != NULL) {
                        c->parent = NULL;
                    }
                } else {
                    int ci = 0;
                    while (ci <= p->n && p->ch[ci] != x) {
                        ++ci;
                    }
                    if (ci <= p->n) {
                        p->ch[ci] = c;
                        if (c != NULL) {        /* 防御: 与根分支(根退化处理)对称 */
                            c->parent = p;
                        }
                    }
                }
                free(x);
                x = c;                      /* 孩子顶替后继续检查级联退化 */
                continue;
            }
            if (p != NULL) {
                int ci = 0;
                while (ci <= p->n && p->ch[ci] != x) {
                    ++ci;
                }
                if (ci > 0 && ci <= p->n) {
                    BPTNode *m = p->ch[ci];
                    while (m != NULL && !m->leaf) {
                        m = m->ch[0];
                    }
                    if (m != NULL && m->n > 0) {
                        p->keys[ci - 1] = m->keys[0];
                    }
                }
            }
            x = x->parent;
        }
    }
    return (DsResult){DS_OK, "删除成功"};
}

/* ========== 统计 ========== */

size_t bpt_count(BPTree t)
{
    BPTNode *leaf = leftmost(t);
    size_t cnt = 0;
    while (leaf != NULL) {
        cnt += (size_t)leaf->n;
        leaf = (BPTNode *)LEAF_NEXT(leaf);
    }
    return cnt;
}

size_t bpt_height(BPTree t)
{
    size_t h = 0;
    BPTNode *x = t;
    while (x != NULL) {
        ++h;
        x = x->leaf ? NULL : x->ch[0];
    }
    return h;
}

/* ========== 遍历 ========== */

DsResult bpt_inorder(BPTree t, BPTVisit visit)
{
    BPTNode *leaf = NULL;
    int i = 0;

    if (visit == NULL) {
        return (DsResult){DS_NULL_PTR, "回调函数为空"};
    }
    if (t == NULL) {
        return (DsResult){DS_OK, "空树遍历完成"};
    }
    leaf = leftmost(t);
    while (leaf != NULL) {
        for (i = 0; i < leaf->n; ++i) {
            visit(leaf->keys[i]);
        }
        leaf = (BPTNode *)LEAF_NEXT(leaf);
    }
    return (DsResult){DS_OK, "中序遍历完成"};
}

/* ========== 销毁 ========== */

static void destroy_rec(BPTNode *x)
{
    int i = 0;
    if (x == NULL) {
        return;
    }
    if (!x->leaf) {
        for (i = 0; i <= x->n; ++i) {
            destroy_rec(x->ch[i]);
        }
    }
    free(x);
}

DsResult bpt_destroy(BPTree *t)
{
    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }
    destroy_rec(*t);
    *t = NULL;
    return (DsResult){DS_OK, "B+树销毁成功"};
}

/* ========== 验证 ========== */

/* 自上而下路由键不变式校验: 每个内部结点对每个 ci>0,
 * keys[ci-1] 必须等于孩子 ch[ci] 子树当前最小键(沿 ch[ci] 走到
 * 最左叶取首键); 违规计入 cnt(路由键失准会表现为查找假未命中) */
static void route_verify(BPTNode *x, size_t *cnt)
{
    int i = 0;
    if (x == NULL || x->leaf) {
        return;
    }
    for (i = 1; i <= x->n; ++i) {
        BPTNode *minleaf = leftmost(x->ch[i]);
        if (minleaf == NULL || minleaf->n == 0 ||
            x->keys[i - 1] != minleaf->keys[0]) {
            ++(*cnt);
        }
    }
    for (i = 0; i <= x->n; ++i) {
        route_verify(x->ch[i], cnt);
    }
}

DsResult bpt_verify(BPTree t, size_t *viol)
{
    BPTNode *leaf = NULL;
    int prev = 0;
    int have_prev = 0;
    size_t cnt = 0;

    if (viol != NULL) {
        *viol = 0;
    }
    if (t == NULL) {
        return (DsResult){DS_EMPTY, "B+树为空"};
    }
    leaf = leftmost(t);
    while (leaf != NULL) {
        int i = 0;
        for (i = 0; i < leaf->n; ++i) {
            if (have_prev && prev >= leaf->keys[i]) {
                cnt++;
            }
            prev = leaf->keys[i];
            have_prev = 1;
        }
        leaf = (BPTNode *)LEAF_NEXT(leaf);
    }
    route_verify(t, &cnt);
    if (viol != NULL) {
        *viol = cnt;
    }
    return cnt == 0 ? (DsResult){DS_OK, "B+树结构验证通过"}
                    : (DsResult){DS_ERROR, "B+树结构验证失败"};
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "位置/下标越界";
        case DS_OVERFLOW:    return "内存分配失败";
        case DS_EMPTY:       return "B+树为空";
        case DS_NOT_FOUND:   return "未找到目标关键字";
        default:             return "未知状态码";
    }
}
