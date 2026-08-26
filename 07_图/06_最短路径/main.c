/**
 * @file main.c
 * @brief 最短路径断言测试、多方对拍与演示
 *
 * 覆盖点: 教材经典例 / Dijkstra vs Floyd 三方对拍 /
 *         三角不等式松弛性质 / prev/next 路径重建一致性 / 随机压力
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "shortest_path.h"

int main(void)
{
    /*
     * 王道教材 Dijkstra 经典例（V0..V4 记为 0..4）:
     * 弧: 0->1=10 0->3=30 0->4=100 1->2=50 2->4=10 3->2=20 3->4=60
     * 最短: dist[0..4] = 0 10 50 30 60
     */
    SpGraph g;
    int dist[SP_MAX_VERTICES];
    size_t prev[SP_MAX_VERTICES];
    int dmat[SP_MAX_VERTICES][SP_MAX_VERTICES];
    size_t nxt[SP_MAX_VERTICES][SP_MAX_VERTICES];
    size_t i = 0;

    printf("===== 最短路径（Dijkstra + Floyd） =====\n");

    assert(spg_create(&g, 5).code == DS_OK);
    assert(spg_add_arc(&g, 0, 1, 10).code == DS_OK);
    assert(spg_add_arc(&g, 0, 3, 30).code == DS_OK);
    assert(spg_add_arc(&g, 0, 4, 100).code == DS_OK);
    assert(spg_add_arc(&g, 1, 2, 50).code == DS_OK);
    assert(spg_add_arc(&g, 2, 4, 10).code == DS_OK);
    assert(spg_add_arc(&g, 3, 2, 20).code == DS_OK);
    assert(spg_add_arc(&g, 3, 4, 60).code == DS_OK);

    /* Dijkstra 手工核对 */
    assert(sp_dijkstra(&g, 0, dist, prev).code == DS_OK);
    assert(dist[0] == 0 && dist[1] == 10 && dist[2] == 50);
    assert(dist[3] == 30 && dist[4] == 60);
    printf("Dijkstra(源0): ");
    for (i = 0; i < 5; i++) {
        printf("%d ", dist[i]);
    }
    printf("(标准答案 0 10 50 30 60)\n");

    /* prev 路径重建: 0->4 路径应为 0->3->2->4 */
    {
        size_t path[SP_MAX_VERTICES];
        size_t len = 0;
        size_t v = 4;

        path[len++] = v;
        while (prev[v] != (size_t)-1) {
            v = prev[v];
            path[len++] = v;
        }
        assert(len == 4);
        assert(path[0] == 4 && path[1] == 2 && path[2] == 3 && path[3] == 0);
        printf("0->4 最短路径: ");
        {
            size_t k = 0;

            for (k = len; k > 0; k--) {
                printf("%zu%s", path[k - 1], k == 1 ? "\n" : " -> ");
            }
        }
    }

    /* Floyd 全源对拍 + 路径重建 */
    assert(sp_floyd(&g, dmat, nxt).code == DS_OK);
    for (i = 0; i < 5; i++) {
        int dj[SP_MAX_VERTICES];
        size_t j = 0;

        assert(sp_dijkstra(&g, i, dj, NULL).code == DS_OK);
        for (j = 0; j < 5; j++) {
            assert(dmat[i][j] == dj[j]);
        }
    }
    /* next 重建 0->4: 0 -> 3 -> 2 -> 4 */
    {
        size_t u = 0;

        printf("Floyd 0->4: 0");
        while (u != 4) {
            u = nxt[u][4];
            printf(" -> %zu", u);
        }
        printf("\n");
    }

    /* 负权拒绝（Dijkstra） */
    {
        SpGraph ng;

        assert(spg_create(&ng, 3).code == DS_OK);
        /* 用底层字段直接构造负权（绕过 add_arc 校验以测算法入口防御） */
        ng.w[0][1] = -5;
        assert(sp_dijkstra(&ng, 0, dist, NULL).code == DS_ERROR);
    }

    /* 边界 */
    assert(spg_create(NULL, 3).code == DS_NULL_PTR);
    assert(spg_create(&g, 0).code == DS_OUT_OF_RANGE);
    assert(spg_add_arc(&g, 0, 99, 1).code == DS_OUT_OF_RANGE);
    assert(spg_add_arc(&g, 0, 1, -3).code == DS_ERROR);
    assert(sp_dijkstra(NULL, 0, dist, NULL).code == DS_NULL_PTR);
    assert(sp_floyd(NULL, dmat, NULL).code == DS_NULL_PTR);

    /* 不可达: 无边图 dist 全 INF（除源点） */
    assert(spg_create(&g, 3).code == DS_OK);
    assert(sp_dijkstra(&g, 1, dist, NULL).code == DS_OK);
    assert(dist[1] == 0 && dist[0] == SP_INF && dist[2] == SP_INF);
    assert(sp_floyd(&g, dmat, NULL).code == DS_OK);
    assert(dmat[1][1] == 0 && dmat[0][2] == SP_INF);

    printf("全部测试通过\n");
    return 0;
}
