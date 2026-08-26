/**
 * @file external_merge.c
 * @brief 外部 k 路归并实现：内嵌"败者树"进行 k 路选择
 *
 * 数组布局与 01_败者树 完全一致：外部结点在 [size,2*size)、内部结点存败者、
 * tree[0] 存整体胜者；段数 k 补位成不小于它的 2 次幂 size。
 *
 * EOF 语义：某文件读尽时把其段值置 INT_MAX，再 adjust，使该段永不再胜出；
 *           当取回的胜者值 == INT_MAX，说明所有文件均已 EOF，归并完成。
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "external_merge.h"


/* 不小于 k 的最小 2 次幂 */
static size_t pow2_size(size_t k)
{
    size_t s = 1;
    while (s < k) s <<= 1u;
    return s;
}

/* 自底向顶建树：返回 node 子树胜者段号，内部结点存败者 */
static size_t em_build(int *tree, const int *seg, size_t size, size_t node)
{
    if (node >= size) {
        return (size_t)tree[node];
    }
    {
        size_t wl = em_build(tree, seg, size, node * 2);
        size_t wr = em_build(tree, seg, size, node * 2 + 1);
        if (seg[wl] <= seg[wr]) {
            tree[node] = (int)wr;
            return wl;
        }
        tree[node] = (int)wl;
        return wr;
    }
}

/* 段 winner 值变化后由叶子向根调整，返回新胜者段号 */
static size_t em_adjust(int *tree, const int *seg, size_t size, size_t winner)
{
    size_t parent = (size + winner) / 2;
    while (parent >= 1) {
        size_t los = (size_t)tree[parent];
        if (seg[winner] > seg[los]) {
            tree[parent] = (int)winner;
            winner = los;
        }
        parent /= 2;
    }
    return winner;
}

DsResult external_merge(FILE **in, size_t k, FILE *out)
{
    size_t size = 0;
    int *tree = NULL;
    int *seg = NULL;
    size_t i = 0;

    if (in == NULL || out == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (k == 0) {
        return (DsResult){DS_OUT_OF_RANGE, "归并路数须 >= 1"};
    }

    size = pow2_size(k);
    tree = malloc(2u * size * sizeof *tree);
    seg = malloc(size * sizeof *seg);
    if (tree == NULL || seg == NULL) {
        free(tree);
        free(seg);
        return (DsResult){DS_OVERFLOW, "内存分配失败"};
    }

    /* 叶子：段号自身；读入各文件首元素作为段值 */
    for (i = 0; i < size; ++i) {
        tree[size + i] = (int)i;
        if (i < k) {
            int x = 0;
            seg[i] = (fscanf(in[i], "%d", &x) == 1) ? x : INT_MAX;
        } else {
            seg[i] = INT_MAX; /* 补位，永败 */
        }
    }

    tree[0] = (int)em_build(tree, seg, size, 1);

    for (;;) {
        size_t w = (size_t)tree[0];
        if (w >= k || seg[w] == INT_MAX) {
            break; /* 胜者已是 EOF => 全部结束 */
        }
        if (fprintf(out, "%d\n", seg[w]) < 0) {
            free(tree);
            free(seg);
            return (DsResult){DS_ERROR, "写入输出文件失败"};
        }
        {
            int x = 0;
            if (fscanf(in[w], "%d", &x) == 1) {
                seg[w] = x;
            } else {
                seg[w] = INT_MAX;
            }
        }
        tree[0] = (int)em_adjust(tree, seg, size, w);
    }

    free(tree);
    free(seg);
    return (DsResult){DS_OK, "外部多路归并完成"};
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "位置/下标越界";
        case DS_OVERFLOW:    return "内存分配失败";
        case DS_EMPTY:       return "对空结构取元素";
        case DS_NOT_FOUND:   return "查找未命中";
        default:             return "未知状态码";
    }
}
