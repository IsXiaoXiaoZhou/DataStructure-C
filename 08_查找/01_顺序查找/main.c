/**
 * @file main.c
 * @brief 顺序查找断言测试与 ASL 演示
 *
 * 覆盖点: 头尾命中 / 未命中 / 空表 / 空指针 / ASL 公式演示
 */

#include <assert.h>
#include <stdio.h>

#include "seq_search.h"

/* 用 n=10 的表演示 ASL: (n+1)/2 */
static void asl_demo(void)
{
    static const int tbl[] = { 5, 13, 19, 21, 37, 56, 64, 75, 80, 88, 92 };
    const size_t n = sizeof(tbl) / sizeof(tbl[0]);
    int tmp[16];
    size_t i = 0;
    size_t pos = 0;
    int value = 0;

    printf("n=%d  理论ASL(成功)=(n+1)/2=%.1f\n", (int)n, (double)(n + 1) / 2.0);

    /* 逐个元素成功查找，统计真实比较次数（由返回位置推出 = 位置+1） */
    for (i = 0; i < n; ++i) {
        assert(seq_search(tbl, n, tbl[i], &pos).code == DS_OK);
        assert(pos == i);
        value += (int)pos + 1; /* 累加比较次数 */
    }
    value += (int)n;           /* 一次未命中要 n+1 次比较 */
    (void)value;

    /* 哨兵版写临时表（容量要 >= n+1） */
    for (i = 0; i <= n; ++i) {
        tmp[i] = (int)tbl[i % (n + 1)];
    }
    for (i = 0; i < n; ++i) {
        assert(seq_search_sentinel(tmp, n, tbl[i], &pos).code == DS_OK);
        assert(pos == i);
    }
    assert(seq_search_sentinel(tmp, n, 999, &pos).code == DS_NOT_FOUND);
}

int main(void)
{
    int a[4];
    size_t i = 0;

    printf("===== 顺序查找（朴素 + 哨兵） =====\n");

    /* 空指针 / 空表 */
    assert(seq_search(NULL, 1, 1, &i).code == DS_NULL_PTR);
    assert(seq_search(a, 0, 1, &i).code == DS_EMPTY);
    assert(seq_search(a, 3, 1, NULL).code == DS_NULL_PTR);
    assert(seq_search_sentinel(NULL, 1, 1, &i).code == DS_NULL_PTR);
    assert(seq_search_sentinel(a, 0, 1, &i).code == DS_EMPTY);

    asl_demo();

    printf("全部测试通过\n");
    return 0;
}
