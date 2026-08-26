/**
 * @file tree_storage.c
 * @brief 树的三种存储结构接口实现
 *
 * 设计要点:
 *   1. 三种结构操作同一棵树时结点下标保持一致（互转按下标
 *      对应搬运 data，双亲/孩子关系按各自方式重建），
 *      这是三方对拍能成立的前提
 *   2. cst_from_parent 中"挂链尾"保证兄弟顺序 = 下标顺序，
 *      与孩子表示法的链序一致
 * 复杂度: 各单点操作 O(1)~O(n)；互转 O(n+e)
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree_storage.h"


/* ============ 1. 双亲表示法 ============ */

DsResult pt_init(ParentTree *t)
{
    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }

    t->count = 0;
    return (DsResult){DS_OK, "双亲表示法初始化成功"};
}

DsResult pt_insert(ParentTree *t, int parent, int data, size_t *idx)
{
    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }
    if (parent == -1) {
        if (t->count != 0) {
            return (DsResult){DS_ERROR, "已有根，不允许重复插入根结点"};
        }
    } else if (parent < 0 || (size_t)parent >= t->count) {
        return (DsResult){DS_OUT_OF_RANGE, "双亲下标非法"};
    }
    if (t->count >= TREE_MAX_NODES) {
        return (DsResult){DS_OVERFLOW, "结点数已达上限"};
    }

    t->nodes[t->count].data = data;
    t->nodes[t->count].parent = parent;
    t->count++;
    if (idx != NULL) {
        *idx = t->count - 1;
    }
    return (DsResult){DS_OK, "插入成功"};
}

size_t pt_count(const ParentTree *t)
{
    return (t == NULL) ? 0 : t->count;
}

DsResult pt_children(const ParentTree *t, size_t i, size_t out[], size_t *n)
{
    size_t k = 0;

    if (t == NULL || out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (i >= t->count) {
        return (DsResult){DS_OUT_OF_RANGE, "结点下标越界"};
    }

    /* 双亲表示法找孩子必须全表扫描 —— 结构决定的代价 */
    *n = 0;
    for (k = 0; k < t->count; k++) {
        if (t->nodes[k].parent == (int)i) {
            out[(*n)++] = k;
        }
    }
    return (DsResult){DS_OK, "查找孩子成功"};
}

DsResult pt_root(const ParentTree *t, size_t *idx)
{
    if (t == NULL || idx == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (t->count == 0) {
        return (DsResult){DS_ERROR, "空树无根"};
    }

    *idx = 0; /* 根总是首结点（insert 保证） */
    return (DsResult){DS_OK, "取根成功"};
}

DsResult pt_print(const ParentTree *t)
{
    size_t i = 0;

    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }

    printf("双亲表示法(%zu 结点): ", t->count);
    for (i = 0; i < t->count; i++) {
        printf("[%zu]=%d(p=%d) ", i, t->nodes[i].data, t->nodes[i].parent);
    }
    printf("\n");
    return (DsResult){DS_OK, "打印完成"};
}

/* ============ 2. 孩子表示法 ============ */

DsResult clt_init(ChildListTree *t)
{
    size_t i = 0;

    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }

    for (i = 0; i < TREE_MAX_NODES; i++) {
        t->nodes[i].data = 0;
        t->nodes[i].first_child = NULL;
    }
    t->count = 0;
    return (DsResult){DS_OK, "孩子表示法初始化成功"};
}

DsResult clt_destroy(ChildListTree *t)
{
    size_t i = 0;

    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }

    /* 逐表头释放孩子链，再整体清零 */
    for (i = 0; i < t->count; i++) {
        ChildLink *p = t->nodes[i].first_child;

        while (p != NULL) {
            ChildLink *next = p->next;

            free(p);
            p = next;
        }
        t->nodes[i].first_child = NULL;
    }
    t->count = 0;
    return (DsResult){DS_OK, "销毁成功"};
}

