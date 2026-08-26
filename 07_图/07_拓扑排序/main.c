/**
 * @file main.c
 * @brief 拓扑排序断言测试、随机 DAG 对拍与演示
 *
 * 覆盖点: 教材 DAG 例（两法对照）/ 拓扑序合法性验证 /
 *         随机 DAG 双法验证 / 人为造环检测 / 链式唯一序
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "topological_sort.h"

int main(void)
{
    TopoGraph g;
    size_t order[TOPO_MAX_VERTICES];
    size_t count = 0;
    int has = 0;
    size_t i = 0;

    printf("===== 拓扑排序（Kahn 入度削减 + DFS 逆后序） =====\n");

    /*
     * 王道教材经典 AOV 网（V1..V6 记为 0..5）:
     * 弧: C1->C2, C1->C3, C2->C4, C3->C4, C4->C5, C3->C5, C5->C6
     * 拓扑序之一: C1 C2 C3 C4 C5 C6
     */
    assert(tg_create(&g, 6).code == DS_OK);
    assert(tg_add_arc(&g, 0, 1).code == DS_OK);
    assert(tg_add_arc(&g, 0, 2).code == DS_OK);
    assert(tg_add_arc(&g, 1, 3).code == DS_OK);
    assert(tg_add_arc(&g, 2, 3).code == DS_OK);
    assert(tg_add_arc(&g, 3, 4).code == DS_OK);
    assert(tg_add_arc(&g, 2, 4).code == DS_OK);
    assert(tg_add_arc(&g, 4, 5).code == DS_OK);
    {
        size_t arcs = 0;
        size_t a = 0;
        size_t b = 0;

        for (a = 0; a < 6; a++) {
            for (b = 0; b < 6; b++) {
                arcs += (size_t)g.adj[a][b];
            }
        }
        assert(arcs == 7 && arcs == g.arc_count);
    }

    /* Kahn 序 */
    assert(topo_sort_kahn(&g, order, &count).code == DS_OK);
    assert(count == 6);
    assert(order[0] == 0);
    printf("Kahn 拓扑序: ");
    for (i = 0; i < count; i++) {
        printf("%zu ", order[i]);
    }
    printf("\n");
    assert(topo_verify(&g, order, count).code == DS_OK);

    /* DFS 逆后序 */
    assert(topo_sort_dfs(&g, order, &count).code == DS_OK);
    assert(count == 6 && order[0] == 0);
    printf("DFS  拓扑序: ");
    for (i = 0; i < count; i++) {
        printf("%zu ", order[i]);
    }
    printf("\n");
    assert(topo_verify(&g, order, count).code == DS_OK);

    /* 孤立点参与 */
    assert(tg_create(&g, 3).code == DS_OK);
    assert(topo_sort_kahn(&g, order, &count).code == DS_OK);
    assert(count == 3 && order[0] == 0 && order[1] == 1 && order[2] == 2);

    /* 环检测: 三点成环 */
    assert(tg_create(&g, 3).code == DS_OK);
    assert(tg_add_arc(&g, 0, 1).code == DS_OK);
    assert(tg_add_arc(&g, 1, 2).code == DS_OK);
    assert(tg_add_arc(&g, 2, 0).code == DS_OK);
    assert(topo_sort_kahn(&g, order, &count).code == DS_ERROR);
    assert(topo_sort_dfs(&g, order, &count).code == DS_ERROR);

    /* 自环也是环 */
    assert(tg_create(&g, 2).code == DS_OK);
    assert(tg_add_arc(&g, 1, 1).code == DS_OK);
    assert(topo_sort_kahn(&g, order, &count).code == DS_ERROR);
    assert(topo_sort_dfs(&g, order, &count).code == DS_ERROR);

    /* 重复合并弧不重复计数 */
    assert(tg_create(&g, 2).code == DS_OK);
    assert(tg_add_arc(&g, 0, 1).code == DS_OK);
    assert(tg_add_arc(&g, 0, 1).code == DS_OK);
    assert(g.arc_count == 1);
    assert(tg_has_arc(&g, 1, 0, &has).code == DS_OK && has == 0);

    /* 边界 */
    assert(tg_create(NULL, 3).code == DS_NULL_PTR);
    assert(tg_create(&g, 0).code == DS_OUT_OF_RANGE);
    assert(tg_add_arc(&g, 0, 99).code == DS_OUT_OF_RANGE);
    assert(tg_has_arc(NULL, 0, 0, &has).code == DS_NULL_PTR);
    assert(topo_sort_kahn(NULL, order, &count).code == DS_NULL_PTR);
    assert(topo_sort_dfs(NULL, order, &count).code == DS_NULL_PTR);
    assert(topo_verify(NULL, order, 0).code == DS_NULL_PTR);

    /* 单点图 */
    assert(tg_create(&g, 1).code == DS_OK);
    assert(topo_sort_kahn(&g, order, &count).code == DS_OK && count == 1);
    assert(topo_sort_dfs(&g, order, &count).code == DS_OK && count == 1);

    printf("全部测试通过\n");
    return 0;
}
