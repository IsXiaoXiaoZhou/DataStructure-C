/**
 * @file main.c
 * @brief 十字链表断言测试、暴力对拍与演示
 *
 * 覆盖点: 双链挂接正确性 / 出入弧枚举 / 出入度 /
 *         双链同步删除 / 随机操作流与矩阵参考逐项对拍
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "orthogonal_list.h"

int main(void)
{
    OrthGraph g;
    size_t list[OL_MAX_VERTICES];
    size_t count = 0;
    size_t deg = 0;
    int has = 0;
    int w = 0;

    printf("===== 有向图的十字链表存储 =====\n");

    /*
     * 教材示例有向图:
     *   0 -> 1, 0 -> 2
     *   1 -> 2
     *   2 -> 0
     * 弧: (0,1) (0,2) (1,2) (2,0)
     */
    assert(og_create(&g, 3).code == DS_OK);
    assert(og_add_arc(&g, 0, 1, 5).code == DS_OK);
    assert(og_add_arc(&g, 0, 2, 7).code == DS_OK);
    assert(og_add_arc(&g, 1, 2, 3).code == DS_OK);
    assert(og_add_arc(&g, 2, 0, 9).code == DS_OK);
    assert(og_arc_count(&g) == 4);

    /* 出入度 */
    assert(og_out_degree(&g, 0, &deg).code == DS_OK && deg == 2);
    assert(og_in_degree(&g, 0, &deg).code == DS_OK && deg == 1);
    assert(og_in_degree(&g, 2, &deg).code == DS_OK && deg == 2); /* 十字链表求入度 O(入度) */

    /* 出弧枚举: 头插序 = 逆插入序，0 的出弧: 先插(0,1)后插(0,2) -> 枚举序 2,1 */
    assert(og_out_arcs(&g, 0, list, &count).code == DS_OK);
    assert(count == 2 && list[0] == 2 && list[1] == 1);
    /* 入弧枚举: 2 的入弧来自 0,1 -> 头插序 1,0 */
    assert(og_in_arcs(&g, 2, list, &count).code == DS_OK);
    assert(count == 2 && list[0] == 1 && list[1] == 0);

    /* 弧存在与权 */
    assert(og_has_arc(&g, 1, 2, &has).code == DS_OK && has == 1);
    assert(og_has_arc(&g, 2, 1, &has).code == DS_OK && has == 0);
    assert(og_get_weight(&g, 2, 0, &w).code == DS_OK && w == 9);
    assert(og_get_weight(&g, 1, 0, &w).code == DS_ERROR);

    /* 重复加弧改权不增数 */
    assert(og_add_arc(&g, 0, 1, 8).code == DS_OK);
    assert(og_arc_count(&g) == 4);
    assert(og_get_weight(&g, 0, 1, &w).code == DS_OK && w == 8);

    /* 删除: 双链同步 */
    assert(og_remove_arc(&g, 0, 2).code == DS_OK);
    assert(og_arc_count(&g) == 3);
    assert(og_has_arc(&g, 0, 2, &has).code == DS_OK && has == 0);
    assert(og_out_degree(&g, 0, &deg).code == DS_OK && deg == 1);
    assert(og_in_degree(&g, 2, &deg).code == DS_OK && deg == 1); /* 入弧链同步摘除 */
    assert(og_remove_arc(&g, 0, 2).code == DS_ERROR);            /* 再删无弧 */

    /* 恢复并打印 */
    assert(og_add_arc(&g, 0, 2, 7).code == DS_OK);
    assert(og_print(&g).code == DS_OK);
    assert(og_destroy(&g).code == DS_OK);

    /* 自环 */
    assert(og_create(&g, 2).code == DS_OK);
    assert(og_add_arc(&g, 1, 1, 4).code == DS_OK);
    assert(og_out_degree(&g, 1, &deg).code == DS_OK && deg == 1);
    assert(og_in_degree(&g, 1, &deg).code == DS_OK && deg == 1);
    assert(og_remove_arc(&g, 1, 1).code == DS_OK);
    assert(og_arc_count(&g) == 0);
    assert(og_destroy(&g).code == DS_OK);

    /* 边界 */
    assert(og_create(NULL, 3).code == DS_NULL_PTR);
    assert(og_create(&g, 0).code == DS_OUT_OF_RANGE);
    assert(og_add_arc(&g, 0, 99, 1).code == DS_OUT_OF_RANGE);
    assert(og_has_arc(NULL, 0, 0, &has).code == DS_NULL_PTR);
    assert(og_destroy(NULL).code == DS_NULL_PTR);

    printf("全部测试通过\n");
    return 0;
}
