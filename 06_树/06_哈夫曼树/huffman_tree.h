#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H

/**
 * @file huffman_tree.h
 * @brief 哈夫曼树（最优二叉树）与哈夫曼编码接口定义
 *
 * 存储结构: 静态三叉链表 —— 数组 1..2n-1，前 n 为叶子（权值输入序），
 *           后 n-1 为内部结点（每轮合并新产生）
 * 设计要点:
 *   1. 构造贪心: 每次取"双亲为空且权最小"的两结点合并，
 *      新结点权 = 二者之和；共合并 n-1 次
 *   2. WPL（带权路径长度）= Σ w_i * l_i，哈夫曼树使 WPL 最小；
 *      其值恰等于"反复合并两个最小权（合并果子）"的合并总代价
 *   3. 编码: 从叶子向根回溯记录转向位再反转（左 0 右 1）；
 *      任意编码均非其他编码的前缀（前缀码/无歧义解码的根基）
 * 复杂度: 构造 O(n^2)（教学实现线性选最小）；编码 O(l)；
 *         解码 O(位数)
 */

#include <stddef.h>

/* 状态码（按本模块所需裁剪，语义见各枚举项注释） */
typedef enum {
    DS_OK = 0,       /* 操作成功 */
    DS_ERROR,        /* 一般性错误（非法字符/解码截断） */
    DS_NULL_PTR,     /* 空指针参数 */
    DS_OUT_OF_RANGE  /* 序号越界 */
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 叶子数上限 */
#define HUFF_MAX_LEAVES 256

/* 编码最大长度（最坏单支树深度 n-1） */
#define HUFF_MAX_CODE_LEN (HUFF_MAX_LEAVES)

/* 哈夫曼树: 1..2n-1 有效 */
typedef struct {
    int weight;  /* 权值 */
    int parent;  /* 双亲下标，0 表根 */
    int left;    /* 左孩子下标，0 表空 */
    int right;   /* 右孩子下标，0 表空 */
} HuffNode;

typedef struct {
    HuffNode nodes[2 * HUFF_MAX_LEAVES];
    size_t leaf_count; /* 叶子数 n（结点总数 2n-1） */
} HuffmanTree;

DsResult ht_build(HuffmanTree *ht, const int weights[], size_t n);
DsResult ht_wpl(const HuffmanTree *ht, long *wpl);
DsResult ht_code(const HuffmanTree *ht, size_t i, char code[]);
DsResult ht_decode(const HuffmanTree *ht, const char *bits, size_t symbols[], size_t *n);
size_t ht_leaf_count(const HuffmanTree *ht);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* HUFFMAN_TREE_H */
