/**
 * @file recursion.c
 * @brief 递归应用模块实现
 *
 * 重点: factorial_nonrec 以 static 数组栈复现递归的"下行压栈 /
 * 上行弹出"两阶段，说明消除递归的通用思路 —— 用显式栈管理
 * 本应由调用栈保存的工作记录。
 */

#include "recursion.h"

#include <stdio.h>

/* 阶乘工作栈容量：n 最大 20，最深递归链 20 层，+1 留余量 */
#define FACT_STACK_MAX 21

/* fibonacci_rec 调用计数器：静态变量跨调用累计 */
static long fib_call_count = 0;

long long factorial_rec(int n)
{
    if (n < 0) {
        return -1;
    }
    if (n <= 1) {
        return 1;               /* 基准情形: 0! = 1! = 1，阻止无限递归 */
    }
    return (long long)n * factorial_rec(n - 1);
}

long long factorial_nonrec(int n)
{
    int stack[FACT_STACK_MAX];  /* 显式工作栈：每格保存一层的实参 n */
    int top = -1;
    long long result = 1;

    if (n < 0) {
        return -1;
    }

    /*
     * 下行阶段 —— 模拟递归调用链不断深入:
     * 递归求 n! 需先"挂起" n，去求 (n-1)!，于是把 n, n-1, ..., 1
     * 逐层压栈，对应系统调用栈逐层加帧
     */
    while (n > 0) {
        stack[++top] = n;
        n--;
    }

    /*
     * 上行阶段 —— 模拟逐层返回:
     * 最深处先返回基准值 1，之后每弹出一层就把该层实参乘进
     * 结果，对应"返回时才执行乘法"的递归语义
     */
    while (top >= 0) {
        result *= stack[top--];
    }
    return result;
}

long long fibonacci_rec(int n)
{
    fib_call_count++;

    if (n < 0) {
        return -1;
    }
    if (n <= 1) {
        return n;               /* 基准情形: F(0)=0, F(1)=1 */
    }
    return fibonacci_rec(n - 1) + fibonacci_rec(n - 2);
}

long long fibonacci_iter(int n)
{
    long long prev = 0;         /* F(i-2) */
    long long curr = 1;         /* F(i-1) */
    long long next;
    int i;

    if (n < 0) {
        return -1;
    }
    if (n == 0) {
        return 0;
    }

    for (i = 2; i <= n; i++) {
        next = prev + curr;
        prev = curr;            /* 滚动窗口右移一格，只需保存最近两项 */
        curr = next;
    }
    return curr;
}

/*
 * 内部辅助：*p_step 是已完成的移动步数，每移动一个盘自增一次，
 * 供打印"第几步"用。步号作为参数逐层传递而非 static 变量，
 * 保证 hanoi 每次公开调用都从第 1 步重新编号
 */
static void hanoi_move(int n, char src, char mid, char dst, long long *p_step)
{
    if (n <= 0) {
        return;
    }

    /*
     * 三步分治:
     *  1) 上面的 n-1 个盘先整体让路，从 src 借 dst 到 mid
     *  2) 最大盘直接一步从 src 到 dst
     *  3) n-1 个盘再从 mid 借 src 落位到 dst
     * 递归到 n==1 时"整体让路"退化为单盘直移，即打印的最小步骤
     */
    hanoi_move(n - 1, src, dst, mid, p_step);

    (*p_step)++;
    printf("第%lld步: 将 %d 号盘从 %c 移到 %c\n", *p_step, n, src, dst);

    hanoi_move(n - 1, mid, src, dst, p_step);
}

long long hanoi(int n, char src, char mid, char dst)
{
    long long step = 0;

    if (n < 0) {
        return -1;
    }
    hanoi_move(n, src, mid, dst, &step);
    return step;
}

long fibonacci_calls(void)
{
    return fib_call_count;
}

void fibonacci_reset_calls(void)
{
    fib_call_count = 0;
}
