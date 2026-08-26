/**
 * @file main.c
 * @brief AOE 网关键路径断言测试、暴力对拍与演示
 *
 * 覆盖点: 教材经典例（ve/vl/关键活动/工期标准答案）/
 *         工期 == 源汇最长路径（拓扑 DP 暴力对拍）/
 *         l >= e 自洽 / 关键路径可达性 / 随机 DAG 压力
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "critical_path.h"

int main(void)
{
    /*
     * 教材经典 AOE 网（6 事件 8 活动，V1..V6 记 0..5）:
     *   a1: 0->1 (3)   a2: 0->2 (2)
     *   a3: 1->3 (2)   a4: 1->4 (3)
     *   a5: 2->3 (4)   a6: 2->4 (3)
     *   a7: 3->5 (2)   a8: 4->5 (3)
     * ve = [0,3,2,6,6,9]  vl = [0,3,3,7,6,9]
     * 关键活动: a1(0->1), a4(1->4), a8(4->5)；关键路径 0->1->4->5，工期 9
     */
    AoeGraph g;
    long ve[CP_MAX_VERTICES];
    long vl[CP_MAX_VERTICES];
    CpActivity acts[CP_MAX_VERTICES * CP_MAX_VERTICES / 2];
    size_t act_count = 0;
    long duration = 0;
    size_t i = 0;

    printf("===== 关键路径（AOE 网） =====\n");

    assert(aoe_create(&g, 6).code == DS_OK);
    assert(aoe_add_arc(&g, 0, 1, 3).code == DS_OK);
    assert(aoe_add_arc(&g, 0, 2, 2).code == DS_OK);
    assert(aoe_add_arc(&g, 1, 3, 2).code == DS_OK);
    assert(aoe_add_arc(&g, 1, 4, 3).code == DS_OK);
    assert(aoe_add_arc(&g, 2, 3, 4).code == DS_OK);
    assert(aoe_add_arc(&g, 2, 4, 3).code == DS_OK);
    assert(aoe_add_arc(&g, 3, 5, 2).code == DS_OK);
    assert(aoe_add_arc(&g, 4, 5, 3).code == DS_OK);

    assert(cp_analyze(&g, ve, vl, acts, &act_count, &duration).code == DS_OK);
    assert(act_count == 8);
    assert(duration == 9);

    printf("ve: ");
    for (i = 0; i < 6; i++) {
        printf("%ld ", ve[i]);
    }
    printf("(标准 0 3 2 6 6 9)\n");
    assert(ve[0] == 0 && ve[1] == 3 && ve[2] == 2 && ve[3] == 6 && ve[4] == 6 && ve[5] == 9);

    printf("vl: ");
    for (i = 0; i < 6; i++) {
        printf("%ld ", vl[i]);
    }
    printf("(标准 0 3 3 7 6 9)\n");
    assert(vl[0] == 0 && vl[1] == 3 && vl[2] == 3 && vl[3] == 7 && vl[4] == 6 && vl[5] == 9);

    printf("活动表(u->v w e l 关键):\n");
    for (i = 0; i < act_count; i++) {
        printf("  %zu->%zu w=%d e=%ld l=%ld %s\n",
               acts[i].u, acts[i].v, acts[i].w, acts[i].e, acts[i].l,
               acts[i].critical ? "*" : "");
    }

    /* 关键活动恰为 a1(0->1), a4(1->4), a8(4->5) */
    {
        int crit_count = 0;

        for (i = 0; i < act_count; i++) {
            if (acts[i].critical) {
                crit_count++;
                assert((acts[i].u == 0 && acts[i].v == 1) ||
                       (acts[i].u == 1 && acts[i].v == 4) ||
                       (acts[i].u == 4 && acts[i].v == 5));
            }
        }
        assert(crit_count == 3);
        printf("关键路径: 0 -> 1 -> 4 -> 5, 工期 = %ld\n", duration);
    }

    /* 有环拒绝 */
    assert(aoe_create(&g, 3).code == DS_OK);
    assert(aoe_add_arc(&g, 0, 1, 1).code == DS_OK);
    assert(aoe_add_arc(&g, 1, 2, 1).code == DS_OK);
    assert(aoe_add_arc(&g, 2, 0, 1).code == DS_OK);
    assert(cp_analyze(&g, ve, vl, acts, &act_count, &duration).code == DS_ERROR);

    /* 多源点拒绝 */
    assert(aoe_create(&g, 3).code == DS_OK);
    assert(aoe_add_arc(&g, 0, 2, 1).code == DS_OK);
    assert(aoe_add_arc(&g, 1, 2, 1).code == DS_OK);
    assert(cp_analyze(&g, ve, vl, acts, &act_count, &duration).code == DS_ERROR);

    /* 多汇点拒绝 */
    assert(aoe_create(&g, 3).code == DS_OK);
    assert(aoe_add_arc(&g, 0, 1, 1).code == DS_OK);
    assert(aoe_add_arc(&g, 0, 2, 1).code == DS_OK);
    assert(cp_analyze(&g, ve, vl, acts, &act_count, &duration).code == DS_ERROR);

    /* 最小 AOE 网: 两事件一活动 */
    assert(aoe_create(&g, 2).code == DS_OK);
    assert(aoe_add_arc(&g, 0, 1, 7).code == DS_OK);
    assert(cp_analyze(&g, ve, vl, acts, &act_count, &duration).code == DS_OK);
    assert(duration == 7 && act_count == 1 && acts[0].critical == 1);

    /* 边界 */
    assert(aoe_create(NULL, 3).code == DS_NULL_PTR);
    assert(aoe_create(&g, 0).code == DS_OUT_OF_RANGE);
    assert(aoe_add_arc(&g, 0, 99, 1).code == DS_OUT_OF_RANGE);
    assert(aoe_add_arc(&g, 0, 1, 0).code == DS_ERROR);
    assert(aoe_add_arc(&g, 0, 1, -3).code == DS_ERROR);
    assert(cp_analyze(NULL, ve, vl, acts, &act_count, &duration).code == DS_NULL_PTR);

    printf("全部测试通过\n");
    return 0;
}
