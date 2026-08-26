/**
 * @file main.c
 * @brief 分块查找测试与边界验证
 *
 * 覆盖点: 建索引正确性 / 命中/未命中 / 边界(key 小于最小、
 * 大于最大、恰为块最大值)
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "blk_search.h"

enum { N = 120, BS = 10, IDX_CAP = 64 };

/* 构造: 块间有序（第 k 块值域 [k*1000, k*1000+799]）、块内随机 */
static void build_blocks(int *a, size_t n, size_t bs)
{
    size_t k = 0, j = 0;

    for (k = 0; k < (n + bs - 1) / bs; ++k) {
        for (j = 0; j < bs; ++j) {
            size_t idx = k * bs + j;
            if (idx >= n) {
                return;
            }
            a[idx] = (int)(k * 1000 + (size_t)(rand() % 800));
        }
    }
}

int main(void)
{
    static BlockIndex idx[IDX_CAP];
    int a[N];
    size_t nb = 0;
    size_t pos = 0;
    size_t cc = 0;
    size_t i = 0;

    printf("===== 分块查找（索引顺序查找） =====\n");

    /* 参数错误 */
    assert(blk_build_index(NULL, 10, 2, idx, IDX_CAP, &nb).code == DS_NULL_PTR);
    assert(blk_build_index(a, 0, 2, idx, IDX_CAP, &nb).code == DS_EMPTY);
    assert(blk_build_index(a, 10, 0, idx, IDX_CAP, &nb).code == DS_OUT_OF_RANGE);
    assert(blk_search(NULL, idx, 1, 5, &pos, &cc).code == DS_NULL_PTR);
    assert(blk_search(a, NULL, 1, 5, &pos, &cc).code == DS_NULL_PTR);
    assert(blk_search(a, idx, 0, 5, &pos, &cc).code == DS_EMPTY);

    srand(20260825u);

    /* 固定场景边界: n=120 bs=10 → b=12 s=10，逐块验证块最大值命中 */
    build_blocks(a, N, BS);
    assert(blk_build_index(a, N, BS, idx, IDX_CAP, &nb).code == DS_OK);
    assert(nb == 12);
    for (i = 0; i < nb; ++i) {
        size_t k = 0;
        int maxv = a[idx[i].start];
        /* 块内真实最大值（与索引一致） */
        for (k = 1; k < idx[i].len; ++k) {
            if (a[idx[i].start + k] > maxv) {
                maxv = a[idx[i].start + k];
            }
        }
        assert(blk_search(a, idx, nb, maxv, &pos, &cc).code == DS_OK);
        assert(idx[i].max_key == maxv);
        assert(blk_search(a, idx, nb, -1, &pos, &cc).code == DS_NOT_FOUND);
        assert(blk_search(a, idx, nb, 100000, &pos, &cc).code == DS_NOT_FOUND);
    }

    printf("全部测试通过\n");
    return 0;
}
