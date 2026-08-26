#ifndef MIN_SPANNING_TREE_H
#define MIN_SPANNING_TREE_H

/**
 * @file min_spanning_tree.h
 * @brief 最小生成树（Prim + Kruskal）接口定义
 *
 * 算法思想:
 *   Prim: 点贪心 —— 从任一点出发，每轮把"跨割边中权最小者"
 *         收入树中，树点集扩张一步，n-1 轮完成，适合稠密图 O(n^2)
 *   Kruskal: 边贪心 —— 全边按权升序，依次尝试入选，
 *         用并查集判两端点是否已连通（成环则弃），适合稀疏图 O(e·α + e·loge)
 * 共同前提: 连通无向网；不连通时应报错而非输出残树
 * 设计要点:
 *   模块自带邻接矩阵存储（自包含）；并查集为内置静态实现
 *   （路径压缩），与 06_树/07_并查集 模块算法同型
 */

#include <stddef.h>

/* 状态码（按本模块所需裁剪，语义见各枚举项注释） */
typedef enum {
    DS_OK = 0,       /* 操作成功 */
    DS_ERROR,        /* 一般性错误（图不连通，无生成树） */
    DS_NULL_PTR,     /* 空指针参数 */
    DS_OUT_OF_RANGE  /* 下标越界 */
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 顶点数上限 */
#define MST_MAX_VERTICES 60

/* 无边哨兵 */
#define MST_INF 0x3FFFFFFF

/* 边（输出） */
typedef struct {
    size_t u;
    size_t v;
    int w;
} MstEdge;

/* 无向网（邻接矩阵） */
typedef struct {
    int w[MST_MAX_VERTICES][MST_MAX_VERTICES];
    size_t n;
    size_t edge_count;
} MstGraph;

DsResult mstg_create(MstGraph *g, size_t n);
DsResult mstg_add_edge(MstGraph *g, size_t u, size_t v, int w);
DsResult mstg_has_edge(const MstGraph *g, size_t u, size_t v, int *has);
DsResult mst_prim(const MstGraph *g, MstEdge edges[], size_t *count, long *total);
DsResult mst_kruskal(const MstGraph *g, MstEdge edges[], size_t *count, long *total);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* MIN_SPANNING_TREE_H */
