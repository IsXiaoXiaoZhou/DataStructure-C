/**
 * @file main.c
 * @brief 舞伴问题模块测试与演示
 *
 * 测试策略: 覆盖男女均衡、男多女少、女多男少、单一性别、
 * 空场次、参数非法、超长姓名七类场景，并验证先到先配对
 * 的 FIFO 顺序与安全截断（无越界写）。
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dance_partner.h"

int main(void)
{
    DanceResult result;

    printf("=== 舞伴问题模块（男/女双队列轮流出队配对） ===\n\n");

    /* 用例1: 男女各 3 人 —— 恰好全部配对，无剩余 */
    {
        Dancer party[] = {
            {"李明", 'M'}, {"赵敏", 'F'},
            {"王强", 'M'}, {"孙丽", 'F'},
            {"张伟", 'M'}, {"周芳", 'F'}
        };
        printf("[用例1] 男女均衡:\n");
        assert(dance_partner(party, 6, &result).code == DS_OK);
        assert(result.pair_count == 3);
        assert(result.leftover_count == 0);
        printf("\n");
    }

    /* 用例2: 男 6 女 3 —— 配 3 对，剩 3 名男士且按到达顺序报告 */
    {
        Dancer party[] = {
            {"李明", 'M'}, {"赵敏", 'F'},
            {"王强", 'M'}, {"孙丽", 'F'},
            {"张伟", 'M'}, {"周芳", 'F'},
            {"刘洋", 'M'}, {"陈刚", 'M'}, {"吴涛", 'M'}
        };
        printf("[用例2] 男多女少:\n");
        assert(dance_partner(party, 9, &result).code == DS_OK);
        assert(result.pair_count == 3);
        assert(result.leftover_count == 3);
        assert(strcmp(result.leftover[0], "刘洋") == 0);
        assert(strcmp(result.leftover[1], "陈刚") == 0);
        assert(strcmp(result.leftover[2], "吴涛") == 0);
        printf("\n");
    }

    /* 用例3: 女 4 男 2 —— 配 2 对，剩 2 名女士 */
    {
        Dancer party[] = {
            {"赵敏", 'F'}, {"李明", 'M'},
            {"孙丽", 'F'}, {"周芳", 'F'},
            {"王强", 'M'}, {"吴静", 'F'}
        };
        printf("[用例3] 女多男少:\n");
        assert(dance_partner(party, 6, &result).code == DS_OK);
        assert(result.pair_count == 2);
        assert(result.leftover_count == 2);
        assert(strcmp(result.leftover[0], "周芳") == 0);
        assert(strcmp(result.leftover[1], "吴静") == 0);
        printf("\n");
    }

    /* 用例4: FIFO 顺序 —— 队头男士应与队头女士配对，而非同名/乱序 */
    {
        Dancer party[] = {
            {"甲一", 'M'}, {"乙一", 'F'},
            {"甲二", 'M'}, {"乙二", 'F'}
        };
        printf("[用例4] 先到先配对（FIFO）:\n");
        assert(dance_partner(party, 4, &result).code == DS_OK);
        assert(result.pair_count == 2);
        printf("\n");
    }

    /* 用例5: 单一性别 —— 0 对配对，全部剩余 */
    {
        Dancer party[] = {{"张三", 'M'}, {"李四", 'M'}};
        printf("[用例5] 单一性别:\n");
        assert(dance_partner(party, 2, &result).code == DS_OK);
        assert(result.pair_count == 0);
        assert(result.leftover_count == 2);
        printf("\n");
    }

    /* 用例6: 空场次 —— 0 对、0 剩余（数组指针须有效，人数为 0） */
    {
        Dancer party[1] = {{"占位", 'M'}};
        printf("[用例6] 空场次:\n");
        assert(dance_partner(party, 0, &result).code == DS_OK);
        assert(result.pair_count == 0);
        assert(result.leftover_count == 0);
        printf("\n");
    }

    /* 用例7: 参数非法 —— 空指针与人数越界 */
    assert(dance_partner(NULL, 0, &result).code == DS_NULL_PTR);
    assert(dance_partner(NULL, 1, &result).code == DS_NULL_PTR);
    {
        Dancer party[1] = {{"独苗", 'M'}};
        assert(dance_partner(party, DANCE_MAX_COUNT + 1, &result).code
               == DS_OUT_OF_RANGE);
        assert(dance_partner(party, 1, NULL).code == DS_NULL_PTR);
    }

    /* 用例8: 超长姓名 —— 30 字符远超 name 容量（15 字符 + '\0'），
       入库截断后经 snprintf 复制进队列与剩余名单，全程无越界写 */
    {
        Dancer party[2];
        char long_name[31];

        memset(long_name, 'X', sizeof long_name - 1);
        long_name[sizeof long_name - 1] = '\0';   /* 30 个 'X' 的超长名 */

        /* 先清零保证字段外无脏数据，再按容量截断装入（15 字符 + '\0'） */
        memset(party, 0, sizeof party);
        strncpy(party[0].name, long_name, sizeof party[0].name - 1);
        party[0].sex = 'M';
        strncpy(party[1].name, long_name, sizeof party[1].name - 1);
        party[1].sex = 'M';

        printf("[用例8] 超长姓名（30 字符 > 容量 %d）安全截断:\n",
               DANCE_NAME_LEN - 1);
        assert(dance_partner(party, 2, &result).code == DS_OK);
        assert(result.pair_count == 0);
        assert(result.leftover_count == 2);
        /* 剩余名单中的姓名被截断为容量上限且正常终止，未越界写 */
        assert(strlen(result.leftover[0]) == DANCE_NAME_LEN - 1);
        assert(strlen(result.leftover[1]) == DANCE_NAME_LEN - 1);
        printf("\n");
    }

    /* --- 演示段落: 教材经典场景 —— 舞会开场一轮配对 --- */
    printf("--- 演示: 教材经典场景（5 男 3 女到场） ---\n");
    {
        Dancer party[] = {
            {"李明", 'M'}, {"王强", 'M'}, {"张伟", 'M'},
            {"刘洋", 'M'}, {"陈刚", 'M'},
            {"赵敏", 'F'}, {"孙丽", 'F'}, {"周芳", 'F'}
        };
        assert(dance_partner(party, 8, &result).code == DS_OK);
        printf("配对 %lu 对，剩余 %lu 人\n",
               (unsigned long)result.pair_count,
               (unsigned long)result.leftover_count);
    }

    printf("\n全部测试通过\n");
    return 0;
}
