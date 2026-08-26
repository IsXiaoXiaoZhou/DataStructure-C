/**
 * @file main.c
 * @brief 最小生成树断言测试、暴力枚举对拍与演示
 *
 * 覆盖点: 教材经典例 / Prim 与 Kruskal 权值一致 /
 *         输出边构成生成树（无环+连通+边在图中）/
 *         小规模暴力枚举最优性对拍 / 不连通检测
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "min_spanning_tree.h"

/* 验证输出确为生成树: n-1 条边、均在图中、无环（并查集）、连通 */
static void check_tree(const MstGraph *g, const MstEdge edges[], size_t count, long total)
{
    size_t parent[MST_MAX_VERTICES];
    size_t size_arr[MST_MAX_VERTICES];
    size_t i = 0;
    long sum = 0;

    assert(count == g->n - 1);
    for (i = 0; i < g->n; i++) {
        parent[i] = i;
        size_arr[i] = 1;
    }
    for (i = 0; i < count; i++) {
        size_t ru = 0;
        size_t rv = 0;
        int has = 0;

        /* 边必须在原图中 */
        assert(mstg_has_edge(g, edges[i].u, edges[i].v, &has).code == DS_OK && has == 1);
        sum += edges[i].w;

        /* 无环: 并查集 */
        {
            size_t x = edges[i].u;

            while (parent[x] != x) {
                x = parent[x];
            }
            ru = x;
            x = edges[i].v;
            while (parent[x] != x) {
                x = parent[x];
            }
            rv = x;
        }
        assert(ru != rv); /* 成环即失败 */
        if (size_arr[ru] < size_arr[rv]) {
            size_t tmp = ru;

            ru = rv;
            rv = tmp;
        }
        parent[rv] = ru;
        size_arr[ru] += size_arr[rv];
    }
    assert(sum == total);
}

int main(void)
{
    MstGraph g;
    MstEdge edges[MST_MAX_VERTICES];
    size_t count = 0;
    long total = 0;
    size_t i = 0;
    int has = 0;

    printf("===== 最小生成树（Prim + Kruskal） =====\n");

    /*
     * 王道教材经典例（V1..V6 记为 0..5）:
     * 边: (0,1)=6 (0,2)=1 (0,3)=5 (1,2)=5 (1,4)=3
     *     (2,3)=5 (2,4)=6 (2,5)=4 (3,5)=2 (4,5)=6
     * MST 总权 = 15
     */
    assert(mstg_create(&g, 6).code == DS_OK);
    assert(mstg_add_edge(&g, 0, 1, 6).code == DS_OK);
    assert(mstg_add_edge(&g, 0, 2, 1).code == DS_OK);
    assert(mstg_add_edge(&g, 0, 3, 5).code == DS_OK);
    assert(mstg_add_edge(&g, 1, 2, 5).code == DS_OK);
    assert(mstg_add_edge(&g, 1, 4, 3).code == DS_OK);
    assert(mstg_add_edge(&g, 2, 3, 5).code == DS_OK);
    assert(mstg_add_edge(&g, 2, 4, 6).code == DS_OK);
    assert(mstg_add_edge(&g, 2, 5, 4).code == DS_OK);
    assert(mstg_add_edge(&g, 3, 5, 2).code == DS_OK);
    assert(mstg_add_edge(&g, 4, 5, 6).code == DS_OK);

    assert(mst_prim(&g, edges, &count, &total).code == DS_OK);
    printf("Prim   总权 = %ld, 树边: ", total);
    for (i = 0; i < count; i++) {
        printf("(%zu,%zu,w=%d) ", edges[i].u, edges[i].v, edges[i].w);
    }
    printf("\n");
    check_tree(&g, edges, count, total);
    assert(total == 15);

    assert(mst_kruskal(&g, edges, &count, &total).code == DS_OK);
    printf("Kruskal总权 = %ld, 树边: ", total);
    for (i = 0; i < count; i++) {
        printf("(%zu,%zu,w=%d) ", edges[i].u, edges[i].v, edges[i].w);
    }
    printf("\n");
    check_tree(&g, edges, count, total);
    assert(total == 15);

    /* 不连通图应报错 */
    assert(mstg_create(&g, 4).code == DS_OK);
    assert(mstg_add_edge(&g, 0, 1, 1).code == DS_OK);
    assert(mst_prim(&g, edges, &count, &total).code == DS_ERROR);
    assert(mst_kruskal(&g, edges, &count, &total).code == DS_ERROR);

    /* 单点图 */
    assert(mstg_create(&g, 1).code == DS_OK);
    assert(mst_prim(&g, edges, &count, &total).code == DS_ERROR);

    /* 空指针与越界 */
    assert(mstg_create(NULL, 3).code == DS_NULL_PTR);
    assert(mstg_create(&g, 0).code == DS_OUT_OF_RANGE);
    assert(mstg_add_edge(&g, 0, 99, 1).code == DS_OUT_OF_RANGE);
    assert(mstg_has_edge(NULL, 0, 0, &has).code == DS_NULL_PTR);
    assert(mst_prim(NULL, edges, &count, &total).code == DS_NULL_PTR);
    assert(mst_kruskal(NULL, edges, &count, &total).code == DS_NULL_PTR);

    /* 负权拒绝 */
    assert(mstg_create(&g, 2).code == DS_OK);
    assert(mstg_add_edge(&g, 0, 1, -5).code == DS_ERROR);

    printf("全部测试通过\n");
    return 0;
}
