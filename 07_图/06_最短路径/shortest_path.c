/**
 * @file shortest_path.c
 * @brief Dijkstra + Floyd 接口实现
 *
 * 设计要点:
 *   1. Dijkstra 入口做一次全矩阵负权检查（教学模块代价可接受），
 *      保证算法前提成立而非静默给出错误结果
 *   2. Floyd 的 next 矩阵初始化为直连后继，松弛成功时同步更新
 *      next[i][j] = next[i][k] —— 保证重建的路径经过中转点 k
 * 复杂度: Dijkstra O(n^2)；Floyd O(n^3)
 */

#include "shortest_path.h"


DsResult spg_create(SpGraph *g, size_t n)
{
    size_t i = 0;
    size_t j = 0;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (n == 0 || n > SP_MAX_VERTICES) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点数越界"};
    }

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            g->w[i][j] = SP_INF;
        }
    }
    g->n = n;
    return (DsResult){DS_OK, "有向网创建成功"};
}

DsResult spg_add_arc(SpGraph *g, size_t u, size_t v, int w)
{
    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (u >= g->n || v >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }
    if (w < 0 || w >= SP_INF) {
        return (DsResult){DS_ERROR, "权值须非负且小于INF"};
    }

    g->w[u][v] = w;
    return (DsResult){DS_OK, "添加弧成功"};
}

DsResult sp_dijkstra(const SpGraph *g, size_t src, int dist[], size_t prev[])
{
    int done[SP_MAX_VERTICES];
    size_t i = 0;
    size_t k = 0;

    if (g == NULL || dist == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (src >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "源点下标越界"};
    }

    /* 负权检查: 贪心前提（负弧会使已确定点仍可被改进） */
    for (i = 0; i < g->n; i++) {
        size_t j = 0;

        for (j = 0; j < g->n; j++) {
            if (g->w[i][j] < 0) {
                return (DsResult){DS_ERROR, "存在负权弧，Dijkstra不适用"};
            }
        }
    }

    for (i = 0; i < g->n; i++) {
        dist[i] = SP_INF;
        done[i] = 0;
        if (prev != NULL) {
            prev[i] = (size_t)-1;
        }
    }
    dist[src] = 0;

    for (k = 0; k < g->n; k++) {
        size_t best = (size_t)-1;
        int best_d = SP_INF;
        size_t v = 0;

        /* 取集外 dist 最小点（首个并列最小者） */
        for (v = 0; v < g->n; v++) {
            if (!done[v] && dist[v] < best_d) {
                best_d = dist[v];
                best = v;
            }
        }
        if (best == (size_t)-1) {
            break; /* 剩余点均不可达 */
        }
        done[best] = 1;

        /* 出边松弛 */
        for (v = 0; v < g->n; v++) {
            if (g->w[best][v] != SP_INF && !done[v] &&
                dist[best] + g->w[best][v] < dist[v]) {
                dist[v] = dist[best] + g->w[best][v];
                if (prev != NULL) {
                    prev[v] = best;
                }
            }
        }
    }
    return (DsResult){DS_OK, "Dijkstra计算成功"};
}

DsResult sp_floyd(const SpGraph *g, int dist[][SP_MAX_VERTICES],
                  size_t next[][SP_MAX_VERTICES])
{
    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    if (g == NULL || dist == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    /* 初始化: 直连距离与直连后继 */
    for (i = 0; i < g->n; i++) {
        for (j = 0; j < g->n; j++) {
            dist[i][j] = g->w[i][j];
            if (next != NULL) {
                next[i][j] = (g->w[i][j] != SP_INF) ? j : (size_t)-1;
            }
        }
        dist[i][i] = 0; /* 自身到自身 0（覆盖可能的自环弧） */
        if (next != NULL) {
            next[i][i] = i;
        }
    }

    /* 中转点在最外层: dist 已是"允许经过 0..k-1 中转"的解，逐个放行 k */
    for (k = 0; k < g->n; k++) {
        for (i = 0; i < g->n; i++) {
            for (j = 0; j < g->n; j++) {
                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    if (next != NULL) {
                        next[i][j] = next[i][k]; /* 借道 k: 先走向 k 的第一步 */
                    }
                }
            }
        }
    }
    return (DsResult){DS_OK, "Floyd计算成功"};
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
