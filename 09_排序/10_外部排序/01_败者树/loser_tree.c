/**
 * @file loser_tree.c
 * @brief k 路归并"败者树"实现（最小关键字胜出）
 */

#include "loser_tree.h"


size_t lt_size_needed(size_t k)
{
    size_t s = 1;
    if (k == 0) return 1;
    while (s < k) s <<= 1;
    return s;
}

/* 自底向上建树: 从叶子 node 出发向上比较，返回该子树胜者段号 */
static int build_at(const LoserTree *lt, size_t node)
{
    if (node >= lt->size) {
        /* 叶子: 段号 = node - size */
        return lt->tree[node];
    }
    /* 内部结点: 先递归建左右子树得到两个胜者 */
    int left_winner = build_at(lt, 2 * node);
    int right_winner = build_at(lt, 2 * node + 1);
    int loser = 0;
    int winner = 0;

    if (lt->seg[left_winner] <= lt->seg[right_winner]) {
        winner = left_winner;
        loser = right_winner;
    } else {
        winner = right_winner;
        loser = left_winner;
    }
    lt->tree[node] = loser;
    return winner;
}

/* 从叶子 i 向上调整: 沿父链与已存败者比较，胜者上浮 */
static int adjust_from(const LoserTree *lt, size_t i)
{
    size_t node = lt->size + i;  /* 叶子下标 */
    int winner = (int)i;

    while (node > 1) {
        size_t parent = node / 2;
        int stored_loser = lt->tree[parent];

        if (lt->seg[winner] > lt->seg[stored_loser]) {
            /* 当前胜者败了: 存的败者变为胜者 */
            lt->tree[parent] = winner;
            winner = stored_loser;
        }
        /* else: 当前胜者仍是胜者，存的败者不变 */
        node = parent;
    }
    return winner;
}

DsResult lt_init(LoserTree *lt, int *tree_buf, int *seg, size_t k)
{
    size_t i = 0;
    if (lt == NULL || tree_buf == NULL || seg == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (k == 0) {
        return (DsResult){DS_OUT_OF_RANGE, "段数须 >= 1"};
    }
    lt->k    = k;
    lt->size = lt_size_needed(k);
    lt->tree = tree_buf;
    lt->seg  = seg;

    for (i = 0; i < lt->size; ++i) {
        lt->tree[lt->size + i] = (int)i;
    }
    for (i = k; i < lt->size; ++i) {
        lt->seg[i] = INT_MAX;
    }

    lt->tree[0] = (int)build_at(lt, 1);
    return (DsResult){DS_OK, "败者树初始化成功"};
}

DsResult lt_rebuild(LoserTree *lt)
{
    if (lt == NULL || lt->tree == NULL || lt->seg == NULL || lt->size == 0) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    lt->tree[0] = (int)build_at(lt, 1);
    return (DsResult){DS_OK, "重建成功"};
}

DsResult lt_winner(const LoserTree *lt, size_t *idx, int *val)
{
    if (lt == NULL || idx == NULL || val == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (lt->tree == NULL || lt->seg == NULL || lt->size == 0) {
        return (DsResult){DS_EMPTY, "败者树尚未初始化"};
    }
    *idx = (size_t)lt->tree[0];
    *val = lt->seg[*idx];
    return (DsResult){DS_OK, "取胜者成功"};
}

DsResult lt_adjust(LoserTree *lt, size_t i)
{
    if (lt == NULL || lt->tree == NULL || lt->seg == NULL || lt->size == 0) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (i >= lt->k) {
        return (DsResult){DS_OUT_OF_RANGE, "段号越界"};
    }
    lt->tree[0] = (int)adjust_from(lt, i);
    return (DsResult){DS_OK, "调整成功"};
}

DsResult lt_exhaust(LoserTree *lt, size_t i)
{
    if (lt == NULL || lt->seg == NULL || lt->tree == NULL || lt->size == 0) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (i >= lt->k) {
        return (DsResult){DS_OUT_OF_RANGE, "段号越界"};
    }
    lt->seg[i] = INT_MAX;
    return lt_adjust(lt, i);
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "位置/下标越界";
        case DS_OVERFLOW:    return "空间已满或内存分配失败";
        case DS_EMPTY:       return "对空结构取元素";
        case DS_NOT_FOUND:   return "查找未命中";
        default:             return "未知状态码";
    }
}
