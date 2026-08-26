/**
 * @file adjacency_list.c
 * @brief 图的邻接表存储接口实现
 *
 * 设计要点:
 *   1. 尾插保序: set_edge 扫到链尾再挂 —— 插入序即链序，
 *      测试按编号升序插边即可复现矩阵法的编号序
 *   2. remove_edge 无向情形两次摘链（u 链删 v、v 链删 u），
 *      每次都是标准单链表按值摘除
 *   3. dfs 非递归用"逆序压栈"：邻居按链序入临时数组再逆序压栈，
 *      使出栈顺序与链序一致
 * 复杂度: 加边 O(deg)；判边 O(deg)；DFS/BFS O(n+e)
 */

#include <stdio.h>
#include <stdlib.h>

#include "adjacency_list.h"


DsResult ag_create(AdjListGraph *g, size_t n, int directed)
{
    size_t i = 0;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (n == 0 || n > AG_MAX_VERTICES) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点数越界"};
    }

    for (i = 0; i < n; i++) {
        g->first[i] = NULL;
    }
    g->n = n;
    g->directed = directed;
    g->edge_count = 0;
    return (DsResult){DS_OK, "邻接表图创建成功"};
}

DsResult ag_destroy(AdjListGraph *g)
{
    size_t i = 0;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }

    for (i = 0; i < g->n; i++) {
        AdjArcNode *p = g->first[i];

        while (p != NULL) {
            AdjArcNode *next = p->next;

            free(p);
            p = next;
        }
        g->first[i] = NULL;
    }
    g->n = 0;
    g->edge_count = 0;
    return (DsResult){DS_OK, "邻接表图销毁成功"};
}

size_t ag_vertex_count(const AdjListGraph *g)
{
    return (g == NULL) ? 0 : g->n;
}

size_t ag_edge_count(const AdjListGraph *g)
{
    return (g == NULL) ? 0 : g->edge_count;
}

/* 尾插一条弧 u->v（已存在则改权并置 *existed=1），不改 edge_count */
static DsStatus append_arc(AdjListGraph *g, size_t u, size_t v, int w, int *existed)
{
    AdjArcNode *node = NULL;
    AdjArcNode **pp = &g->first[u];

    while (*pp != NULL) {
        if ((*pp)->adjvex == v) {
            (*pp)->weight = w; /* 重边合并语义: 改权 */
            *existed = 1;
            return DS_OK;
        }
        pp = &(*pp)->next;
    }
    node = (AdjArcNode *)malloc(sizeof(AdjArcNode));
    if (node == NULL) {
        return DS_ERROR;
    }
    node->adjvex = v;
    node->weight = w;
    node->next = NULL;
    *pp = node; /* 空指针技巧: pp 恰指向插入点 */
    *existed = 0;
    return DS_OK;
}

/* 摘除 u->v 首条弧，返回是否摘到 */
static int remove_arc(AdjListGraph *g, size_t u, size_t v)
{
    AdjArcNode **pp = &g->first[u];

    while (*pp != NULL) {
        if ((*pp)->adjvex == v) {
            AdjArcNode *victim = *pp;

            *pp = victim->next;
            free(victim);
            return 1;
        }
        pp = &(*pp)->next;
    }
    return 0;
}

