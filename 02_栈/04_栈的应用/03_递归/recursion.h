/**
 * @file recursion.h
 * @brief 递归应用模块接口定义（阶乘 / 斐波那契 / 汉诺塔 / 栈模拟转非递归）
 *
 * 设计要点:
 *   1. 三个经典递归原型 —— 阶乘演示"自相似子问题"，
 *      斐波那契演示"重叠子问题导致的指数爆炸"，
 *      汉诺塔演示"多分支递归的移动序列"
 *   2. factorial_nonrec 用工作记录栈把递归改写为迭代 ——
 *      揭示"递归 = 系统隐式栈"的本质
 * 复杂度: factorial/fibonacci_iter 为 O(n)；fibonacci_rec 为 O(2^n)；
 *         hanoi 为 O(2^n) 但属问题固有规模（2^n-1 步不可省略）
 */

#ifndef RECURSION_H
#define RECURSION_H

/**
 * @brief 递归求 n 的阶乘
 * @param n 非负整数，合法范围 [0, 20]（21! 起 long long 溢出）
 * @return n!；n 为负返回 -1 表示参数非法
 * @note  递归式 n! = n * (n-1)!，基准情形 0! = 1
 */
long long factorial_rec(int n);

/**
 * @brief 用 static 栈模拟递归过程求阶乘（非递归版）
 * @param n 非负整数，合法范围 [0, 20]
 * @return n!；n 为负返回 -1
 * @note  把递归调用链显式化：下行阶段依次压入 n..1 模拟递归深入，
 *        回升阶段逐层弹出累乘模拟返回值向上传递
 */
long long factorial_nonrec(int n);

/**
 * @brief 递归求斐波那契数列第 n 项（F(0)=0, F(1)=1）
 * @param n 非负整数，n 增大 1 调用量约乘 1.618（黄金比）
 * @return F(n)；n 为负返回 -1
 * @note  每次调用分裂出两个子调用且子树大量重叠，复杂度 O(2^n)，
 *        调用次数由 fibonacci_calls() 观测
 */
long long fibonacci_rec(int n);

/**
 * @brief 迭代法求斐波那契数列第 n 项
 * @param n 非负整数
 * @return F(n)；n 为负返回 -1
 * @note  只保存相邻两项滚动推进，复杂度 O(n)、空间 O(1)，
 *        与递归版形成效率对比
 */
long long fibonacci_iter(int n);

/**
 * @brief 汉诺塔：把 n 个盘从 src 柱经 mid 柱移到 dst 柱，打印每步移动
 * @param n   盘数（自上而下编号 1..n）
 * @param src 起始柱、mid 中转柱、dst 目标柱（传柱名如 'A','B','C'）
 * @return 总移动步数，恒为 2^n - 1；n 为负返回 -1
 * @note  分治三部曲: 先移上 n-1 个到 mid，再移最大盘到 dst，
 *        最后把 n-1 个从 mid 移到 dst
 */
long long hanoi(int n, char src, char mid, char dst);

/**
 * @brief 读取 fibonacci_rec 自最近一次 fibonacci_reset_calls 后的累计调用次数
 * @return 调用次数（含首次调用自身）
 */
long fibonacci_calls(void);

/**
 * @brief 清零 fibonacci_rec 调用计数器
 */
void fibonacci_reset_calls(void);

#endif /* RECURSION_H */
