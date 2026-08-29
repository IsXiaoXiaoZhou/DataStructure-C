#ifndef TREE_STORAGE_H
#define TREE_STORAGE_H

/**
 * @file tree_storage.h
 * @brief 树的三种存储结构接口定义
 *
 * 存储结构（同一棵树的三种等价表示）:
 *   1. 双亲表示法: 顺序数组，每结点存 data + 双亲下标
 *      —— 找双亲 O(1)，找孩子要全表扫描
 *   2. 孩子表示法: 顺序数组 + 每格挂孩子链表
 *      —— 找孩子沿链 O(孩子数)，找双亲要扫描全部链
 *   3. 孩子兄弟表示法: 二叉链表，first_child 指向第一个孩子，
 *      next_sibling 指向下一兄弟 —— 树/森林与二叉树互转的桥梁
 * 设计要点:
 *   提供三种表示的互转（双亲 -> 孩子 / 双亲 -> 孩子兄弟），
 *   用于验证同一棵树在各表示下信息一致
 * 复杂度: 双亲法插入 O(1)；孩子法插入 O(当前孩子数)
 *         （ChildBox 无尾指针，沿孩子链尾插以保持孩子下标序）；
 *         互转 O(n+e)
 */

#include <stddef.h>

/* 状态码（按本模块所需裁剪，语义见各枚举项注释） */
typedef enum {
    DS_OK = 0,       /* 操作成功 */
    DS_ERROR,        /* 一般性错误 */
    DS_NULL_PTR,     /* 空指针参数 */
    DS_OUT_OF_RANGE, /* 下标越界 */
    DS_OVERFLOW      /* 容量已满 */
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 树最大结点数（三种表示共用） */
#define TREE_MAX_NODES 100

/* ============ 1. 双亲表示法 ============ */

/* 双亲表示法结点：数据 + 双亲下标（根结点 parent 为 -1） */
typedef struct {
    int data;
    int parent; /* 双亲在数组中的下标；根为 -1 */
} ParentNode;

/* 双亲表示法树 */
typedef struct {
    ParentNode nodes[TREE_MAX_NODES];
    size_t count; /* 结点个数 */
} ParentTree;

DsResult pt_init(ParentTree *t);
DsResult pt_insert(ParentTree *t, int parent, int data, size_t *idx);
size_t pt_count(const ParentTree *t);
DsResult pt_children(const ParentTree *t, size_t i, size_t out[], size_t *n);
DsResult pt_root(const ParentTree *t, size_t *idx);
DsResult pt_print(const ParentTree *t);

/* ============ 2. 孩子表示法 ============ */

/* 孩子链表结点：孩子下标 + 下一兄弟链 */
typedef struct ChildLink {
    size_t child;             /* 孩子在结点数组中的下标 */
    struct ChildLink *next;   /* 同一双亲的下一孩子 */
} ChildLink;

/* 孩子表示法表头结点：数据 + 孩子链首 */
typedef struct {
    int data;
    ChildLink *first_child; /* 孩子链表头（按插入序） */
} ChildBox;

/* 孩子表示法树 */
typedef struct {
    ChildBox nodes[TREE_MAX_NODES];
    size_t count;
} ChildListTree;

DsResult clt_init(ChildListTree *t);
DsResult clt_destroy(ChildListTree *t);
DsResult clt_insert(ChildListTree *t, int parent, int data, size_t *idx);
size_t clt_count(const ChildListTree *t);
DsResult clt_children(const ChildListTree *t, size_t i, size_t out[], size_t *n);
DsResult clt_print(const ChildListTree *t);

/* ============ 3. 孩子兄弟表示法 ============ */

/* 孩子兄弟链结点（即二叉链表结点） */
typedef struct CSNode {
    int data;
    struct CSNode *first_child;   /* 第一个孩子 */
    struct CSNode *next_sibling;  /* 下一兄弟 */
} CSNode;

typedef CSNode *CSTree; /* 根指针，空树为 NULL */

DsResult cst_from_parent(const ParentTree *pt, CSTree *t);
DsResult cst_destroy(CSTree *t);
size_t cst_count(CSTree t);
size_t cst_depth(CSTree t);
DsResult cst_preorder(CSTree t, int out[], size_t *n);
DsResult cst_postorder(CSTree t, int out[], size_t *n);
DsResult cst_print(CSTree t);

/* ============ 互转与公共 ============ */

DsResult convert_parent_to_childlist(const ParentTree *pt, ChildListTree *clt);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* TREE_STORAGE_H */
