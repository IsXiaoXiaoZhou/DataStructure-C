#ifndef ORTHOGONAL_LIST_H
#define ORTHOGONAL_LIST_H

/**
 * @file orthogonal_list.h
 * @brief 有向图的十字链表存储接口定义
 *
 * 存储结构: 顶点表配两条链 —— first_out 出弧链（同弧尾，沿 tlink）、
 *           first_in 入弧链（同弧头，沿 hlink）；
 *           每条弧只有一个弧结点，同时挂在两条链上
 *               tailvex   headvex
 *               tlink(同尾) hlink(同头)
 * 设计要点:
 *   1. 十字链表 = 邻接表 + 逆邻接表的合体:
 *      出度沿 first_out 数 O(出度)，入度沿 first_in 数 O(入度)，
 *      解决邻接表求入度须全表扫描的痛点
 *   2. 弧结点双链共享: 删除时须从出、入两条链同时摘除
 *   3. 仅用于有向图（无向图用邻接多重表，见 04 模块）
 * 复杂度: 插弧 O(1)（头插）；判弧/删弧 O(deg)；度数 O(deg)
 */

#include <stddef.h>

/* 状态码（按本模块所需裁剪，语义见各枚举项注释） */
typedef enum {
    DS_OK = 0,       /* 操作成功 */
    DS_ERROR,        /* 一般性错误（无此弧/已存在） */
    DS_NULL_PTR,     /* 空指针参数 */
    DS_OUT_OF_RANGE  /* 顶点下标越界 */
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 顶点数上限 */
#define OL_MAX_VERTICES 100

/* 弧结点: 一条弧同时归属弧尾出链与弧头入链 */
typedef struct OrthArcNode {
    size_t tailvex;              /* 弧尾顶点 */
    size_t headvex;              /* 弧头顶点 */
    struct OrthArcNode *hlink;   /* 下一同弧头弧（入弧链） */
    struct OrthArcNode *tlink;   /* 下一同弧尾弧（出弧链） */
    int weight;
} OrthArcNode;

/* 十字链表有向图 */
typedef struct {
    OrthArcNode *first_in[OL_MAX_VERTICES];  /* 入弧链头（按头插序，新弧在前） */
    OrthArcNode *first_out[OL_MAX_VERTICES]; /* 出弧链头 */
    size_t n;
    size_t arc_count;
} OrthGraph;

DsResult og_create(OrthGraph *g, size_t n);
DsResult og_destroy(OrthGraph *g);
size_t og_vertex_count(const OrthGraph *g);
size_t og_arc_count(const OrthGraph *g);
DsResult og_add_arc(OrthGraph *g, size_t tail, size_t head, int w);
DsResult og_remove_arc(OrthGraph *g, size_t tail, size_t head);
DsResult og_has_arc(const OrthGraph *g, size_t tail, size_t head, int *has);
DsResult og_get_weight(const OrthGraph *g, size_t tail, size_t head, int *w);
DsResult og_out_arcs(const OrthGraph *g, size_t u, size_t heads[], size_t *count);
DsResult og_in_arcs(const OrthGraph *g, size_t u, size_t tails[], size_t *count);
DsResult og_out_degree(const OrthGraph *g, size_t u, size_t *deg);
DsResult og_in_degree(const OrthGraph *g, size_t u, size_t *deg);
DsResult og_print(const OrthGraph *g);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* ORTHOGONAL_LIST_H */
