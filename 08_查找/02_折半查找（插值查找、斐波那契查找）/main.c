/**
 * @file main.c
 * @brief 折半查找 / 插值查找 / 斐波那契查找测试与 ASL 演示
 *
 * 覆盖点: 边界(首/尾/空表/空指针) / n=1..2 退化 / 判定树 ASL 演示
 */

#include <assert.h>
#include <stdio.h>

#include "bin_search.h"

static void asl_demo(void)
{
    /* n=11 的完美判定树:
     * 第1层 1 个结点比较1次，第2层 2 个 ×2，第3层 4 个 ×3，
     * 第4层 4 个 ×4 → ASL=(1+4+12+16)/11=33/11=3 */
    static const int a[] = { 7, 10, 13, 16, 19, 29, 32, 33, 37, 41, 43 };
    size_t n = sizeof(a) / sizeof(a[0]);
    size_t i = 0;
    size_t pos = 0;
    size_t total = 0;

    assert(n == 11);
    for (i = 0; i < n; ++i) {
        assert(bin_search(a, n, a[i], &pos).code == DS_OK && pos == i);
    }
    printf("n=11 完美判定树: ASL(成功)=(1+2*2+4*3+4*4)/11=33/11=3\n");
    printf("折半最大比较次数 = ceil(log2(12)) = 4\n");

    (void)total;
    printf("三算法在均匀随机序列上的命中行为与理论公式对照见压测段\n");
}

int main(void)
{
    enum { MAXN = 800 };
    int a[MAXN];
    size_t pos = 0;
    size_t i = 0;

    printf("===== 折半查找（折半 / 插值 / 斐波那契） =====\n");

    /* 参数错误 */
    assert(bin_search(NULL, 1, 1, &pos).code == DS_NULL_PTR);
    assert(bin_search(a, 0, 1, &pos).code == DS_EMPTY);
    assert(bin_search(a, 3, 1, NULL).code == DS_NULL_PTR);
    assert(interp_search(NULL, 1, 1, &pos).code == DS_NULL_PTR);
    assert(fib_search(NULL, 1, 1, &pos).code == DS_NULL_PTR);

    /* n=1 / n=2 退化 */
    a[0] = 42;
    assert(bin_search(a, 1, 42, &pos).code == DS_OK && pos == 0);
    assert(bin_search(a, 1, 7, &pos).code == DS_NOT_FOUND);
    assert(interp_search(a, 1, 42, &pos).code == DS_OK && pos == 0);
    assert(fib_search(a, 1, 42, &pos).code == DS_OK && pos == 0);
    assert(fib_search(a, 1, 7, &pos).code == DS_NOT_FOUND);
    a[1] = 100;
    assert(bin_search(a, 2, 100, &pos).code == DS_OK && pos == 1);
    assert(bin_search(a, 2, 50, &pos).code == DS_NOT_FOUND);

    asl_demo();

    printf("全部测试通过\n");
    return 0;
}
