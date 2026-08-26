/**
 * @file adjacency_multilist.c
 * @brief 无向图邻接多重表存储接口实现
 *
 * 设计要点:
 *   1. "看路牌走链": 边结点不重复存 v 的邻居信息，
 *      next_of(edge, v) = (edge->ivex == v) ? ilink : jlink
 *   2. 摘除边 (u,v): 在 u 依附链上用二级指针定位该边
 *      （另一端==v），替换为 next_of(...,u)；v 链同理。
 *      自环 u==v 时两条链是同一条，只摘一次
 * 复杂度: 插 O(1)；判/删 O(deg)
 */

#include <stdio.h>
#include <stdlib.h>

#include "adjacency_multilist.h"


/* 走链辅助: e 边依附于 v，返回依附链上 e 的后继 */
static AmlEdgeNode *next_of(const AmlEdgeNode *e, size_t v)
{
    return (e->ivex == v) ? e->ilink : e->jlink;
}

DsResult aml_create(AmlGraph *g, size_t n)
{
    size_t i = 0;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (n == 0 || n > AML_MAX_VERTICES) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点数越界"};
    }

    for (i = 0; i < n; i++) {
        g->first_edge[i] = NULL;
    }
    g->n = n;
    g->edge_count = 0;
    return (DsResult){DS_OK, "邻接多重表创建成功"};
}

DsResult aml_destroy(AmlGraph *g)
{
    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }

    /* 沿每个顶点链头循环删除，直到所有边释放完毕 */
    {
        size_t i = 0;

        for (i = 0; i < g->n; i++) {
            while (g->first_edge[i] != NULL) {
                AmlEdgeNode *e = g->first_edge[i];

                /* 删掉该链头边（两端同步摘除），循环至链空 */
                aml_remove_edge(g, e->ivex, e->jvex);
            }
        }
    }
    g->n = 0;
    return (DsResult){DS_OK, "邻接多重表销毁成功"};
}

size_t aml_vertex_count(const AmlGraph *g)
{
    return (g == NULL) ? 0 : g->n;
}

size_t aml_edge_count(const AmlGraph *g)
{
    return (g == NULL) ? 0 : g->edge_count;
}

DsResult aml_set_edge(AmlGraph *g, size_t u, size_t v, int w)
{
    AmlEdgeNode *edge = NULL;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (u >= g->n || v >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    /* 已存在则改权 */
    {
        const AmlEdgeNode *p = g->first_edge[u];

        while (p != NULL) {
            if ((p->ivex == u && p->jvex == v) || (p->ivex == v && p->jvex == u)) {
                ((AmlEdgeNode *)p)->weight = w;
                return (DsResult){DS_OK, "边已存在，更新权值"};
            }
            p = next_of(p, u);
        }
    }

    edge = (AmlEdgeNode *)malloc(sizeof(AmlEdgeNode));
    if (edge == NULL) {
        return (DsResult){DS_ERROR, "边结点分配失败"};
    }
    edge->ivex = u;
    edge->jvex = v;
    edge->weight = w;

    /* 头插两条依附链；自环时两次头插同一条链（形成单结点环，合法） */
    edge->ilink = g->first_edge[u];
    g->first_edge[u] = edge;
    if (u != v) {
        edge->jlink = g->first_edge[v];
        g->first_edge[v] = edge;
    } else {
        edge->jlink = edge->ilink; /* 自环: j 链与 i 链同一条 */
    }

    g->edge_count++;
    return (DsResult){DS_OK, "添加边成功"};
}

/* 在 v 的依附链上定位"另一端为 other"的边结点的二级指针 */
static AmlEdgeNode **locate(AmlGraph *g, size_t v, size_t other)
{
    AmlEdgeNode **pp = &g->first_edge[v];

    while (*pp != NULL) {
        size_t peer = ((*pp)->ivex == v) ? (*pp)->jvex : (*pp)->ivex;

        if (peer == other) {
            return pp;
        }
        pp = (AmlEdgeNode **)((*pp)->ivex == v ? &(*pp)->ilink : &(*pp)->jlink);
    }
    return NULL;
}

DsResult aml_remove_edge(AmlGraph *g, size_t u, size_t v)
{
    AmlEdgeNode **pos_u = NULL;
    AmlEdgeNode **pos_v = NULL;
    AmlEdgeNode *edge = NULL;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (u >= g->n || v >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    pos_u = locate(g, u, v);
    if (pos_u == NULL) {
        return (DsResult){DS_ERROR, "无此边"};
    }
    edge = *pos_u;

    if (u != v) {
        pos_v = locate(g, v, u);
        if (pos_v == NULL) {
            return (DsResult){DS_ERROR, "内部不变量破坏"};
        }
        *pos_v = next_of(edge, v); /* v 链摘除 */
    }
    *pos_u = next_of(edge, u);     /* u 链摘除 */

    free(edge);
    g->edge_count--;
    return (DsResult){DS_OK, "删除边成功"};
}

DsResult aml_has_edge(const AmlGraph *g, size_t u, size_t v, int *has)
{
    const AmlEdgeNode *p = NULL;

    if (g == NULL || has == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n || v >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    for (p = g->first_edge[u]; p != NULL; p = next_of(p, u)) {
        size_t peer = (p->ivex == u) ? p->jvex : p->ivex;

        if (peer == v) {
            *has = 1;
            return (DsResult){DS_OK, "查询边成功"};
        }
    }
    *has = 0;
    return (DsResult){DS_OK, "查询边成功"};
}

DsResult aml_get_weight(const AmlGraph *g, size_t u, size_t v, int *w)
{
    const AmlEdgeNode *p = NULL;

    if (g == NULL || w == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n || v >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    for (p = g->first_edge[u]; p != NULL; p = next_of(p, u)) {
        size_t peer = (p->ivex == u) ? p->jvex : p->ivex;

        if (peer == v) {
            *w = p->weight;
            return (DsResult){DS_OK, "获取权值成功"};
        }
    }
    return (DsResult){DS_ERROR, "无此边"};
}

DsResult aml_neighbors(const AmlGraph *g, size_t u, size_t out[], size_t *count)
{
    const AmlEdgeNode *p = NULL;

    if (g == NULL || out == NULL || count == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    *count = 0;
    for (p = g->first_edge[u]; p != NULL; p = next_of(p, u)) {
        /* 邻居 = 边的另一端（自环时另一端即自己，计一次） */
        out[(*count)++] = (p->ivex == u) ? p->jvex : p->ivex;
    }
    return (DsResult){DS_OK, "枚举邻居成功"};
}

DsResult aml_degree(const AmlGraph *g, size_t u, size_t *deg)
{
    const AmlEdgeNode *p = NULL;

    if (g == NULL || deg == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    *deg = 0;
    for (p = g->first_edge[u]; p != NULL; p = next_of(p, u)) {
        (*deg)++;
    }
    return (DsResult){DS_OK, "计算度数成功"};
}

DsResult aml_print(const AmlGraph *g)
{
    size_t i = 0;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }

    printf("邻接多重表(%zu 顶点, %zu 边):\n", g->n, g->edge_count);
    for (i = 0; i < g->n; i++) {
        const AmlEdgeNode *p = NULL;

        printf("  顶点 %zu 依附边:", i);
        for (p = g->first_edge[i]; p != NULL; p = next_of(p, i)) {
            size_t peer = (p->ivex == i) ? p->jvex : p->ivex;

            printf(" %zu-%zu(w=%d)", i, peer, p->weight);
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
