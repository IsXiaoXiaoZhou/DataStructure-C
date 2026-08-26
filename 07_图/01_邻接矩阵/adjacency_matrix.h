#ifndef ADJACENCY_MATRIX_H
#define ADJACENCY_MATRIX_H

/**
 * @file adjacency_matrix.h
 * @brief 图的邻接矩阵存储（含 DFS / BFS）接口定义
 *
 * 存储结构: n*n 二维数组，matrix[u][v] 存权值，
 *           无边用 INF 哨兵；无向图对称赋值
 * 设计要点:
 *   1. 统一"网"模型: 无权图视为权 1 的网，删除边 = 置 INF，
 *      判边 = 与 INF 比较
 *   2. 邻接矩阵判定任意 (u,v) 边 O(1) —— 核心优势；
 *      代价是空间 O(n^2) 且枚举邻居须扫整行
 *   3. 无向图加边对称双写、删边对称双删，保持矩阵对称不变量
 * 复杂度: 判边 O(1)；邻域/度数 O(n)；DFS/BFS O(n^2)
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
#define MG_MAX_VERTICES 100

/* 无边哨兵（足够大避免加法溢出，小于 int 上限一半） */
#define MG_INF 0x3FFFFFFF

/* 邻接矩阵图 */
typedef struct {
    int    weight[MG_MAX_VERTICES][MG_MAX_VERTICES]; /* INF 表无边 */
    size_t n;            /* 顶点数，下标 0..n-1 */
    int    directed;     /* 1 有向 / 0 无向 */
    size_t edge_count;   /* 边（无向按 1 条计）/ 弧数 */
} MatrixGraph;

DsResult mg_create(MatrixGraph *g, size_t n, int directed);
size_t mg_vertex_count(const MatrixGraph *g);
size_t mg_edge_count(const MatrixGraph *g);
DsResult mg_set_edge(MatrixGraph *g, size_t u, size_t v, int w);
DsResult mg_remove_edge(MatrixGraph *g, size_t u, size_t v);
DsResult mg_has_edge(const MatrixGraph *g, size_t u, size_t v, int *has);
DsResult mg_get_weight(const MatrixGraph *g, size_t u, size_t v, int *w);
DsResult mg_neighbors(const MatrixGraph *g, size_t u, size_t out[], size_t *count);
DsResult mg_out_degree(const MatrixGraph *g, size_t u, size_t *deg);
DsResult mg_in_degree(const MatrixGraph *g, size_t u, size_t *deg);
DsResult mg_dfs(const MatrixGraph *g, size_t start, size_t out[], size_t *count);
DsResult mg_bfs(const MatrixGraph *g, size_t start, size_t out[], size_t *count);
DsResult mg_print(const MatrixGraph *g);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* ADJACENCY_MATRIX_H */
