#ifndef ADJACENCY_MULTILIST_H
#define ADJACENCY_MULTILIST_H

/**
 * @file adjacency_multilist.h
 * @brief 无向图的邻接多重表存储接口定义
 *
 * 存储结构: 每条边仅一个边结点，同时挂在两个端点的依附链上
 *           边结点: [ivex | jvex | ilink | jlink | weight]
 *           ilink 指向下一条依附于 ivex 的边，jlink 同理依附于 jvex
 * 设计要点:
 *   1. 与邻接表存无向图"每边两份弧结点"相比，多重表每边一份
 *      —— 删除边只需释放一个结点（这是它的核心卖点）
 *   2. 遍历顶点 v 的依附链时，每走一步要判断当前边的哪端
 *      是 v: ivex==v 则沿 ilink，否则沿 jlink（"看路牌走链"）
 *   3. 删除边须从两端依附链同时摘除同一结点，注意
 *      若边为自环（两端同顶点）两链退化为一条
 * 复杂度: 插边 O(1)；判边/删边 O(deg)；度数 O(deg)
 */

#include <stddef.h>

/* 状态码（按本模块所需裁剪，语义见各枚举项注释） */
typedef enum {
    DS_OK = 0,       /* 操作成功 */
    DS_ERROR,        /* 一般性错误（无此边/已存在） */
    DS_NULL_PTR,     /* 空指针参数 */
    DS_OUT_OF_RANGE  /* 顶点下标越界 */
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 顶点数上限 */
#define AML_MAX_VERTICES 100

/* 边结点: 一条边同时归属两端点的依附链 */
typedef struct AmlEdgeNode {
    size_t ivex;                 /* 边的一端 */
    size_t jvex;                 /* 边的另一端 */
    struct AmlEdgeNode *ilink;   /* 下一条依附于 ivex 的边 */
    struct AmlEdgeNode *jlink;   /* 下一条依附于 jvex 的边 */
    int weight;
} AmlEdgeNode;

/* 邻接多重表无向图 */
typedef struct {
    AmlEdgeNode *first_edge[AML_MAX_VERTICES]; /* 各顶点首条依附边 */
    size_t n;
    size_t edge_count;
} AmlGraph;

DsResult aml_create(AmlGraph *g, size_t n);
DsResult aml_destroy(AmlGraph *g);
size_t aml_vertex_count(const AmlGraph *g);
size_t aml_edge_count(const AmlGraph *g);
DsResult aml_set_edge(AmlGraph *g, size_t u, size_t v, int w);
DsResult aml_remove_edge(AmlGraph *g, size_t u, size_t v);
DsResult aml_has_edge(const AmlGraph *g, size_t u, size_t v, int *has);
DsResult aml_get_weight(const AmlGraph *g, size_t u, size_t v, int *w);
DsResult aml_neighbors(const AmlGraph *g, size_t u, size_t out[], size_t *count);
DsResult aml_degree(const AmlGraph *g, size_t u, size_t *deg);
DsResult aml_print(const AmlGraph *g);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* ADJACENCY_MULTILIST_H */
