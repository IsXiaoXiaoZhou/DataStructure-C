#ifndef BPLUS_H
#define BPLUS_H

/**
 * @file bplus.h
 * @brief B+ 树接口定义（多路平衡索引+叶链，数据库索引标准结构）
 *
 * 与 B 树区别:
 *   1. 所有关键字存于叶结点，叶结点串联为升序链表
 *   2. 内部结点仅存路由键(=右子树最小键)
 *   3. 查找到达叶层；范围查询沿叶链扫描 O(k + log_t n)
 * 复杂度: 插入/删除/查找 O(log_t n)，范围查询 O(k + log_t n)
 */

#include <stddef.h>

typedef enum {
    DS_OK = 0,
    DS_ERROR,
    DS_NULL_PTR,
    DS_OUT_OF_RANGE,
    DS_OVERFLOW,
    DS_EMPTY
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/*
 * 结点键数不变式（按 bplus.c 实现核实，数据库常用变体）:
 *   - 结构: n 个关键字配 n+1 棵子树，内部结点 keys[i] 为路由键 =
 *     右子树 ch[i+1] 的最小键（ch[0] 子树全部键 < keys[0]）；
 *     与教材/王道 m 阶 B+ 树"n 棵子树 n 个关键字"的记法不同
 *   - 上界: 任意结点键数 <= 2t（孩子数 <= 2t+1），到达即分裂
 *   - 下界: 仅由插入分裂保证 —— 叶分裂后各含 t 键，内部分裂后
 *     左 t 键 / 右 t-1 键；删除不借位不合并（仅摘除空叶并回填
 *     祖先路由键），故非根结点键数不维护教材 ⌈m/2⌉ 下限，可降至 1
 */
#define BPT_T 3                       /* 最小度数 t */
#define BPT_MAX (2 * BPT_T)           /* 每结点最多键数 */
#define BPT_CHILD_MAX (BPT_MAX + 1)   /* 最多孩子数 */

typedef void (*BPTVisit)(int value);

typedef struct BPTNode {
    int               n;                /* 当前键数 */
    int               keys[BPT_MAX];
    struct BPTNode   *ch[BPT_CHILD_MAX]; /* 孩子指针(内部有效), 叶时 ch[0]=next */
    struct BPTNode   *parent;           /* 父指针(回溯用) */
    int               leaf;             /* 1=叶 */
} BPTNode;

typedef BPTNode *BPTree;

void   bpt_init(BPTree *t);
DsResult bpt_insert(BPTree *t, int key);
DsResult bpt_search(BPTree t, int key);
DsResult bpt_range(BPTree t, int lo, int hi, int *out, size_t cap, size_t *n);
DsResult bpt_delete(BPTree *t, int key);
size_t bpt_count(BPTree t);
size_t bpt_height(BPTree t);
DsResult bpt_verify(BPTree t, size_t *viol);
DsResult bpt_inorder(BPTree t, BPTVisit v);
DsResult bpt_destroy(BPTree *t);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif
