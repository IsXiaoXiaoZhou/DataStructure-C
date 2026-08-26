#ifndef SHORTEST_PATH_H
#define SHORTEST_PATH_H

/**
 * @file shortest_path.h
 * @brief 最短路径（Dijkstra 单源 + Floyd 全源）接口定义
 *
 * 算法思想:
 *   Dijkstra: 贪心 —— 维护确定集 S，每轮取集外 dist 最小点入集，
 *             并对其出边松弛。要求边权非负（负权会破坏
 *             "已确定点不再改进"的贪心前提）
 *   Floyd: 动态规划 —— dist[k][i][j] = 允许经过中转点 0..k 的
 *          最短路边权矩阵滚动降维: 以 k 为中转尝试 dist[i][k]+dist[k][j]
 *          三重循环，可处理负权边（但不能有负权回路）
 * 设计要点:
 *   模块自带邻接矩阵（有向带权网）；Dijkstra 附带 prev 前驱数组
 *   支持路径重建；Floyd 附带 next 后继矩阵支持路径重建
 */

#include <stddef.h>

/* 状态码（按本模块所需裁剪，语义见各枚举项注释） */
typedef enum {
    DS_OK = 0,       /* 操作成功 */
    DS_ERROR,        /* 一般性错误（负权/非法参数） */
    DS_NULL_PTR,     /* 空指针参数 */
    DS_OUT_OF_RANGE  /* 下标越界 */
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 顶点数上限 */
#define SP_MAX_VERTICES 60

/* 无边/不可达哨兵 */
#define SP_INF 0x3FFFFFFF

/* 有向带权网 */
typedef struct {
    int w[SP_MAX_VERTICES][SP_MAX_VERTICES]; /* SP_INF 表无弧 */
    size_t n;
} SpGraph;

DsResult spg_create(SpGraph *g, size_t n);
DsResult spg_add_arc(SpGraph *g, size_t u, size_t v, int w);
DsResult sp_dijkstra(const SpGraph *g, size_t src, int dist[], size_t prev[]);
DsResult sp_floyd(const SpGraph *g, int dist[][SP_MAX_VERTICES],
                  size_t next[][SP_MAX_VERTICES]);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* SHORTEST_PATH_H */