DsResult ag_set_edge(AdjListGraph *g, size_t u, size_t v, int w)
{
    int existed = 0;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (u >= g->n || v >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    if (append_arc(g, u, v, w, &existed) != DS_OK) {
        return (DsResult){DS_ERROR, "弧结点分配失败"};
    }
    if (!g->directed && u != v) {
        int existed2 = 0;

        if (append_arc(g, v, u, w, &existed2) != DS_OK) {
            return (DsResult){DS_ERROR, "弧结点分配失败"};
        }
    }
    if (!existed) {
        g->edge_count++;
    }
    return (DsResult){DS_OK, "设置边成功"};
}

DsResult ag_remove_edge(AdjListGraph *g, size_t u, size_t v)
{
    int removed = 0;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }
    if (u >= g->n || v >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    removed = remove_arc(g, u, v);
    if (!removed) {
        return (DsResult){DS_ERROR, "无此边"}; /* 无此边 */
    }
    if (!g->directed && u != v) {
        remove_arc(g, v, u); /* 对称端必存在（不变量保证） */
    }
    g->edge_count--;
    return (DsResult){DS_OK, "删除边成功"};
}

DsResult ag_has_edge(const AdjListGraph *g, size_t u, size_t v, int *has)
{
    const AdjArcNode *p = NULL;

    if (g == NULL || has == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n || v >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    for (p = g->first[u]; p != NULL; p = p->next) {
        if (p->adjvex == v) {
            *has = 1;
            return (DsResult){DS_OK, "查询边成功"};
        }
    }
    *has = 0;
    return (DsResult){DS_OK, "查询边成功"};
}

DsResult ag_get_weight(const AdjListGraph *g, size_t u, size_t v, int *w)
{
    const AdjArcNode *p = NULL;

    if (g == NULL || w == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n || v >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    for (p = g->first[u]; p != NULL; p = p->next) {
        if (p->adjvex == v) {
            *w = p->weight;
            return (DsResult){DS_OK, "获取权值成功"};
        }
    }
    return (DsResult){DS_ERROR, "无此边"};
}

DsResult ag_neighbors(const AdjListGraph *g, size_t u, size_t out[], size_t *count)
{
    const AdjArcNode *p = NULL;

    if (g == NULL || out == NULL || count == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    *count = 0;
    for (p = g->first[u]; p != NULL; p = p->next) {
        out[(*count)++] = p->adjvex;
    }
    return (DsResult){DS_OK, "枚举邻居成功"};
}

DsResult ag_out_degree(const AdjListGraph *g, size_t u, size_t *deg)
{
    const AdjArcNode *p = NULL;

    if (g == NULL || deg == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    *deg = 0;
    for (p = g->first[u]; p != NULL; p = p->next) {
        (*deg)++;
    }
    return (DsResult){DS_OK, "计算出度成功"};
}

DsResult ag_in_degree(const AdjListGraph *g, size_t u, size_t *deg)
{
    size_t i = 0;

    if (g == NULL || deg == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (u >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "顶点下标越界"};
    }

    /* 出边邻接表求入度只能全表扫描（逆邻接表可免） */
    *deg = 0;
    for (i = 0; i < g->n; i++) {
        const AdjArcNode *p = NULL;

        for (p = g->first[i]; p != NULL; p = p->next) {
            if (p->adjvex == u) {
                (*deg)++;
            }
        }
    }
    return (DsResult){DS_OK, "计算入度成功"};
}

DsResult ag_dfs(const AdjListGraph *g, size_t start, size_t out[], size_t *count)
{
    static size_t stack[AG_MAX_VERTICES * AG_MAX_VERTICES]; /* 栈: 邻居逆序展开可能较大 */
    static int visited[AG_MAX_VERTICES];
    static size_t buf[AG_MAX_VERTICES]; /* 邻居缓冲 */
    size_t top = 0;

    if (g == NULL || out == NULL || count == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (start >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "起点下标越界"};
    }

    {
        size_t i = 0;

        for (i = 0; i < g->n; i++) {
            visited[i] = 0;
        }
    }

    *count = 0;
    stack[top++] = start;
    while (top > 0) {
        size_t u = stack[--top];

        if (visited[u]) {
            continue; /* 栈中可能残留已访问顶点（重边/多路径） */
        }
        visited[u] = 1;
        out[(*count)++] = u;

        /* 邻居收集后逆序压栈: 出栈序与链序一致 */
        {
            size_t nb = 0;
            const AdjArcNode *p = NULL;

            for (p = g->first[u]; p != NULL; p = p->next) {
                if (!visited[p->adjvex]) {
                    buf[nb++] = p->adjvex;
                }
            }
            while (nb > 0) {
                stack[top++] = buf[--nb];
            }
        }
    }
    return (DsResult){DS_OK, "DFS遍历完成"};
}

DsResult ag_bfs(const AdjListGraph *g, size_t start, size_t out[], size_t *count)
{
    static size_t queue[AG_MAX_VERTICES * AG_MAX_VERTICES];
    static int visited[AG_MAX_VERTICES];
    size_t head = 0;
    size_t tail = 0;

    if (g == NULL || out == NULL || count == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (start >= g->n) {
        return (DsResult){DS_OUT_OF_RANGE, "起点下标越界"};
    }

    {
        size_t i = 0;

        for (i = 0; i < g->n; i++) {
            visited[i] = 0;
        }
    }

    *count = 0;
    visited[start] = 1;
    queue[tail++] = start;
    while (head < tail) {
        size_t u = queue[head++];
        const AdjArcNode *p = NULL;

        out[(*count)++] = u;
        for (p = g->first[u]; p != NULL; p = p->next) {
            if (!visited[p->adjvex]) {
                visited[p->adjvex] = 1;
                queue[tail++] = p->adjvex;
            }
        }
    }
    return (DsResult){DS_OK, "BFS遍历完成"};
}

DsResult ag_print(const AdjListGraph *g)
{
    size_t i = 0;

    if (g == NULL) {
        return (DsResult){DS_NULL_PTR, "图指针为空"};
    }

    printf("邻接表(%s, %zu 顶点, %zu 边):\n",
           g->directed ? "有向" : "无向", g->n, g->edge_count);
    for (i = 0; i < g->n; i++) {
        const AdjArcNode *p = NULL;

        printf("  %zu ->", i);
        for (p = g->first[i]; p != NULL; p = p->next) {
            printf(" (%zu,w=%d)", p->adjvex, p->weight);
        }
        printf("\n");
    }
    return (DsResult){DS_OK, "打印完成"};
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "顶点下标越界";
        default:             return "未知状态码";
    }
}
