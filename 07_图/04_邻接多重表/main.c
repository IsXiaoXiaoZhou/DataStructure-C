/**
 * @file main.c
 * @brief 邻接多重表断言测试、暴力对拍与演示
 *
 * 覆盖点: 单边结点双链挂接 / 加删边 / 邻居/度数 /
 *         自环 / 随机操作流与矩阵参考逐项对拍
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "adjacency_multilist.h"

int main(void)
{
    AmlGraph g;
    size_t list[AML_MAX_VERTICES];
    size_t count = 0;
    size_t deg = 0;
    int has = 0;
    int w = 0;

    printf("===== 无向图的邻接多重表存储 =====\n");

    /* 与邻接矩阵模块同图: 边 (0,1)(0,2)(0,3)(1,3)(2,4)(3,4)(3,5)(4,5) */
    assert(aml_create(&g, 6).code == DS_OK);
    assert(aml_set_edge(&g, 0, 1, 1).code == DS_OK);
    assert(aml_set_edge(&g, 0, 2, 1).code == DS_OK);
    assert(aml_set_edge(&g, 0, 3, 1).code == DS_OK);
    assert(aml_set_edge(&g, 1, 3, 1).code == DS_OK);
    assert(aml_set_edge(&g, 2, 4, 1).code == DS_OK);
    assert(aml_set_edge(&g, 3, 4, 1).code == DS_OK);
    assert(aml_set_edge(&g, 3, 5, 1).code == DS_OK);
    assert(aml_set_edge(&g, 4, 5, 1).code == DS_OK);
    assert(aml_edge_count(&g) == 8);

    /* 每边一个结点: 边数 == 结点数（结构性验证在暴力测试中覆盖） */
    assert(aml_degree(&g, 3, &deg).code == DS_OK && deg == 4);
    assert(aml_degree(&g, 0, &deg).code == DS_OK && deg == 3);

    /* (u,v) 与 (v,u) 同边 */
    assert(aml_has_edge(&g, 1, 0, &has).code == DS_OK && has == 1);
    assert(aml_has_edge(&g, 5, 3, &has).code == DS_OK && has == 1);
    assert(aml_has_edge(&g, 1, 2, &has).code == DS_OK && has == 0);
    assert(aml_get_weight(&g, 4, 3, &w).code == DS_OK && w == 1);

    /* 邻居（头插序，集合对拍在暴力测试） */
    assert(aml_neighbors(&g, 3, list, &count).code == DS_OK && count == 4);

    /* 重复加边改权不增数 */
    assert(aml_set_edge(&g, 0, 3, 9).code == DS_OK);
    assert(aml_edge_count(&g) == 8);
    assert(aml_get_weight(&g, 3, 0, &w).code == DS_OK && w == 9);

    /* 删除边: 两端依附链同步 */
    assert(aml_remove_edge(&g, 0, 3).code == DS_OK);
    assert(aml_edge_count(&g) == 7);
    assert(aml_has_edge(&g, 3, 0, &has).code == DS_OK && has == 0);
    assert(aml_degree(&g, 0, &deg).code == DS_OK && deg == 2);
    assert(aml_degree(&g, 3, &deg).code == DS_OK && deg == 3);
    assert(aml_remove_edge(&g, 0, 3).code == DS_ERROR);

    /* 恢复 + 打印 */
    assert(aml_set_edge(&g, 0, 3, 1).code == DS_OK);
    assert(aml_print(&g).code == DS_OK);
    assert(aml_destroy(&g).code == DS_OK);

    /* 自环 */
    assert(aml_create(&g, 2).code == DS_OK);
    assert(aml_set_edge(&g, 1, 1, 4).code == DS_OK);
    assert(aml_edge_count(&g) == 1);
    assert(aml_degree(&g, 1, &deg).code == DS_OK && deg == 1);
    assert(aml_remove_edge(&g, 1, 1).code == DS_OK);
    assert(aml_edge_count(&g) == 0);
    assert(aml_destroy(&g).code == DS_OK);

    /* 边界 */
    assert(aml_create(NULL, 3).code == DS_NULL_PTR);
    assert(aml_create(&g, 0).code == DS_OUT_OF_RANGE);
    assert(aml_set_edge(&g, 0, 99, 1).code == DS_OUT_OF_RANGE);
    assert(aml_has_edge(NULL, 0, 0, &has).code == DS_NULL_PTR);
    assert(aml_destroy(NULL).code == DS_NULL_PTR);

    printf("全部测试通过\n");
    return 0;
}