DsResult clt_insert(ChildListTree *t, int parent, int data, size_t *idx)
{
    ChildLink *link = NULL;

    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }
    if (parent == -1) {
        if (t->count != 0) {
            return (DsResult){DS_ERROR, "已有根，不允许重复插入根结点"};
        }
    } else if (parent < 0 || (size_t)parent >= t->count) {
        return (DsResult){DS_OUT_OF_RANGE, "双亲下标非法"};
    }
    if (t->count >= TREE_MAX_NODES) {
        return (DsResult){DS_OVERFLOW, "结点数已达上限"};
    }

    t->nodes[t->count].data = data;
    t->nodes[t->count].first_child = NULL;

    if (parent != -1) {
        /* 尾插保持孩子下标序（与打印/遍历顺序稳定一致） */
        link = (ChildLink *)malloc(sizeof(ChildLink));
        if (link == NULL) {
            return (DsResult){DS_ERROR, "孩子链结点分配失败"};
        }
        link->child = t->count;
        link->next = NULL;

        if (t->nodes[parent].first_child == NULL) {
            t->nodes[parent].first_child = link;
        } else {
            ChildLink *p = t->nodes[parent].first_child;

            while (p->next != NULL) {
                p = p->next;
            }
            p->next = link;
        }
    }

    t->count++;
    if (idx != NULL) {
        *idx = t->count - 1;
    }
    return (DsResult){DS_OK, "插入成功"};
}

size_t clt_count(const ChildListTree *t)
{
    return (t == NULL) ? 0 : t->count;
}

DsResult clt_children(const ChildListTree *t, size_t i, size_t out[], size_t *n)
{
    const ChildLink *p = NULL;

    if (t == NULL || out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (i >= t->count) {
        return (DsResult){DS_OUT_OF_RANGE, "结点下标越界"};
    }

    *n = 0;
    for (p = t->nodes[i].first_child; p != NULL; p = p->next) {
        out[(*n)++] = p->child;
    }
    return (DsResult){DS_OK, "查找孩子成功"};
}

DsResult clt_print(const ChildListTree *t)
{
    size_t i = 0;

    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }

    printf("孩子表示法(%zu 结点): ", t->count);
    for (i = 0; i < t->count; i++) {
        const ChildLink *p = NULL;

        printf("[%zu]=%d(孩子: ", i, t->nodes[i].data);
        for (p = t->nodes[i].first_child; p != NULL; p = p->next) {
            printf("%zu ", p->child);
        }
        printf(") ");
    }
    printf("\n");
    return (DsResult){DS_OK, "打印完成"};
}

/* ============ 3. 孩子兄弟表示法 ============ */

