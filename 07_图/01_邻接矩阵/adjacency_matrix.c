/**
 * @file adjacency_matrix.c
 * @brief 图的邻接矩阵存储接口实现
 *
 * 设计要点:
 *   1. 无向图一切写操作对称化（set/remove），读操作自然成立
 *   2. DFS 递归 + mg_neighbors 的编号序保证访问序确定，
 *      便于与邻接表模块对拍
 * 复杂度: 见头文件各 API 注释
 */

#include <stdio.h>

#include "adjacency_matrix.h"


DsResult mg_create(MatrixGraph *g, size_t n, int directed)
{
    size_t i = 0;
    size_t j = 0;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (n == 0 || n > MG_MAX_VERTICES) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点数越界"};
    }

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            g->weight[i][j] = MG_INF;
        }
    }
    g->n = n;
    g->directed = directed;
    g->edge_count = 0;
    return (DsResult){DS_OK, "邻接矩阵图创建成功"};
}

size_t mg_vertex_count(const MatrixGraph *g)
{
    return (g == NULL) ? 0 : g->n;
}

size_t mg_edge_count(const MatrixGraph *g)
{
    return (g == NULL) ? 0 : g->edge_count;
}

DsResult mg_set_edge(MatrixGraph *g, size_t u, size_t v, int w)
{
    int had = 0;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (u >= g->n || v >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }
    if (w == MG_INF) {
        return mg_remove_edge(g, u, v);
    }

    had = (g->weight[u][v] != MG_INF);
    g->weight[u][v] = w;
    if (!g->directed) {
        g->weight[v][u] = w; /* 无向: 对称不变量 */
    }
    if (!had) {
        g->edge_count++;
    }
    return (DsResult){DS_OK, "设置边成功"};
}

DsResult mg_remove_edge(MatrixGraph *g, size_t u, size_t v)
{
    int had = 0;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (u >= g->n || v >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    had = (g->weight[u][v] != MG_INF);
    g->weight[u][v] = MG_INF;
    if (!g->directed) {
        g->weight[v][u] = MG_INF;
    }
    if (had) {
        g->edge_count--;
    }
    return (DsResult){DS_OK, "删除边成功"};
}

DsResult mg_has_edge(const MatrixGraph *g, size_t u, size_t v, int *has)
{
    if (g == NULL || has == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n || v >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    *has = (g->weight[u][v] != MG_INF);
    return (DsResult){DS_OK, "查询边成功"};
}

DsResult mg_get_weight(const MatrixGraph *g, size_t u, size_t v, int *w)
{
    if (g == NULL || w == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n || v >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    *w = g->weight[u][v];
    return (DsResult){DS_OK, "获取权值成功"};
}

DsResult mg_neighbors(const MatrixGraph *g, size_t u, size_t out[], size_t *count)
{
    size_t j = 0;

    if (g == NULL || out == NULL || count == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    *count = 0;
    for (j = 0; j < g->n; j++) {
        if (g->weight[u][j] != MG_INF) {
            out[(*count)++] = j; /* 行扫描即编号升序 */
        }
    }
    return (DsResult){DS_OK, "枚举邻居成功"};
}

DsResult mg_out_degree(const MatrixGraph *g, size_t u, size_t *deg)
{
    size_t j = 0;

    if (g == NULL || deg == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    *deg = 0;
    for (j = 0; j < g->n; j++) {
        if (g->weight[u][j] != MG_INF) {
            (*deg)++;
        }
    }
    return (DsResult){DS_OK, "计算出度成功"};
}

DsResult mg_in_degree(const MatrixGraph *g, size_t u, size_t *deg)
{
    size_t i = 0;

    if (g == NULL || deg == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    *deg = 0;
    for (i = 0; i < g->n; i++) {
        if (g->weight[i][u] != MG_INF) {
            (*deg)++;
        }
    }
    return (DsResult){DS_OK, "计算入度成功"};
}

/* DFS 递归辅助 */
static void dfs_rec(const MatrixGraph *g, size_t u, int visited[], size_t out[], size_t *count)
{
    size_t j = 0;

    visited[u] = 1;
    out[(*count)++] = u; /* 首次到达即访问 */
    for (j = 0; j < g->n; j++) {
        if (g->weight[u][j] != MG_INF && !visited[j]) {
            dfs_rec(g, j, visited, out, count);
        }
    }
}

DsResult mg_dfs(const MatrixGraph *g, size_t start, size_t out[], size_t *count)
{
    static int visited[MG_MAX_VERTICES];

    if (g == NULL || out == NULL || count == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (start >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "起点下标越界"};
    }

    {
        size_t i = 0;

        for (i = 0; i < g->n; i++) {
            visited[i] = 0;
        }
    }
    *count = 0;
    dfs_rec(g, start, visited, out, count);
    return (DsResult){DS_OK, "DFS遍历完成"};
}

DsResult mg_bfs(const MatrixGraph *g, size_t start, size_t out[], size_t *count)
{
    static size_t queue[MG_MAX_VERTICES];
    static int visited[MG_MAX_VERTICES];
    size_t head = 0;
    size_t tail = 0;

    if (g == NULL || out == NULL || count == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (start >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "起点下标越界"};
    }

    {
        size_t i = 0;

        for (i = 0; i < g->n; i++) {
            visited[i] = 0;
        }
    }

    *count = 0;
    visited[start] = 1; /* 入队即标记，防止重复入队 */
    queue[tail++] = start;
    while (head < tail) {
        size_t u = queue[head++];
        size_t j = 0;

        out[(*count)++] = u;
        for (j = 0; j < g->n; j++) {
            if (g->weight[u][j] != MG_INF && !visited[j]) {
                visited[j] = 1;
                queue[tail++] = j;
            }
        }
    }
    return (DsResult){DS_OK, "BFS遍历完成"};
}

DsResult mg_print(const MatrixGraph *g)
{
    size_t i = 0;
    size_t j = 0;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }

    printf("邻接矩阵(%s, %zu 顶点, %zu 边):\n     ",
           g->directed ? "有向" : "无向", g->n, g->edge_count);
    for (j = 0; j < g->n; j++) {
        printf("%4zu", j);
    }
    printf("\n");
    for (i = 0; i < g->n; i++) {
        printf("  %2zu:", i);
        for (j = 0; j < g->n; j++) {
            if (g->weight[i][j] == MG_INF) {
                printf("   .");
            } else {
                printf("%4d", g->weight[i][j]);
            }
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
