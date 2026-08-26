/**
 * @file critical_path.c
 * @brief AOE 网关键路径接口实现
 *
 * 设计要点:
 *   1. 单独写一遍 Kahn 拓扑（本模块的基础设施）并保留 topo 序，
 *      正推 ve 沿序取 max，逆序推 vl 取 min —— 两次线性扫描
 *   2. 源点唯一性: 入度 0 者恰一个；汇点: 出度 0 者恰一个。
 *      多源/多汇在真实工程应加超源超汇，本模块按错误处理
 *   3. vl 初始化为 ve[汇]（汇点 vl = ve），逆推其余点
 * 复杂度: O(n^2)（邻接矩阵教学实现）
 */

#include <string.h>

#include "critical_path.h"


DsResult aoe_create(AoeGraph *g, size_t n)
{
    size_t i = 0;
    size_t j = 0;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (n == 0 || n > CP_MAX_VERTICES) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点数越界"};
    }

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            g->w[i][j] = CP_INF;
        }
    }
    g->n = n;
    g->arc_count = 0;
    return (DsResult){DS_OK, "AOE网创建成功"};
}

DsResult aoe_add_arc(AoeGraph *g, size_t u, size_t v, int w)
{
    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (u >= g->n || v >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }
    if (w <= 0) {
        return (DsResult){DS_ERROR, "活动耗时须为正"};
    }

    if (g->w[u][v] == CP_INF) {
        g->arc_count++;
    }
    g->w[u][v] = w;
    return (DsResult){DS_OK, "添加弧成功"};
}

DsResult cp_analyze(const AoeGraph *g, long ve[], long vl[],
                    CpActivity acts[], size_t *act_count, long *duration)
{
    size_t indeg[CP_MAX_VERTICES];
    size_t topo[CP_MAX_VERTICES];
    size_t tcount = 0;
    size_t src = (size_t)-1;
    size_t dst = (size_t)-1;
    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    if (g == NULL || ve == NULL || vl == NULL || acts == NULL ||
        act_count == NULL || duration == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    /* 度数统计 + 源汇识别 */
    for (i = 0; i < g->n; i++) {
        size_t d_in = 0;
        size_t d_out = 0;

        for (j = 0; j < g->n; j++) {
            if (g->w[j][i] != CP_INF) {
                d_in++;
            }
            if (g->w[i][j] != CP_INF) {
                d_out++;
            }
        }
        indeg[i] = d_in;
        if (d_in == 0) {
            if (src != (size_t)-1) {
                return (DsResult){DS_ERROR, "存在多个源点"};
            }
            src = i;
        }
        if (d_out == 0) {
            if (dst != (size_t)-1) {
                return (DsResult){DS_ERROR, "存在多个汇点"};
            }
            dst = i;
        }
    }
    if (src == (size_t)-1 || dst == (size_t)-1) {
        return (DsResult){DS_ERROR, "无源点或无汇点"};
    }

    /* Kahn 拓扑 */
    {
        size_t deg[CP_MAX_VERTICES];

        memcpy(deg, indeg, sizeof(deg[0]) * g->n);
        for (k = 0; k < g->n; k++) {
            size_t pick = (size_t)-1;

            for (i = 0; i < g->n; i++) {
                if (deg[i] == 0) {
                    pick = i;
                    break;
                }
            }
            if (pick == (size_t)-1) {
                return (DsResult){DS_ERROR, "存在有向环"};
            }
            deg[pick] = (size_t)-1; /* 已输出标记 */
            topo[tcount++] = pick;
            for (j = 0; j < g->n; j++) {
                if (g->w[pick][j] != CP_INF && deg[j] != (size_t)-1) {
                    deg[j]--;
                }
            }
        }
    }

    /* 正推 ve: ve[v] = max(ve[u] + w(u,v))，拓扑序保证 u 先就绪 */
    for (i = 0; i < g->n; i++) {
        ve[i] = 0;
    }
    for (k = 0; k < tcount; k++) {
        size_t u = topo[k];

        for (j = 0; j < g->n; j++) {
            if (g->w[u][j] != CP_INF && ve[u] + g->w[u][j] > ve[j]) {
                ve[j] = ve[u] + g->w[u][j];
            }
        }
    }

    /* 逆推 vl: vl[u] = min(vl[v] - w(u,v))，vl[汇] = ve[汇] */
    for (i = 0; i < g->n; i++) {
        vl[i] = ve[dst];
    }
    for (k = tcount; k > 0; k--) {
        size_t u = topo[k - 1];

        for (j = 0; j < g->n; j++) {
            if (g->w[u][j] != CP_INF && vl[j] - g->w[u][j] < vl[u]) {
                vl[u] = vl[j] - g->w[u][j];
            }
        }
    }

    /* 活动表: e = ve[u], l = vl[v] - w */
    *act_count = 0;
    *duration = ve[dst];
    for (i = 0; i < g->n; i++) {
        for (j = 0; j < g->n; j++) {
            if (g->w[i][j] == CP_INF) {
                continue;
            }
            acts[*act_count].u = i;
            acts[*act_count].v = j;
            acts[*act_count].w = g->w[i][j];
            acts[*act_count].e = ve[i];
            acts[*act_count].l = vl[j] - g->w[i][j];
            acts[*act_count].critical = (acts[*act_count].e == acts[*act_count].l);
            (*act_count)++;
        }
    }
    return (DsResult){DS_OK, "关键路径分析成功"};
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
