/**
 * @file union_find.c
 * @brief 并查集接口实现
 *
 * 设计要点:
 *   1. find 的两趟式路径压缩: 先沿链找到根，再从 x 走一遍
 *      把沿途结点 parent 直指根。相比"隔代压缩"（路径减半）
 *      压得更平，且迭代实现无递归栈风险
 *   2. union 前两次 find 已顺带压缩，按大小挂接保证 O(log n) 高
 * 复杂度: init O(n)；find/union/connected 均摊 O(α(n))
 */

#include <stdlib.h>

#include "union_find.h"


DsResult uf_init(UnionFind *uf, size_t n)
{
    size_t i = 0;

    if (uf == NULL) {
        return (DsResult){DS_NULL_PTR, "并查集指针为空"};
    }
    if (n == 0) {
        return (DsResult){DS_OUT_OF_RANGE, "元素个数须大于0"};
    }

    uf->parent = (size_t *)malloc(n * sizeof(size_t));
    uf->size = (size_t *)malloc(n * sizeof(size_t));
    if (uf->parent == NULL || uf->size == NULL) {
        free(uf->parent);
        free(uf->size);
        uf->parent = NULL;
        uf->size = NULL;
        return (DsResult){DS_ERROR, "内存分配失败"};
    }

    for (i = 0; i < n; i++) {
        uf->parent[i] = i; /* 各自成根 */
        uf->size[i] = 1;
    }
    uf->n = n;
    uf->sets = n;
    return (DsResult){DS_OK, "并查集初始化成功"};
}

DsResult uf_destroy(UnionFind *uf)
{
    if (uf == NULL) {
        return (DsResult){DS_NULL_PTR, "并查集指针为空"};
    }

    free(uf->parent);
    free(uf->size);
    uf->parent = NULL;
    uf->size = NULL;
    uf->n = 0;
    uf->sets = 0;
    return (DsResult){DS_OK, "并查集销毁成功"};
}

DsResult uf_find(UnionFind *uf, size_t x, size_t *root)
{
    size_t r = 0;
    size_t p = 0;

    if (uf == NULL || root == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (x >= uf->n) {
        return (DsResult){DS_OUT_OF_RANGE, "元素下标越界"};
    }

    /* 第一趟: 沿双亲链找根（parent 指向自身者即根） */
    r = x;
    while (uf->parent[r] != r) {
        r = uf->parent[r];
    }

    /* 第二趟: 沿途直挂根（路径压缩） */
    p = x;
    while (uf->parent[p] != r) {
        size_t next = uf->parent[p];

        uf->parent[p] = r;
        p = next;
    }

    *root = r;
    return (DsResult){DS_OK, "查找成功"};
}

DsResult uf_union(UnionFind *uf, size_t x, size_t y, int *merged)
{
    size_t rx = 0;
    size_t ry = 0;

    if (uf == NULL) {
        return (DsResult){DS_NULL_PTR, "并查集指针为空"};
    }
    if (x >= uf->n || y >= uf->n) {
        return (DsResult){DS_OUT_OF_RANGE, "元素下标越界"};
    }

    if (uf_find(uf, x, &rx).code != DS_OK || uf_find(uf, y, &ry).code != DS_OK) {
        return (DsResult){DS_ERROR, "查找失败"};
    }
    if (rx == ry) {
        if (merged != NULL) {
            *merged = 0; /* 本就同集 */
        }
        return (DsResult){DS_OK, "已在同一集合，无需合并"};
    }

    /* 按大小合并: 小树挂大树（等大时任挂，取 ry 挂 rx） */
    if (uf->size[rx] < uf->size[ry]) {
        size_t tmp = rx;

        rx = ry;
        ry = tmp;
    }
    uf->parent[ry] = rx;
    uf->size[rx] += uf->size[ry];
    uf->sets--;

    if (merged != NULL) {
        *merged = 1;
    }
    return (DsResult){DS_OK, "合并成功"};
}

DsResult uf_connected(UnionFind *uf, size_t x, size_t y)
{
    size_t rx = 0;
    size_t ry = 0;

    if (uf == NULL) {
        return (DsResult){DS_NULL_PTR, "并查集指针为空"};
    }
    if (x >= uf->n || y >= uf->n) {
        return (DsResult){DS_OUT_OF_RANGE, "元素下标越界"};
    }

    uf_find(uf, x, &rx);
    uf_find(uf, y, &ry);
    return (rx == ry) ? (DsResult){DS_OK, "连通"}
                      : (DsResult){DS_ERROR, "不连通"};
}

size_t uf_set_count(const UnionFind *uf)
{
    return (uf == NULL) ? 0 : uf->sets;
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "下标越界";
        default:             return "未知状态码";
    }
}
