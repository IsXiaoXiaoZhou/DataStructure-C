/**
 * @file topological_sort.c
 * @brief 拓扑排序（Kahn + DFS 逆后序）接口实现
 *
 * 设计要点:
 *   1. Kahn 实现不显式建队列: 每轮线性扫"未输出且入度为 0"
 *      的最小编号点 —— 教学上突出"入度削减"本质
 *   2. DFS 法 color 三值: 0 未访问 / 1 进行中(灰) / 2 完成(黑)；
 *      递归返回前压栈（finish 栈），最后整体反转
 * 复杂度: 两法均 O(n^2)（邻接矩阵）；邻接表版 O(n+e)
 */

#include "topological_sort.h"


DsResult tg_create(TopoGraph *g, size_t n)
{
    size_t i = 0;
    size_t j = 0;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (n == 0 || n > TOPO_MAX_VERTICES) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点数越界"};
    }

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            g->adj[i][j] = 0;
        }
    }
    g->n = n;
    g->arc_count = 0;
    return (DsResult){DS_OK, "AOV网创建成功"};
}

DsResult tg_add_arc(TopoGraph *g, size_t u, size_t v)
{
    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (u >= g->n || v >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    if (!g->adj[u][v]) {
        g->adj[u][v] = 1;
        g->arc_count++;
    }
    return (DsResult){DS_OK, "添加弧成功"};
}

DsResult tg_has_arc(const TopoGraph *g, size_t u, size_t v, int *has)
{
    if (g == NULL || has == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n || v >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    *has = g->adj[u][v];
    return (DsResult){DS_OK, "查询弧成功"};
}

/* ---------- Kahn 入度削减法 ---------- */

DsResult topo_sort_kahn(const TopoGraph *g, size_t order[], size_t *count)
{
    size_t indeg[TOPO_MAX_VERTICES];
    size_t emitted[TOPO_MAX_VERTICES];
    size_t i = 0;
    size_t k = 0;

    if (g == NULL || order == NULL || count == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    /* 入度统计: 第 v 列的弧数 */
    for (i = 0; i < g->n; i++) {
        size_t d = 0;
        size_t r = 0;

        for (r = 0; r < g->n; r++) {
            if (g->adj[r][i]) {
                d++;
            }
        }
        indeg[i] = d;
        emitted[i] = 0;
    }

    *count = 0;
    for (k = 0; k < g->n; k++) {
        size_t pick = (size_t)-1;

        /* 取编号最小的 0 入度未输出点 */
        for (i = 0; i < g->n; i++) {
            if (!emitted[i] && indeg[i] == 0) {
                pick = i;
                break;
            }
        }
        if (pick == (size_t)-1) {
            return (DsResult){DS_ERROR, "存在有向环，无法完成拓扑排序"};
        }

        order[(*count)++] = pick;
        emitted[pick] = 1;
        for (i = 0; i < g->n; i++) {
            if (g->adj[pick][i]) {
                indeg[i]--; /* 削减后继入度 */
            }
        }
    }
    return (DsResult){DS_OK, "Kahn拓扑排序成功"};
}

/* ---------- DFS 逆后序法 ---------- */

/* DFS 辅助: 返回 1 表示发现环 */
static int dfs_visit(const TopoGraph *g, size_t u, int color[],
                     size_t finish[], size_t *ftop)
{
    size_t v = 0;

    color[u] = 1; /* 灰: 进行中 */

    for (v = 0; v < g->n; v++) {
        if (!g->adj[u][v]) {
            continue;
        }
        if (color[v] == 1) {
            return 1; /* 灰边 -> 回边 -> 有环 */
        }
        if (color[v] == 0) {
            if (dfs_visit(g, v, color, finish, ftop)) {
                return 1;
            }
        }
        /* color==2 黑: 已完成，前向/交叉边，安全 */
    }

    color[u] = 2;         /* 黑: 完成 */
    finish[(*ftop)++] = u; /* 完成序入栈 */
    return 0;
}

DsResult topo_sort_dfs(const TopoGraph *g, size_t order[], size_t *count)
{
    int color[TOPO_MAX_VERTICES];
    size_t finish[TOPO_MAX_VERTICES];
    size_t ftop = 0;
    size_t i = 0;

    if (g == NULL || order == NULL || count == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    for (i = 0; i < g->n; i++) {
        color[i] = 0;
    }

    for (i = 0; i < g->n; i++) {
        if (color[i] == 0) {
            if (dfs_visit(g, i, color, finish, &ftop)) {
                return (DsResult){DS_ERROR, "存在有向环，无法完成拓扑排序"};
            }
        }
    }

    /* 完成序的逆序 = 拓扑序 */
    *count = g->n;
    for (i = 0; i < g->n; i++) {
        order[i] = finish[g->n - 1 - i];
    }
    return (DsResult){DS_OK, "DFS拓扑排序成功"};
}

/* ---------- 验证工具 ---------- */

DsResult topo_verify(const TopoGraph *g, const size_t order[], size_t count)
{
    size_t pos[TOPO_MAX_VERTICES];
    size_t i = 0;
    size_t j = 0;

    if (g == NULL || order == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (count != g->n) {
        return (DsResult){DS_ERROR, "序列长度不等于顶点数"};
    }

    for (i = 0; i < count; i++) {
        if (order[i] >= g->n) {
            return (DsResult){DS_ERROR, "序列含非法顶点"};
        }
        pos[order[i]] = i;
    }

    /* 每条弧 (u,v): u 必须在 v 之前 */
    for (i = 0; i < g->n; i++) {
        for (j = 0; j < g->n; j++) {
            if (g->adj[i][j] && pos[i] >= pos[j]) {
                return (DsResult){DS_ERROR, "拓扑序不合法"};
            }
        }
    }
    return (DsResult){DS_OK, "拓扑序合法"};
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
