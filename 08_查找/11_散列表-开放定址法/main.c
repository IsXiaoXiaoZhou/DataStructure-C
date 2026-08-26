/**
 * @file main.c
 * @brief 开放定址法散列表测试（线性探测 + 二次探测 + 墓碑）
 */

#include <assert.h>
#include <stdio.h>

#include "hash_open.h"

enum { M = 31 };

static void test_strategy(HOStrategy strategy, const char *name)
{
    HashOpen ht;
    size_t i = 0;
    int total_cmp_success = 0;
    int cnt_success = 0;

    printf("\n--- %s (m=%d) ---\n", name, M);

    assert(ho_init(&ht, M, strategy).code == DS_OK);
    assert(ho_insert(NULL, 1).code == DS_NULL_PTR);
    assert(ho_search(NULL, 1, NULL).code == DS_NULL_PTR);
    assert(ho_delete(NULL, 1).code == DS_NULL_PTR);

    assert(ho_insert(&ht, 10).code == DS_OK);
    assert(ho_insert(&ht, 10).code == DS_ERROR);
    assert(ho_search(&ht, 10, NULL).code == DS_OK);
    assert(ho_delete(&ht, 10).code == DS_OK);
    assert(ho_delete(&ht, 10).code == DS_ERROR);
    assert(ho_search(&ht, 10, NULL).code == DS_ERROR);
    assert(ho_size(&ht) == 0);
    assert(ho_destroy(&ht).code == DS_OK);

    assert(ho_init(&ht, M, strategy).code == DS_OK);
    for (i = 0; i < 20; ++i) {
        assert(ho_insert(&ht, (int)i).code == DS_OK);
    }
    assert(ho_size(&ht) == 20);
    printf("插入 0..19: n=%zu α=%.2f\n", ho_size(&ht), ho_load_factor(&ht));

    for (i = 0; i < 20; ++i) {
        int cmp = 0;
        assert(ho_search(&ht, (int)i, &cmp).code == DS_OK);
        assert(cmp >= 1 && cmp <= M);
        total_cmp_success += cmp;
        cnt_success++;
    }
    printf("成功查找平均比较次数: %.2f\n", total_cmp_success / (double)cnt_success);

    for (i = 0; i < 20; ++i) {
        assert(ho_delete(&ht, (int)i).code == DS_OK);
    }
    for (i = 0; i < 20; ++i) {
        assert(ho_search(&ht, (int)i, NULL).code == DS_ERROR);
    }
    assert(ho_size(&ht) == 0);
    assert(ho_destroy(&ht).code == DS_OK);
}

int main(void)
{
    printf("===== 散列表-开放定址法（线性探测 + 二次探测 + 墓碑） =====\n");

    test_strategy(HO_LINEAR, "线性探测");
    test_strategy(HO_QUADRATIC, "二次探测");

    printf("全部测试通过\n");
    return 0;
}
