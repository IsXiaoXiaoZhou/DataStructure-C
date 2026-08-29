/**
 * @file main.c
 * @brief 拉链法散列表测试 + 装填因子演示
 */

#include <assert.h>
#include <stdio.h>

#include "hash_chain.h"

enum { M = 13 };

int main(void)
{
    HashChain ht;
    size_t i = 0;

    printf("===== 散列表-拉链法（除留余数 + 链地址，m=%d） =====\n", M);

    /* 参数检查 */
    assert(hc_init(&ht, M).code == DS_OK);
    assert(hc_insert(NULL, 1).code == DS_NULL_PTR);
    assert(hc_search(NULL, 1, NULL).code == DS_NULL_PTR);
    assert(hc_delete(NULL, 1).code == DS_NULL_PTR);
    assert(hc_insert(&ht, 10).code == DS_OK);
    assert(hc_insert(&ht, 10).code == DS_ERROR);  /* 重复 */
    assert(hc_search(&ht, 10, NULL).code == DS_OK);
    assert(hc_search(&ht, 99, NULL).code == DS_NOT_FOUND);
    assert(hc_delete(&ht, 10).code == DS_OK);
    assert(hc_delete(&ht, 10).code == DS_NOT_FOUND);  /* 已删（键不存在） */
    assert(hc_search(&ht, 10, NULL).code == DS_NOT_FOUND);
    assert(hc_size(&ht) == 0);
    assert(hc_destroy(&ht).code == DS_OK);

    /* ---- 批量插入 + 装填因子 ---- */
    assert(hc_init(&ht, M).code == DS_OK);
    for (i = 0; i < 50; ++i) {
        assert(hc_insert(&ht, (int)i).code == DS_OK);
    }
    assert(hc_size(&ht) == 50);
    printf("插入 0..49: n=%zu α=%.2f\n", hc_size(&ht), hc_load_factor(&ht));

    /* 比较次数统计 */
    {
        int total_cmp = 0;
        for (i = 0; i < 50; ++i) {
            int cmp = 0;
            assert(hc_search(&ht, (int)i, &cmp).code == DS_OK);
            total_cmp += cmp;
        }
        printf("成功查找总比较次数: %d, 平均: %.2f (理论≈%.2f)\n",
               total_cmp, total_cmp / 50.0, 1.0 + hc_load_factor(&ht) / 2.0);
    }
    assert(hc_destroy(&ht).code == DS_OK);

    /* 删到空 */
    assert(hc_init(&ht, M).code == DS_OK);
    for (i = 0; i < 500; ++i) {
        hc_delete(&ht, (int)i);
    }
    assert(hc_size(&ht) == 0);
    assert(hc_destroy(&ht).code == DS_OK);

    printf("全部测试通过\n");
    return 0;
}
