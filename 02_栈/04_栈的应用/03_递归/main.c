/**
 * @file main.c
 * @brief 递归模块测试与演示
 *
 * 测试策略: 阶乘/斐波那契对比递归版与迭代版/栈模拟版结果一致；
 * 汉诺塔验证步数公式 2^n - 1 与移动序列；
 * 借助静态计数器观测 fibonacci_rec 的指数级调用爆炸。
 */

#include <assert.h>
#include <stdio.h>

#include "recursion.h"

int main(void)
{
    long long result;
    long calls;
    int i;

    printf("=== 递归模块（阶乘 / 斐波那契 / 汉诺塔 / 递归转非递归） ===\n\n");

    /* 用例1: 阶乘基准情形 —— 0! 与 1! 均为 1 */
    assert(factorial_rec(0) == 1);
    assert(factorial_rec(1) == 1);

    /* 用例2: 阶乘递推正确性 —— 5! = 120，10! = 3628800 */
    assert(factorial_rec(5) == 120);
    assert(factorial_rec(10) == 3628800LL);

    /* 用例3: 负参数返回 -1 */
    assert(factorial_rec(-3) == -1);

    /* 用例4: 栈模拟非递归版与递归版在 [0,20] 全程一致 */
    for (i = 0; i <= 20; i++) {
        assert(factorial_nonrec(i) == factorial_rec(i));
    }
    assert(factorial_nonrec(-1) == -1);

    /* 用例5: 斐波那契前 10 项应为 0 1 1 2 3 5 8 13 21 34 */
    {
        long long expect[] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
        for (i = 0; i < 10; i++) {
            assert(fibonacci_rec(i) == expect[i]);
            assert(fibonacci_iter(i) == expect[i]);
        }
    }

    /* 用例6: 递归与迭代两版在 n=30 处结果一致（30 项约 81 万） */
    assert(fibonacci_rec(30) == fibonacci_iter(30));
    assert(fibonacci_rec(30) == 832040LL);

    /* 用例7: fibonacci_rec 调用计数 —— F(5) 分裂树共 15 个结点 */
    fibonacci_reset_calls();
    result = fibonacci_rec(5);
    calls = fibonacci_calls();
    assert(result == 5);
    assert(calls == 15);

    /* 用例8: 负参数返回 -1 */
    assert(fibonacci_rec(-2) == -1);
    assert(fibonacci_iter(-2) == -1);

    /* 用例9: 汉诺塔边界 —— 0 盘 0 步、负盘数返回 -1（均不产生移动打印） */
    assert(hanoi(0, 'A', 'B', 'C') == 0);
    assert(hanoi(-1, 'A', 'B', 'C') == -1);

    /* --- 演示段落1: 汉诺塔移动序列（含步数公式断言） --- */
    printf("--- 演示1: 汉诺塔移动序列 ---\n");
    result = hanoi(1, 'A', 'B', 'C');
    assert(result == 1);            /* 1 盘恰 1 步 */
    printf("\n");
    result = hanoi(3, 'A', 'B', 'C');
    assert(result == 7);            /* 3 盘恰 2^3-1 = 7 步 */
    printf("\n共 %lld 步（2^3 - 1）\n", result);
    printf("\n");

    /* --- 演示段落2: fibonacci_rec 调用次数随 n 指数增长 --- */
    printf("--- 演示2: fibonacci_rec 调用次数观测（对比 fibonacci_iter 的 O(n)） ---\n");
    for (i = 5; i <= 30; i += 5) {
        fibonacci_reset_calls();
        result = fibonacci_rec(i);
        calls = fibonacci_calls();
        printf("n=%2d  F(n)=%-10lld  递归调用次数=%ld\n", i, result, calls);
    }
    printf("n 每增大 5，调用次数约乘 11 倍 —— 重叠子问题导致指数级爆炸\n");

    printf("\n全部测试通过\n");
    return 0;
}
