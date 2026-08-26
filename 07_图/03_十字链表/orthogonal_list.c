/**
 * @file orthogonal_list.c
 * @brief 有向图十字链表存储接口实现
 *
 * 设计要点:
 *   1. add_arc 头插: 新弧成为 first_out[tail] 与 first_in[head]
 *      的新链头，链序为"新弧在前"（逆插入序）
 *   2. remove_arc 用二级指针分别在两条链上定位同一条弧结点
 *      （tailvex/headvex 双字段匹配），再一次性 free ——
 *      双链共享结点的删除必须两链同步，否则悬垂
 * 复杂度: 插 O(1)；判/删 O(deg)；度数 O(deg)
 */

#include <stdio.h>
#include <stdlib.h>

#include "orthogonal_list.h"


DsResult og_create(OrthGraph *g, size_t n)
{
    size_t i = 0;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (n == 0 || n > OL_MAX_VERTICES) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点数越界"};
    }

    for (i = 0; i < n; i++) {
        g->first_in[i] = NULL;
        g->first_out[i] = NULL;
    }
    g->n = n;
    g->arc_count = 0;
    return (DsResult){DS_OK, "十字链表图创建成功"};
}

DsResult og_destroy(OrthGraph *g)
{
    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }

    /*
     * 每条弧只有一个结点，仅沿出弧链释放即可覆盖全部弧；
     * 若同时遍历入弧链会 double free
     */
    {
        size_t i = 0;

        for (i = 0; i < g->n; i++) {
            OrthArcNode *p = g->first_out[i];

            while (p != NULL) {
                OrthArcNode *next = p->tlink;

                free(p);
                p = next;
            }
            g->first_out[i] = NULL;
            g->first_in[i] = NULL;
        }
    }
    g->n = 0;
    g->arc_count = 0;
    return (DsResult){DS_OK, "十字链表图销毁成功"};
}

size_t og_vertex_count(const OrthGraph *g)
{
    return (g == NULL) ? 0 : g->n;
}

size_t og_arc_count(const OrthGraph *g)
{
    return (g == NULL) ? 0 : g->arc_count;
}

