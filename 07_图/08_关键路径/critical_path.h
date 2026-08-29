#ifndef CRITICAL_PATH_H
#define CRITICAL_PATH_H

/**
 * @file critical_path.h
 * @brief AOE 网关键路径分析接口定义
 *
 * 概念: AOE 网（Activity On Edge）—— 弧为活动（耗时 = 权），
 *       顶点为事件；单源点（入度 0，工程开始）单汇点（出度 0，工程完成）
 * 四个时间量:
 *   ve[v] 事件最早发生: 源到 v 的最长路径长（拓扑序正推取 max）
 *   vl[v] 事件最迟发生: 不拖累汇点 ve 的最晚时刻（逆拓扑序推取 min）
 *   e(u,v)  活动最早开始 = ve[u]
 *   l(u,v)  活动最迟开始 = vl[v] - w(u,v)
 * 关键活动: e == l（时间余量 slack 为 0）；
 * 关键路径: 全部由关键活动构成的源->汇路径，长度 = ve[汇] = 工程最短工期
 * 前提: DAG（拓扑排序是正逆推的基础）；若有环或非单源汇结构报错
 * 复杂度: 正逆推 + 活动统计均 O(n+e)
 */

#include <stddef.h>

/* 状态码（按本模块所需裁剪，语义见各枚举项注释） */
typedef enum {
    DS_OK = 0,       /* 操作成功 */
    DS_ERROR,        /* 一般性错误（有环/多源多汇） */
    DS_NULL_PTR,     /* 空指针参数 */
    DS_OUT_OF_RANGE  /* 下标越界 */
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 顶点数上限 */
#define CP_MAX_VERTICES 100

/* 无弧哨兵 */
#define CP_INF 0x3FFFFFFF

/* 活动分析结果 */
typedef struct {
    size_t u;
    size_t v;
    int w;        /* 活动耗时 */
    long e;       /* 最早开始 */
    long l;       /* 最迟开始 */
    int critical; /* 1 关键活动（e==l） */
} CpActivity;

/* AOE 网（有向带权，边 = 活动） */
typedef struct {
    int w[CP_MAX_VERTICES][CP_MAX_VERTICES]; /* CP_INF 表无弧 */
    size_t n;
    size_t arc_count;
} AoeGraph;

DsResult aoe_create(AoeGraph *g, size_t n);
DsResult aoe_add_arc(AoeGraph *g, size_t u, size_t v, int w);
DsResult cp_analyze(const AoeGraph *g, long ve[], long vl[],
                    CpActivity acts[], size_t *act_count, long *duration);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* CRITICAL_PATH_H */
