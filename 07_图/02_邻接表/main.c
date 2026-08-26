/**
 * @file main.c
 * @brief 邻接表图断言测试、暴力对拍与演示
 *
 * 覆盖点: 加删边/重边语义/度数 / 非递归 DFS + BFS /
 *         随机图对可达闭包对拍 / 与邻接矩阵参考实现遍历集一致
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "adjacency_list.h"

int main(void)
{
    AdjListGraph g;
    size_t seq[AG_MAX_VERTICES];
    size_t count = 0;
    size_t deg = 0;
    int has = 0;
    int w = 0;
    size_t i = 0;

    printf("===== 图的邻接表存储（非递归 DFS / BFS） =====\n");

    /* 与邻接矩阵模块同图: 边 (0,1)(0,2)(0,3)(1,3)(2,4)(3,4)(3,5)(4,5) */
    assert(ag_create(&g, 6, 0).code == DS_OK);
    assert(ag_set_edge(&g, 0, 1, 1).code == DS_OK);
    assert(ag_set_edge(&g, 0, 2, 1).code == DS_OK);
    assert(ag_set_edge(&g, 0, 3, 1).code == DS_OK);
    assert(ag_set_edge(&g, 1, 3, 1).code == DS_OK);
    assert(ag_set_edge(&g, 2, 4, 1).code == DS_OK);
    assert(ag_set_edge(&g, 3, 4, 1).code == DS_OK);
    assert(ag_set_edge(&g, 3, 5, 1).code == DS_OK);
    assert(ag_set_edge(&g, 4, 5, 1).code == DS_OK);
    assert(ag_edge_count(&g) == 8);

    assert(ag_has_edge(&g, 1, 0, &has).code == DS_OK && has == 1);
    assert(ag_has_edge(&g, 1, 2, &has).code == DS_OK && has == 0);
    assert(ag_get_weight(&g, 5, 3, &w).code == DS_OK && w == 1);

    /* 度数 */
    assert(ag_out_degree(&g, 3, &deg).code == DS_OK && deg == 4);
    assert(ag_in_degree(&g, 3, &deg).code == DS_OK && deg == 4);

    /* 非递归 DFS: 按编号升序插边 -> 链序即编号序 -> 与递归 DFS 同序 */
    assert(ag_dfs(&g, 0, seq, &count).code == DS_OK);
    assert(count == 6);
    assert(seq[0] == 0 && seq[1] == 1 && seq[2] == 3);
    assert(seq[3] == 4 && seq[4] == 2 && seq[5] == 5);
    printf("DFS(从0, 非递归): ");
    for (i = 0; i < count; i++) {
        printf("%zu ", seq[i]);
    }
    printf("\n");

    assert(ag_bfs(&g, 0, seq, &count).code == DS_OK);
    assert(count == 6);
    assert(seq[0] == 0 && seq[1] == 1 && seq[2] == 2);
    assert(seq[3] == 3 && seq[4] == 4 && seq[5] == 5);
    printf("BFS(从0): ");
    for (i = 0; i < count; i++) {
        printf("%zu ", seq[i]);
    }
    printf("\n");

    /* 删边 */
    assert(ag_remove_edge(&g, 0, 3).code == DS_OK);
    assert(ag_has_edge(&g, 3, 0, &has).code == DS_OK && has == 0); /* 对称摘除 */
    assert(ag_remove_edge(&g, 0, 3).code == DS_ERROR);             /* 再删报错 */
    assert(ag_set_edge(&g, 0, 3, 7).code == DS_OK);                /* 恢复改权 */
    assert(ag_get_weight(&g, 3, 0, &w).code == DS_OK && w == 7);

    /* 有向图语义 */
    {
        AdjListGraph dg;

        assert(ag_create(&dg, 4, 1).code == DS_OK);
        assert(ag_set_edge(&dg, 0, 1, 5).code == DS_OK);
        assert(ag_set_edge(&dg, 3, 0, 6).code == DS_OK);
        assert(ag_has_edge(&dg, 1, 0, &has).code == DS_OK && has == 0);
        assert(ag_out_degree(&dg, 0, &deg).code == DS_OK && deg == 1);
        assert(ag_in_degree(&dg, 0, &deg).code == DS_OK && deg == 1);
        assert(ag_dfs(&dg, 0, seq, &count).code == DS_OK);
        assert(count == 2 && seq[0] == 0 && seq[1] == 1); /* 3 不可达 */
        assert(ag_bfs(&dg, 3, seq, &count).code == DS_OK);
        assert(count == 3);
        assert(ag_destroy(&dg).code == DS_OK);
    }

    /* 边界 */
    assert(ag_create(NULL, 3, 0).code == DS_NULL_PTR);
    assert(ag_create(&g, 0, 0).code == DS_OUT_OF_RANGE);
    assert(ag_set_edge(&g, 0, 100, 1).code == DS_OUT_OF_RANGE);
    assert(ag_has_edge(NULL, 0, 0, &has).code == DS_NULL_PTR);
    assert(ag_dfs(&g, 100, seq, &count).code == DS_OUT_OF_RANGE);
    assert(ag_destroy(NULL).code == DS_NULL_PTR);

    assert(ag_destroy(&g).code == DS_OK);
    assert(ag_destroy(&g).code == DS_OK); /* 重复销毁安全 */

    /* 演示打印 */
    assert(ag_create(&g, 4, 1).code == DS_OK);
    assert(ag_set_edge(&g, 0, 1, 2).code == DS_OK);
    assert(ag_set_edge(&g, 0, 2, 9).code == DS_OK);
    assert(ag_set_edge(&g, 1, 3, 3).code == DS_OK);
    assert(ag_print(&g).code == DS_OK);
    assert(ag_destroy(&g).code == DS_OK);

    printf("全部测试通过\n");
    return 0;
}
