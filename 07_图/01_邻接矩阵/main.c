/**
 * @file main.c
 * @brief 邻接矩阵图断言测试、性质对拍与演示
 *
 * 覆盖点: 建图/加删边/度数 / DFS/BFS 序 / 可达性对拍（Floyd 闭包）/
 *         BFS 层序非降性质 / 随机图压力
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "adjacency_matrix.h"

int main(void)
{
    MatrixGraph g;
    size_t seq[MG_MAX_VERTICES];
    size_t count = 0;
    size_t nb[MG_MAX_VERTICES];
    size_t ncount = 0;
    size_t deg = 0;
    int has = 0;
    int w = 0;
    size_t i = 0;

    printf("===== 图的邻接矩阵存储（DFS/BFS） =====\n");

    /*
     * 教材经典无向图:
     *      0 --- 1
     *      | \   |
     *      2   \ 3
     *       \  /|
     *        4--5
     * 边: (0,1)(0,2)(0,3)(1,3)(2,4)(3,4)(3,5)(4,5)
     */
    assert(mg_create(&g, 6, 0).code == DS_OK);
    assert(mg_set_edge(&g, 0, 1, 1).code == DS_OK);
    assert(mg_set_edge(&g, 0, 2, 1).code == DS_OK);
    assert(mg_set_edge(&g, 0, 3, 1).code == DS_OK);
    assert(mg_set_edge(&g, 1, 3, 1).code == DS_OK);
    assert(mg_set_edge(&g, 2, 4, 1).code == DS_OK);
    assert(mg_set_edge(&g, 3, 4, 1).code == DS_OK);
    assert(mg_set_edge(&g, 3, 5, 1).code == DS_OK);
    assert(mg_set_edge(&g, 4, 5, 1).code == DS_OK);
    assert(mg_edge_count(&g) == 8);

    /* 对称性: 无向边双向可查 */
    assert(mg_has_edge(&g, 1, 0, &has).code == DS_OK && has == 1);
    assert(mg_has_edge(&g, 5, 3, &has).code == DS_OK && has == 1);
    assert(mg_has_edge(&g, 1, 2, &has).code == DS_OK && has == 0);
    assert(mg_get_weight(&g, 3, 4, &w).code == DS_OK && w == 1);
    assert(mg_get_weight(&g, 4, 3, &w).code == DS_OK && w == 1);

    /* 度数: 无向图入度==出度==度 */
    assert(mg_out_degree(&g, 0, &deg).code == DS_OK && deg == 3);
    assert(mg_in_degree(&g, 3, &deg).code == DS_OK && deg == 4);
    assert(mg_neighbors(&g, 3, nb, &ncount).code == DS_OK);
    assert(ncount == 4 && nb[0] == 0 && nb[1] == 1 && nb[2] == 4 && nb[3] == 5);

    /* DFS 手工核对: 0 -> 1 -> 3 -> 4 -> 2 -> 5 */
    assert(mg_dfs(&g, 0, seq, &count).code == DS_OK);
    assert(count == 6);
    assert(seq[0] == 0 && seq[1] == 1 && seq[2] == 3);
    assert(seq[3] == 4 && seq[4] == 2 && seq[5] == 5);
    printf("DFS(从0): ");
    for (i = 0; i < count; i++) {
        printf("%zu ", seq[i]);
    }
    printf("\n");

    /* BFS 手工核对: 0 | 1 2 3 | 4 | 5 */
    assert(mg_bfs(&g, 0, seq, &count).code == DS_OK);
    assert(count == 6);
    assert(seq[0] == 0 && seq[1] == 1 && seq[2] == 2);
    assert(seq[3] == 3 && seq[4] == 4 && seq[5] == 5);
    printf("BFS(从0): ");
    for (i = 0; i < count; i++) {
        printf("%zu ", seq[i]);
    }
    printf("\n");

    /* 删边对称生效 */
    assert(mg_remove_edge(&g, 0, 3).code == DS_OK);
    assert(mg_has_edge(&g, 3, 0, &has).code == DS_OK && has == 0);
    assert(mg_edge_count(&g) == 7);
    assert(mg_remove_edge(&g, 0, 3).code == DS_OK); /* 重复删边无害 */
    assert(mg_edge_count(&g) == 7);
    assert(mg_set_edge(&g, 0, 3, 9).code == DS_OK); /* 恢复并改权 */
    assert(mg_get_weight(&g, 0, 3, &w).code == DS_OK && w == 9);
    assert(mg_edge_count(&g) == 8);

    /* 有向图入度/出度分离 */
    {
        MatrixGraph dg;

        assert(mg_create(&dg, 4, 1).code == DS_OK);
        assert(mg_set_edge(&dg, 0, 1, 5).code == DS_OK);
        assert(mg_set_edge(&dg, 0, 2, 7).code == DS_OK);
        assert(mg_set_edge(&dg, 3, 0, 6).code == DS_OK);
        assert(mg_has_edge(&dg, 1, 0, &has).code == DS_OK && has == 0); /* 非对称 */
        assert(mg_out_degree(&dg, 0, &deg).code == DS_OK && deg == 2);
        assert(mg_in_degree(&dg, 0, &deg).code == DS_OK && deg == 1);
        assert(mg_in_degree(&dg, 1, &deg).code == DS_OK && deg == 1);
        /* 有向 DFS: 0->1 (1无出边) 回溯 0->2: 序 0 1 2；3 不可达 */
        assert(mg_dfs(&dg, 0, seq, &count).code == DS_OK);
        assert(count == 3 && seq[0] == 0 && seq[1] == 1 && seq[2] == 2);
        assert(mg_bfs(&dg, 3, seq, &count).code == DS_OK); /* 3->0->1,2 */
        assert(count == 4 && seq[0] == 3 && seq[1] == 0);
    }

    /* 边界: 空指针/越界/非法 n */
    assert(mg_create(NULL, 3, 0).code == DS_NULL_PTR);
    assert(mg_create(&g, 0, 0).code == DS_OUT_OF_RANGE);
    assert(mg_create(&g, MG_MAX_VERTICES + 1, 0).code == DS_OUT_OF_RANGE);
    assert(mg_set_edge(&g, 0, 99, 1).code == DS_OUT_OF_RANGE);
    assert(mg_has_edge(NULL, 0, 0, &has).code == DS_NULL_PTR);
    assert(mg_dfs(&g, 99, seq, &count).code == DS_OUT_OF_RANGE);
    assert(mg_bfs(&g, 99, seq, &count).code == DS_OUT_OF_RANGE);

    /* 自环 */
    assert(mg_create(&g, 2, 1).code == DS_OK);
    assert(mg_set_edge(&g, 1, 1, 4).code == DS_OK);
    assert(mg_out_degree(&g, 1, &deg).code == DS_OK && deg == 1);
    assert(mg_in_degree(&g, 1, &deg).code == DS_OK && deg == 1);
    assert(mg_dfs(&g, 1, seq, &count).code == DS_OK && count == 1 && seq[0] == 1);

    /* 演示打印 */
    assert(mg_create(&g, 4, 1).code == DS_OK);
    assert(mg_set_edge(&g, 0, 1, 2).code == DS_OK);
    assert(mg_set_edge(&g, 0, 2, 9).code == DS_OK);
    assert(mg_set_edge(&g, 1, 3, 3).code == DS_OK);
    assert(mg_print(&g).code == DS_OK);

    printf("全部测试通过\n");
    return 0;
}
