#ifndef TOPOLOGICAL_SORT_H
#define TOPOLOGICAL_SORT_H

/**
 * @file topological_sort.h
 * @brief 有向无环图（DAG / AOV 网）拓扑排序接口定义
 *
 * 算法思想（两种经典实现）:
 *   1. 入度削减法（Kahn）: 入度 0 者无前驱，可输出；
 *      输出后其出边邻居入度减 1，产生新 0 入度点；
 *      若输出数 < n，剩余点形成环（有向环上每点入度 >= 1）
 *   2. DFS 逆后序法: 完成时刻的逆序恰为拓扑序
 *      （u 在 v 之前完成 <=> 无 v->u 路径），环检测用
 *      三色标记（遇"进行中"灰点即有环）
 * 应用: 工程任务排序（AOV 网）、检测有向图是否有环、
 *       关键路径的前置（见 08 模块）
 */

#include <stddef.h>

/* 状态码（按本模块所需裁剪，语义见各枚举项注释） */
typedef enum {
    DS_OK = 0,       /* 操作成功 */
    DS_ERROR,        /* 一般性错误（存在有向环） */
    DS_NULL_PTR,     /* 空指针参数 */
    DS_OUT_OF_RANGE  /* 下标越界 */
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 顶点数上限 */
#define TOPO_MAX_VERTICES 100

/* AOV 网（有向图，邻接矩阵 0/1） */
typedef struct {
    int adj[TOPO_MAX_VERTICES][TOPO_MAX_VERTICES]; /* 1 有弧 */
    size_t n;
    size_t arc_count;
} TopoGraph;

DsResult tg_create(TopoGraph *g, size_t n);
DsResult tg_add_arc(TopoGraph *g, size_t u, size_t v);
DsResult tg_has_arc(const TopoGraph *g, size_t u, size_t v, int *has);
DsResult topo_sort_kahn(const TopoGraph *g, size_t order[], size_t *count);
DsResult topo_sort_dfs(const TopoGraph *g, size_t order[], size_t *count);
DsResult topo_verify(const TopoGraph *g, const size_t order[], size_t count);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* TOPOLOGICAL_SORT_H */
