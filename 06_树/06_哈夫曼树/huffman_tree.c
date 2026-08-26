/**
 * @file huffman_tree.c
 * @brief 哈夫曼树与哈夫曼编码接口实现
 *
 * 设计要点:
 *   1. select_two_min 线性扫"无双亲且未被选"的最小两结点，
 *      平手取下标小者 —— 保证确定性，便于测试断言
 *   2. 编码长度上限 n-1（单支哈夫曼树），缓冲按最坏容量给足
 * 复杂度: 构造 O(n^2)；WPL O(n^2)；编码 O(l)；解码 O(位数)
 */

#include <string.h>

#include "huffman_tree.h"


/* 在 nodes[1..range] 中选 parent==0 的最小两个（s1 权 <= s2 权，下标小者优先） */
static void select_two_min(const HuffmanTree *ht, size_t range, size_t *s1, size_t *s2)
{
    size_t i = 0;
    int min1 = 0x7FFFFFFF;
    int min2 = 0x7FFFFFFF;

    *s1 = 0;
    *s2 = 0;
    for (i = 1; i <= range; i++) {
        if (ht->nodes[i].parent != 0) {
            continue; /* 已被之前的合并收编 */
        }
        if (ht->nodes[i].weight < min1) {
            /* 新最小，原最小滑落为次小 */
            min2 = min1;
            *s2 = *s1;
            min1 = ht->nodes[i].weight;
            *s1 = i;
        } else if (ht->nodes[i].weight < min2) {
            min2 = ht->nodes[i].weight;
            *s2 = i;
        }
    }
}

DsResult ht_build(HuffmanTree *ht, const int weights[], size_t n)
{
    size_t i = 0;

    if (ht == NULL || weights == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (n < 1 || n > HUFF_MAX_LEAVES) {
        return (DsResult){DS_OUT_OF_RANGE, "叶子数越界，合法范围 [1, 256]"};
    }
    for (i = 0; i < n; i++) {
        if (weights[i] <= 0) {
            return (DsResult){DS_ERROR, "权值须为正数"};
        }
    }

    ht->leaf_count = n;
    for (i = 1; i <= 2 * n - 1; i++) {
        ht->nodes[i].weight = 0;
        ht->nodes[i].parent = 0;
        ht->nodes[i].left = 0;
        ht->nodes[i].right = 0;
    }
    for (i = 1; i <= n; i++) {
        ht->nodes[i].weight = weights[i - 1];
    }

    if (n == 1) {
        return (DsResult){DS_OK, "单叶子树构建成功"}; /* 单叶子无合并，根即叶 */
    }

    /* 共 n-1 轮合并，新结点编号 n+1 .. 2n-1 */
    for (i = n + 1; i <= 2 * n - 1; i++) {
        size_t s1 = 0;
        size_t s2 = 0;

        select_two_min(ht, i - 1, &s1, &s2);
        ht->nodes[s1].parent = (int)i;
        ht->nodes[s2].parent = (int)i;
        ht->nodes[i].left = (int)s1;   /* 小者为左 */
        ht->nodes[i].right = (int)s2;
        ht->nodes[i].weight = ht->nodes[s1].weight + ht->nodes[s2].weight;
    }
    return (DsResult){DS_OK, "哈夫曼树构建成功"};
}

DsResult ht_wpl(const HuffmanTree *ht, long *wpl)
{
    size_t i = 0;

    if (ht == NULL || wpl == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *wpl = 0;
    for (i = 1; i <= ht->leaf_count; i++) {
        int depth = 0;
        int p = ht->nodes[i].parent;

        while (p != 0) {
            depth++;
            p = ht->nodes[p].parent;
        }
        *wpl += (long)ht->nodes[i].weight * depth;
    }
    return (DsResult){DS_OK, "WPL计算成功"};
}

DsResult ht_code(const HuffmanTree *ht, size_t i, char code[])
{
    char rev[HUFF_MAX_CODE_LEN + 1];
    size_t len = 0;
    int c = 0;
    int p = 0;

    if (ht == NULL || code == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (i < 1 || i > ht->leaf_count) {
        return (DsResult){DS_OUT_OF_RANGE, "叶子序号越界"};
    }

    /* 单叶子树约定编码 "0"（解码时 0/1 均可映射，教学约定取 "0"） */
    if (ht->leaf_count == 1) {
        code[0] = '0';
        code[1] = '\0';
        return (DsResult){DS_OK, "单叶子编码成功"};
    }

    /* 叶 -> 根 回溯记方向（根方向决定当前结点是左 0 还是右 1） */
    c = (int)i;
    p = ht->nodes[c].parent;
    while (p != 0) {
        rev[len++] = (ht->nodes[p].left == c) ? '0' : '1';
        c = p;
        p = ht->nodes[p].parent;
    }

    /* 回溯得到的是逆序位串，反转即编码 */
    {
        size_t k = 0;

        for (k = 0; k < len; k++) {
            code[k] = rev[len - 1 - k];
        }
        code[len] = '\0';
    }
    return (DsResult){DS_OK, "编码成功"};
}

DsResult ht_decode(const HuffmanTree *ht, const char *bits, size_t symbols[], size_t *n)
{
    size_t count = 0;
    const char *p = bits;
    int cur = 0;

    if (ht == NULL || bits == NULL || symbols == NULL || n == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (ht->leaf_count == 0) {
        return (DsResult){DS_ERROR, "空树无法解码"};
    }

    /* 单叶子树: 每个字符解出一个符号 */
    if (ht->leaf_count == 1) {
        for (; *p != '\0'; p++) {
            if (*p != '0' && *p != '1') {
                return (DsResult){DS_ERROR, "含非法字符"};
            }
            symbols[count++] = 1;
        }
        *n = count;
        return (DsResult){DS_OK, "单叶子解码成功"};
    }

    cur = (int)(2 * ht->leaf_count - 1); /* 根编号 */
    for (; *p != '\0'; p++) {
        if (*p != '0' && *p != '1') {
            return (DsResult){DS_ERROR, "含非法字符"};
        }
        cur = (*p == '0') ? ht->nodes[cur].left : ht->nodes[cur].right;
        if (cur == 0) {
            return (DsResult){DS_ERROR, "位串走到空结点"}; /* 防御: 位串走到空（构造正确不会发生） */
        }
        if (ht->nodes[cur].left == 0 && ht->nodes[cur].right == 0) {
            symbols[count++] = (size_t)cur; /* 到叶: 输出并回根 */
            cur = (int)(2 * ht->leaf_count - 1);
        }
    }
    if (cur != (int)(2 * ht->leaf_count - 1)) {
        return (DsResult){DS_ERROR, "位串截断，编码不完整"}; /* 位串结束在半路（不完整编码） */
    }

    *n = count;
    return (DsResult){DS_OK, "解码成功"};
}

size_t ht_leaf_count(const HuffmanTree *ht)
{
    return (ht == NULL) ? 0 : ht->leaf_count;
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "序号越界";
        default:             return "未知状态码";
    }
}