DsResult og_add_arc(OrthGraph *g, size_t tail, size_t head, int w)
{
    OrthArcNode *arc = NULL;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (tail >= g->n || head >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    /* 已存在则改权（简单弧图语义，无重弧） */
    {
        OrthArcNode *p = g->first_out[tail];

        while (p != NULL) {
            if (p->headvex == head) {
                p->weight = w;
                return (DsResult){DS_OK, "弧已存在，更新权值"};
            }
            p = p->tlink;
        }
    }

    arc = (OrthArcNode *)malloc(sizeof(OrthArcNode));
    if (arc == NULL) {
        return (DsResult){DS_ERROR, "弧结点分配失败"};
    }
    arc->tailvex = tail;
    arc->headvex = head;
    arc->weight = w;

    /* 头插入弧链与出弧链: 各自成为新链头 */
    arc->tlink = g->first_out[tail];
    g->first_out[tail] = arc;
    arc->hlink = g->first_in[head];
    g->first_in[head] = arc;

    g->arc_count++;
    return (DsResult){DS_OK, "添加弧成功"};
}

/* 在出弧链定位 (tail,head) 的二级指针，未找到返回 NULL */
static OrthArcNode **locate_in_out(OrthGraph *g, size_t tail, size_t head)
{
    OrthArcNode **pp = &g->first_out[tail];

    while (*pp != NULL) {
        if ((*pp)->headvex == head) {
            return pp;
        }
        pp = &(*pp)->tlink;
    }
    return NULL;
}

/* 在入弧链定位 (tail,head) 的二级指针 */
static OrthArcNode **locate_in_in(OrthGraph *g, size_t tail, size_t head)
{
    OrthArcNode **pp = &g->first_in[head];

    while (*pp != NULL) {
        if ((*pp)->tailvex == tail) {
            return pp;
        }
        pp = &(*pp)->hlink;
    }
    return NULL;
}

DsResult og_remove_arc(OrthGraph *g, size_t tail, size_t head)
{
    OrthArcNode **out_pos = NULL;
    OrthArcNode **in_pos = NULL;
    OrthArcNode *arc = NULL;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (tail >= g->n || head >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    out_pos = locate_in_out(g, tail, head);
    in_pos = locate_in_in(g, tail, head);
    if (out_pos == NULL || in_pos == NULL) {
        return (DsResult){DS_ERROR, "无此弧"}; /* 两链必须同时命中同一弧 */
    }

    arc = *out_pos;
    *out_pos = arc->tlink; /* 出弧链摘除 */
    *in_pos = arc->hlink;  /* 入弧链摘除 */
    free(arc);
    g->arc_count--;
    return (DsResult){DS_OK, "删除弧成功"};
}

DsResult og_has_arc(const OrthGraph *g, size_t tail, size_t head, int *has)
{
    const OrthArcNode *p = NULL;

    if (g == NULL || has == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (tail >= g->n || head >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    for (p = g->first_out[tail]; p != NULL; p = p->tlink) {
        if (p->headvex == head) {
            *has = 1;
            return (DsResult){DS_OK, "查询弧成功"};
        }
    }
    *has = 0;
    return (DsResult){DS_OK, "查询弧成功"};
}

DsResult og_get_weight(const OrthGraph *g, size_t tail, size_t head, int *w)
{
    const OrthArcNode *p = NULL;

    if (g == NULL || w == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (tail >= g->n || head >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    for (p = g->first_out[tail]; p != NULL; p = p->tlink) {
        if (p->headvex == head) {
            *w = p->weight;
            return (DsResult){DS_OK, "获取权值成功"};
        }
    }
    return (DsResult){DS_ERROR, "无此弧"};
}

DsResult og_out_arcs(const OrthGraph *g, size_t u, size_t heads[], size_t *count)
{
    const OrthArcNode *p = NULL;

    if (g == NULL || heads == NULL || count == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    *count = 0;
    for (p = g->first_out[u]; p != NULL; p = p->tlink) {
        heads[(*count)++] = p->headvex;
    }
    return (DsResult){DS_OK, "枚举出弧成功"};
}

DsResult og_in_arcs(const OrthGraph *g, size_t u, size_t tails[], size_t *count)
{
    const OrthArcNode *p = NULL;

    if (g == NULL || tails == NULL || count == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    *count = 0;
    for (p = g->first_in[u]; p != NULL; p = p->hlink) {
        tails[(*count)++] = p->tailvex;
    }
    return (DsResult){DS_OK, "枚举入弧成功"};
}

DsResult og_out_degree(const OrthGraph *g, size_t u, size_t *deg)
{
    const OrthArcNode *p = NULL;

    if (g == NULL || deg == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    *deg = 0;
    for (p = g->first_out[u]; p != NULL; p = p->tlink) {
        (*deg)++;
    }
    return (DsResult){DS_OK, "计算出度成功"};
}

DsResult og_in_degree(const OrthGraph *g, size_t u, size_t *deg)
{
    const OrthArcNode *p = NULL;

    if (g == NULL || deg == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    *deg = 0;
    for (p = g->first_in[u]; p != NULL; p = p->hlink) {
        (*deg)++;
    }
    return (DsResult){DS_OK, "计算入度成功"};
}

DsResult og_print(const OrthGraph *g)
{
    size_t i = 0;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }

    printf("十字链表(%zu 顶点, %zu 弧):\n", g->n, g->arc_count);
    for (i = 0; i < g->n; i++) {
        const OrthArcNode *p = NULL;

        printf("  顶点 %zu 出弧:", i);
        for (p = g->first_out[i]; p != NULL; p = p->tlink) {
            printf(" ->%zu(w=%d)", p->headvex, p->weight);
        }
        printf(" | 入弧:");
        for (p = g->first_in[i]; p != NULL; p = p->hlink) {
            printf(" %zu->", p->tailvex);
        }
        printf("\n");
    }
    return (DsResult){DS_OK, "打印完成"};
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "顶点下标越界";
        default:             return "未知状态码";
    }
}
