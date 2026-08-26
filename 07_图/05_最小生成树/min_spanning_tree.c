/**
 * @file min_spanning_tree.c
 * @brief 最小生成树 Prim + Kruskal 接口实现
 *
 * 设计要点:
 *   1. Prim 的 lowcost 初值取第 0 行；INF 表示"已在树中或无边"，
 *      入选后置 INF 防重选
 *   2. Kruskal 内置并查集（find 路径压缩 + 按大小合并），
 *      与独立模块同型实现，保证模块自包含
 * 复杂度: Prim O(n^2)；Kruskal O(e·loge)
 */

#include <stdlib.h>

#include "min_spanning_tree.h"


DsResult mstg_create(MstGraph *g, size_t n)
{
    size_t i = 0;
    size_t j = 0;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (n == 0 || n > MST_MAX_VERTICES) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点数越界"};
    }

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            g->w[i][j] = MST_INF;
        }
    }
    g->n = n;
    g->edge_count = 0;
    return (DsResult){DS_OK, "MST图创建成功"};
}

DsResult mstg_add_edge(MstGraph *g, size_t u, size_t v, int w)
{
    int had = 0;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (u >= g->n || v >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }
    if (w >= MST_INF || w < 0) {
        return (DsResult){DS_ERROR, "权值须为非负且小于INF"};
    }

    had = (g->w[u][v] != MST_INF);
    g->w[u][v] = w;
    g->w[v][u] = w;
    if (!had) {
        g->edge_count++;
    }
    return (DsResult){DS_OK, "添加边成功"};
}

DsResult mstg_has_edge(const MstGraph *g, size_t u, size_t v, int *has)
{
    if (g == NULL || has == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n || v >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    *has = (g->w[u][v] != MST_INF);
    return (DsResult){DS_OK, "查询边成功"};
}

/* ---------- Prim ---------- */

DsResult mst_prim(const MstGraph *g, MstEdge edges[], size_t *count, long *total)
{
    int lowcost[MST_MAX_VERTICES];   /* v 到树点集的最小权（INF=不可达/已入选） */
    size_t nearest[MST_MAX_VERTICES]; /* lowcost 对应的树端点 */
    size_t in_tree[MST_MAX_VERTICES];
    size_t k = 0;
    size_t v = 0;

    if (g == NULL || edges == NULL || count == NULL || total == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (g->n < 2) {
        return (DsResult){DS_ERROR, "单点或空图无生成树"};
    }

    for (v = 0; v < g->n; v++) {
        lowcost[v] = g->w[0][v];
        nearest[v] = 0;
        in_tree[v] = 0;
    }
    in_tree[0] = 1; /* 0 号点起步 */
    lowcost[0] = -1; /* 已入选标记（区别 INF） */

    *count = 0;
    *total = 0;
    for (k = 0; k + 1 < g->n; k++) {
        size_t best = (size_t)-1;
        int best_w = MST_INF;

        /* 选树外 lowcost 最小的点 */
        for (v = 1; v < g->n; v++) {
            if (!in_tree[v] && lowcost[v] > 0 && lowcost[v] < best_w) {
                best_w = lowcost[v];
                best = v;
            }
        }
        if (best == (size_t)-1) {
            return (DsResult){DS_ERROR, "图不连通，无法生成MST"};
        }

        edges[*count].u = nearest[best];
        edges[*count].v = best;
        edges[*count].w = best_w;
        (*count)++;
        *total += best_w;

        in_tree[best] = 1;
        lowcost[best] = -1;

        /* 用新点松弛所有树外点 */
        for (v = 1; v < g->n; v++) {
            if (!in_tree[v] && g->w[best][v] < lowcost[v]) {
                lowcost[v] = g->w[best][v];
                nearest[v] = best;
            }
        }
    }
    return (DsResult){DS_OK, "Prim MST计算成功"};
}

/* ---------- Kruskal ---------- */

/* 内置并查集（同 06_树/07_并查集 算法） */
static size_t uf_find(size_t parent[], size_t x)
{
    size_t r = x;

    while (parent[r] != r) {
        r = parent[r];
    }
    while (parent[x] != r) {
        size_t next = parent[x];

        parent[x] = r;
        x = next;
    }
    return r;
}

/* qsort 比较器: 权值升序，平手 (u,v) 字典序保证确定性 */
static int edge_cmp(const void *a, const void *b)
{
    const MstEdge *ea = (const MstEdge *)a;
    const MstEdge *eb = (const MstEdge *)b;

    if (ea->w != eb->w) {
        return (ea->w < eb->w) ? -1 : 1;
    }
    if (ea->u != eb->u) {
        return (ea->u < eb->u) ? -1 : 1;
    }
    if (ea->v != eb->v) {
        return (ea->v < eb->v) ? -1 : 1;
    }
    return 0;
}

DsResult mst_kruskal(const MstGraph *g, MstEdge edges[], size_t *count, long *total)
{
    size_t parent[MST_MAX_VERTICES];
    size_t size_arr[MST_MAX_VERTICES];
    MstEdge all[MST_MAX_VERTICES * MST_MAX_VERTICES / 2];
    size_t e = 0;
    size_t i = 0;
    size_t j = 0;

    if (g == NULL || edges == NULL || count == NULL || total == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (g->n < 2) {
        return (DsResult){DS_ERROR, "单点或空图无生成树"};
    }

    /* 收集上三角边（无向每边一次） */
    for (i = 0; i < g->n; i++) {
        for (j = i + 1; j < g->n; j++) {
            if (g->w[i][j] != MST_INF) {
                all[e].u = i;
                all[e].v = j;
                all[e].w = g->w[i][j];
                e++;
            }
        }
    }

    qsort(all, e, sizeof(MstEdge), edge_cmp);

    for (i = 0; i < g->n; i++) {
        parent[i] = i;
        size_arr[i] = 1;
    }

    *count = 0;
    *total = 0;
    for (i = 0; i < e && *count < g->n - 1; i++) {
        size_t ru = uf_find(parent, all[i].u);
        size_t rv = uf_find(parent, all[i].v);

        if (ru == rv) {
            continue; /* 两端同集: 入选必成环，弃 */
        }
        /* 按大小合并 */
        if (size_arr[ru] < size_arr[rv]) {
            size_t tmp = ru;

            ru = rv;
            rv = tmp;
        }
        parent[rv] = ru;
        size_arr[ru] += size_arr[rv];

        edges[(*count)++] = all[i];
        *total += all[i].w;
    }

    if (*count != g->n - 1) {
        return (DsResult){DS_ERROR, "图不连通，无法生成MST"};
    }
    return (DsResult){DS_OK, "Kruskal MST计算成功"};
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
