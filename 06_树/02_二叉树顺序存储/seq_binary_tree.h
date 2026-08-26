#ifndef SEQ_BINARY_TREE_H
#define SEQ_BINARY_TREE_H

/**
 * @file seq_binary_tree.h
 * @brief 二叉树的顺序存储（数组实现）接口定义
 *
 * 存储结构: data[1..last] 按完全二叉树编号连续存放，
 *           编号 i 的左孩子 2i、右孩子 2i+1、双亲 i/2
 * 设计要点:
 *   1. 顺序存储天然适合完全二叉树（无空间浪费）；
 *      一般二叉树需按最坏形态补空位，深度为 k 的单支树
 *      将浪费 2^k - n 个单元 —— 教学上用"层序连续追加"
 *      保持完全二叉树形态，保证编号关系恒成立
 *   2. 下标从 1 开始（0 号弃用），使 2i/2i+1/i/2 关系式
 *      无 +1 偏移，与教材编号一致
 * 复杂度: 双亲/孩子定位 O(1) —— 顺序存储的核心优势;
 *         遍历 O(n)
 */

#include <stddef.h>

/* 状态码（按本模块所需裁剪，语义见各枚举项注释） */
typedef enum {
    DS_OK = 0,       /* 操作成功 */
    DS_ERROR,        /* 一般性错误（结构不满足时） */
    DS_NULL_PTR,     /* 空指针参数 */
    DS_OUT_OF_RANGE, /* 下标越界 */
    DS_OVERFLOW      /* 容量已满 */
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 顺序二叉树最大容量（编号 1..SEQ_BT_MAX，深度最多 7 层） */
#define SEQ_BT_MAX 127

/* 顺序二叉树：编号数组 + 有效长度 */
typedef struct {
    int    data[SEQ_BT_MAX + 1]; /* data[i]: 编号 i 的结点，0 号弃用 */
    size_t last;                 /* 最后占用编号（= 结点个数，完全形态连续） */
} SeqBinaryTree;

DsResult sbt_init(SeqBinaryTree *t);
DsResult sbt_insert_level(SeqBinaryTree *t, int data, size_t *idx);
size_t sbt_count(const SeqBinaryTree *t);
int sbt_is_empty(const SeqBinaryTree *t);
DsResult sbt_parent(const SeqBinaryTree *t, size_t i, size_t *parent);
DsResult sbt_left_child(const SeqBinaryTree *t, size_t i, size_t *child);
DsResult sbt_right_child(const SeqBinaryTree *t, size_t i, size_t *child);
DsResult sbt_get(const SeqBinaryTree *t, size_t i, int *value);
size_t sbt_depth(const SeqBinaryTree *t);
DsResult sbt_preorder(const SeqBinaryTree *t, int out[], size_t *n);
DsResult sbt_inorder(const SeqBinaryTree *t, int out[], size_t *n);
DsResult sbt_postorder(const SeqBinaryTree *t, int out[], size_t *n);
DsResult sbt_level_order(const SeqBinaryTree *t, int out[], size_t *n);
DsResult sbt_print(const SeqBinaryTree *t);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* SEQ_BINARY_TREE_H */
