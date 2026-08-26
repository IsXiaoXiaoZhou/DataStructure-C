#ifndef ADJACENCY_LIST_H
#define ADJACENCY_LIST_H

/**
 * @file adjacency_list.h
 * @brief 图的邻接表存储（含 DFS / BFS）接口定义
 *
 * 存储结构: 顶点表（数组）+ 每顶点一条出边弧链表
 *           [v0|first]->(adj=2,w=5)->(adj=4,w=3)->NULL
 * 设计要点:
 *   1. 空间 O(n+e) —— 稀疏图相对邻接矩阵 O(n^2) 的核心优势；
 *      代价是判任意 (u,v) 边须扫 u 的链表 O(deg(u))
 *   2. 弧结点采用"尾插"进链，保持插入先后序稳定，
 *      与邻接矩阵"编号序"在同一测试图上给出一致遍历序（对拍依据）
 *   3. 无向图每条边在两端各挂一个弧结点（边数按 1 条计）；
 *      有向图邻接表只存出边，入度需逆邻接表或全表扫描
 * 复杂度: 加边 O(deg(u))；判边 O(deg(u))；DFS/BFS O(n+e)
 */

#include <stddef.h>

/* 状态码（按本模块所需裁剪，语义见各枚举项注释） */
typedef enum {
    DS_OK = 0,       /* 操作成功 */
    DS_ERROR,        /* 一般性错误 */
    DS_NULL_PTR,     /* 空指针参数 */
    DS_OUT_OF_RANGE  /* 顶点下标越界 */
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 顶点数上限 */
#define AG_MAX_VERTICES 100

/* 边/弧结点 */
typedef struct AdjArcNode {
    size_t adjvex;           /* 弧指向的顶点 */
    int weight;              /* 权值 */
    struct AdjArcNode *next; /* 下一弧 */
} AdjArcNode;

/* 邻接表图 */
typedef struct {
    AdjArcNode *first[AG_MAX_VERTICES]; /* 各顶点弧链头 */
    size_t n;
    int directed;
    size_t edge_count;
} AdjListGraph;

DsResult ag_create(AdjListGraph *g, size_t n, int directed);
DsResult ag_destroy(AdjListGraph *g);
size_t ag_vertex_count(const AdjListGraph *g);
size_t ag_edge_count(const AdjListGraph *g);
DsResult ag_set_edge(AdjListGraph *g, size_t u, size_t v, int w);
DsResult ag_remove_edge(AdjListGraph *g, size_t u, size_t v);
DsResult ag_has_edge(const AdjListGraph *g, size_t u, size_t v, int *has);
DsResult ag_get_weight(const AdjListGraph *g, size_t u, size_t v, int *w);
DsResult ag_neighbors(const AdjListGraph *g, size_t u, size_t out[], size_t *count);
DsResult ag_out_degree(const AdjListGraph *g, size_t u, size_t *deg);
DsResult ag_in_degree(const AdjListGraph *g, size_t u, size_t *deg);
DsResult ag_dfs(const AdjListGraph *g, size_t start, size_t out[], size_t *count);
DsResult ag_bfs(const AdjListGraph *g, size_t start, size_t out[], size_t *count);
DsResult ag_print(const AdjListGraph *g);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* ADJACENCY_LIST_H */
