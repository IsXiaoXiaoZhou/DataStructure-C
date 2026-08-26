/**
 * @file main.c
 * @brief 散列查找性能分析测试（ASL 精确计算 + α 扫描 + 公式对照）
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "hash_perf.h"

enum { M = 37, N_KEYS = 500 };

int main(void)
{
    int keys[N_KEYS];
    int i = 0;
    double asl_s = 0, asl_f = 0;
    double theory_s = 0, theory_f = 0;
    double alpha = 0.0;
    int cnt = 0;

    printf("===== 散列查找性能分析（ASL 精确计算 + 公式对照） =====\n");

    srand(20260825u);
    for (i = 0; i < N_KEYS; ++i) keys[i] = rand() % (M * 20);

    /* 参数检查 */
    assert(hp_chain_asl(NULL, N_KEYS, M, NULL, NULL).code == DS_NULL_PTR);
    assert(hp_chain_asl(keys, 0, M, NULL, NULL).code == DS_EMPTY);
    assert(hp_open_asl(NULL, N_KEYS, M, HP_LINEAR, NULL, NULL).code == DS_NULL_PTR);

    /* ========== 拉链法 ASL 测试 ========== */
    printf("\n--- 拉链法 (m=%d) ---\n", M);

    for (i = 1; i <= 5; ++i) {
        alpha = (double)i * 0.2;
        cnt = (int)(alpha * (double)M);
        if (cnt <= 0 || cnt > N_KEYS) continue;

        assert(hp_chain_asl(keys, (size_t)cnt, M, &asl_s, &asl_f).code == DS_OK);
        theory_s = 1.0 + alpha / 2.0;
        theory_f = alpha;

        printf("  α=%.2f n=%d: 成功 ASL=%.4f (理论=%.4f, 误差=%.4f)"
               "  不成功 ASL=%.4f (理论=%.4f, 误差=%.4f)\n",
               alpha, cnt, asl_s, theory_s, fabs(asl_s - theory_s),
               asl_f, theory_f, fabs(asl_f - theory_f));

        assert(fabs(asl_s - theory_s) < 1.0);
        assert(fabs(asl_f - theory_f) < 0.3);
    }

    /* ========== 线性探测 ASL 测试 ========== */
    printf("\n--- 线性探测 (m=%d) ---\n", M);

    for (i = 1; i <= 5; ++i) {
        alpha = (double)i * 0.15;
        cnt = (int)(alpha * (double)M);
        if (cnt <= 0 || cnt > N_KEYS) continue;

        assert(hp_open_asl(keys, (size_t)cnt, M, HP_LINEAR,
                           &asl_s, &asl_f).code == DS_OK);
        if (alpha < 1.0) {
            theory_s = (1.0 / alpha) * log(1.0 / (1.0 - alpha));
            theory_f = 1.0 / (1.0 - alpha);
        }

        printf("  α=%.2f n=%d: 成功 ASL=%.4f (理论=%.4f, 误差=%.4f)"
               "  不成功 ASL=%.4f (理论=%.4f, 误差=%.4f)\n",
               alpha, cnt, asl_s, theory_s, fabs(asl_s - theory_s),
               asl_f, theory_f, fabs(asl_f - theory_f));

        if (alpha < 0.75) {
            assert(fabs(asl_s - theory_s) < 2.0);
            assert(fabs(asl_f - theory_f) < 2.0);
        }
    }

    /* ========== 二次探测 ASL 测试 ========== */
    printf("\n--- 二次探测 (m=%d) ---\n", M);

    for (i = 1; i <= 5; ++i) {
        alpha = (double)i * 0.15;
        cnt = (int)(alpha * (double)M);
        if (cnt <= 0 || cnt > N_KEYS) continue;

        assert(hp_open_asl(keys, (size_t)cnt, M, HP_QUADRATIC,
                           &asl_s, &asl_f).code == DS_OK);
        if (alpha < 1.0) {
            theory_s = (1.0 / alpha) * log(1.0 / (1.0 - alpha));
            theory_f = 1.0 / (1.0 - alpha);
        }

        printf("  α=%.2f n=%d: 成功 ASL=%.4f (理论=%.4f)"
               "  不成功 ASL=%.4f (理论=%.4f)\n",
               alpha, cnt, asl_s, theory_s, asl_f, theory_f);
    }

    /* ========== 三种结构横向对比 ========== */
    printf("\n--- 横向对比 (α=0.5, m=%d, n=%d) ---\n", M, M / 2);
    {
        int half = M / 2;
        double c_s = 0, c_f = 0, l_s = 0, l_f = 0, q_s = 0, q_f = 0;
        hp_chain_asl(keys, (size_t)half, M, &c_s, &c_f);
        hp_open_asl(keys, (size_t)half, M, HP_LINEAR, &l_s, &l_f);
        hp_open_asl(keys, (size_t)half, M, HP_QUADRATIC, &q_s, &q_f);

        printf("  %-14s  成功ASL    不成功ASL\n", "结构");
        printf("  %-14s  %.4f     %.4f\n", "拉链法", c_s, c_f);
        printf("  %-14s  %.4f     %.4f\n", "线性探测", l_s, l_f);
        printf("  %-14s  %.4f     %.4f\n", "二次探测", q_s, q_f);
    }

    /* ========== α 扫描实验 ========== */
    hp_alpha_scan(keys, N_KEYS, M, 0.1, 0.9, 9);

    printf("全部测试通过\n");
    return 0;
}