DsResult cst_from_parent(const ParentTree *pt, CSTree *t)
{
    CSNode **created = NULL; /* 下标 -> 结点指针 映射表 */
    size_t i = 0;

    if (pt == NULL || t == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *t = NULL;
    if (pt->count == 0) {
        return (DsResult){DS_OK, "空树转换成功"};
    }

    created = (CSNode **)calloc(pt->count, sizeof(CSNode *));
    if (created == NULL) {
        return (DsResult){DS_ERROR, "映射表分配失败"};
    }

    /* 第一遍: 按下标序创建全部结点 */
    for (i = 0; i < pt->count; i++) {
        created[i] = (CSNode *)malloc(sizeof(CSNode));
        if (created[i] == NULL) {
            size_t k = 0;

            for (k = 0; k < i; k++) {
                free(created[k]);
            }
            free(created);
            return (DsResult){DS_ERROR, "结点分配失败"};
        }
        created[i]->data = pt->nodes[i].data;
        created[i]->first_child = NULL;
        created[i]->next_sibling = NULL;
    }

    /* 第二遍: 按双亲关系挂链 —— 尾插保证兄弟序=下标序 */
    for (i = 1; i < pt->count; i++) {
        int p = pt->nodes[i].parent;

        if (p < 0) {
            continue; /* 防御: 多根情形仅取首根挂链（单根树不触发） */
        }
        if (created[p]->first_child == NULL) {
            created[p]->first_child = created[i];
        } else {
            CSNode *sib = created[p]->first_child;

            while (sib->next_sibling != NULL) {
                sib = sib->next_sibling;
            }
            sib->next_sibling = created[i];
        }
    }

    *t = created[0]; /* 首结点即根（pt_insert 保证） */
    free(created);
    return (DsResult){DS_OK, "孩子兄弟表示法转换成功"};
}

/* 后序递归释放: 先释放整棵子树的孩子与兄弟，再释放自身 */
static void free_cs(CSNode *node)
{
    if (node == NULL) {
        return;
    }

    free_cs(node->first_child);
    free_cs(node->next_sibling);
    free(node);
}

DsResult cst_destroy(CSTree *t)
{
    if (t == NULL) {
        return (DsResult){DS_NULL_PTR, "树指针为空"};
    }

    free_cs(*t);
    *t = NULL;
    return (DsResult){DS_OK, "销毁成功"};
}

size_t cst_count(CSTree t)
{
    if (t == NULL) {
        return 0;
    }
    /* 沿 first_child 下降一层，沿 next_sibling 平级展开 */
    return 1 + cst_count(t->first_child) + cst_count(t->next_sibling);
}

size_t cst_depth(CSTree t)
{
    size_t d_child = 0;
    size_t d_sib = 0;

    if (t == NULL) {
        return 0;
    }

    /*
     * 只有孩子方向增加深度；兄弟同层但兄弟的子树深度
     * 可能更深，须向上传递 —— 故取两者最大值
     */
    d_child = cst_depth(t->first_child);
    d_sib = cst_depth(t->next_sibling);
    return (d_child + 1 > d_sib) ? d_child + 1 : d_sib;
}

DsResult cst_preorder(CSTree t, int out[], size_t *n)
{
    if (out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *n = 0;
    if (t == NULL) {
        return (DsResult){DS_OK, "空树遍历完成"};
    }

    /* 先根序: 根 -> 各子树（各子树根经 next_sibling 链自然相连） */
    out[(*n)++] = t->data;
    {
        size_t sub = 0;

        cst_preorder(t->first_child, out + *n, &sub);
        *n += sub;
    }
    {
        size_t sub = 0;

        cst_preorder(t->next_sibling, out + *n, &sub);
        *n += sub;
    }
    return (DsResult){DS_OK, "先根遍历完成"};
}

DsResult cst_postorder(CSTree t, int out[], size_t *n)
{
    if (out == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *n = 0;
    if (t == NULL) {
        return (DsResult){DS_OK, "空树遍历完成"};
    }

    /* 后根序: 各子树 -> 根；兄弟子树在本结点之前完成 */
    {
        size_t sub = 0;

        cst_postorder(t->first_child, out + *n, &sub);
        *n += sub;
    }
    out[(*n)++] = t->data;
    {
        size_t sub = 0;

        cst_postorder(t->next_sibling, out + *n, &sub);
        *n += sub;
    }
    return (DsResult){DS_OK, "后根遍历完成"};
}

/* 缩进打印辅助: depth 为当前缩进层数 */
static void print_indent(const CSNode *node, size_t depth)
{
    size_t i = 0;

    if (node == NULL) {
        return;
    }

    for (i = 0; i < depth; i++) {
        printf("  ");
    }
    printf("|- %d\n", node->data);
    print_indent(node->first_child, depth + 1); /* 孩子降一层 */
    print_indent(node->next_sibling, depth);    /* 兄弟同层 */
}

DsResult cst_print(CSTree t)
{
    if (t == NULL) {
        printf("(空树)\n");
        return (DsResult){DS_OK, "空树打印完成"};
    }

    print_indent(t, 0);
    return (DsResult){DS_OK, "打印完成"};
}

/* ============ 互转 ============ */

DsResult convert_parent_to_childlist(const ParentTree *pt, ChildListTree *clt)
{
    size_t i = 0;

    if (pt == NULL || clt == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    clt_init(clt);
    for (i = 0; i < pt->count; i++) {
        /* 下标不变地重放插入序列，双亲关系即恢复 */
        if (clt_insert(clt, pt->nodes[i].parent, pt->nodes[i].data, NULL).code != DS_OK) {
            clt_destroy(clt);
            return (DsResult){DS_ERROR, "转换失败"};
        }
    }
    return (DsResult){DS_OK, "双亲转孩子表示法成功"};
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
