/**
 * @file main.c
 * @brief B+ 树测试（叶链 + 范围查询 + 插入分裂 + 删除）
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bplus.h"

enum { RNG = 300, CAP = 6000, SVAL = 1000, STRESS_OPS = 2000 };

static int ref[CAP];
static size_t ref_n;

/* 压力对拍: 在树标记 + 叶链遍历收集 */
static unsigned char present[SVAL];
static int seq[SVAL];
static size_t seq_n;

static void seq_collect(int v)
{
    assert(seq_n < (size_t)SVAL);
    seq[seq_n++] = v;
}

static int ref_has(int key)
{
    size_t i = 0;
    for (i = 0; i < ref_n; ++i) {
        if (ref[i] == key) {
            return 1;
        }
    }
    return 0;
}

static void ref_add(int key)
{
    if (!ref_has(key)) {
        ref[ref_n++] = key;
    }
}

static size_t ref_range_count(int lo, int hi)
{
    size_t i = 0, c = 0;
    for (i = 0; i < ref_n; ++i) {
        if (ref[i] >= lo && ref[i] <= hi) {
            ++c;
        }
    }
    return c;
}

int main(void)
{
    BPTree t = NULL;
    size_t i = 0;
    size_t violations = 0;

    printf("===== B+ 树（叶链 + 范围查询，t=%d） =====\n", BPT_T);

    /* 参数检查 */
    bpt_init(&t);
    assert(t == NULL);
    assert(bpt_insert(NULL, 1).code == DS_NULL_PTR);
    assert(bpt_delete(NULL, 1).code == DS_NULL_PTR);
    assert(bpt_destroy(NULL).code == DS_NULL_PTR);
    assert(bpt_search(NULL, 1).code == DS_NOT_FOUND);   /* 空树查找未命中 */
    assert(bpt_delete(&t, 1).code == DS_NOT_FOUND);     /* 空树删除未命中 */
    assert(bpt_inorder(t, NULL).code == DS_NULL_PTR);

    /* 基础: 首键建根 + 查找 + 删除 */
    assert(bpt_insert(&t, 42).code == DS_OK);
    assert(bpt_search(t, 42).code == DS_OK);
    assert(bpt_search(t, 99).code == DS_NOT_FOUND);
    assert(bpt_count(t) == 1);
    assert(bpt_insert(&t, 42).code == DS_ERROR);
    assert(bpt_delete(&t, 42).code == DS_OK);
    assert(t == NULL);
    assert(bpt_delete(&t, 42).code == DS_NOT_FOUND);    /* 删后空树未命中 */
    assert(bpt_destroy(&t).code == DS_OK && t == NULL);

    /* 批量插入 0..199 */
    ref_n = 0;
    for (i = 0; i < 200; ++i) {
        assert(bpt_insert(&t, (int)i).code == DS_OK);
        ref_add((int)i);
    }
    assert(bpt_count(t) == 200);
    assert(bpt_verify(t, &violations).code == DS_OK && violations == 0);
    for (i = 0; i < 200; ++i) {
        assert(bpt_search(t, (int)i).code == DS_OK);
    }
    printf("连续插入 0..199: 高=%d 键数=%d\n",
           (int)bpt_height(t), (int)bpt_count(t));

    /* 范围查询对拍 */
    {
        int out[CAP];
        size_t got = 0;
        srand(42u);
        for (i = 0; i < 300; ++i) {
            int lo = rand() % RNG;
            int hi = lo + rand() % 50;
            size_t expect = ref_range_count(lo, hi);
            assert(bpt_range(t, lo, hi, out, CAP, &got).code == DS_OK);
            assert(got == expect);
            {
                size_t q = 0;
                for (q = 1; q < got; ++q) {
                    assert(out[q - 1] < out[q]);
                }
                for (q = 0; q < got; ++q) {
                    assert(out[q] >= lo && out[q] <= hi);
                }
            }
        }
    }

    assert(bpt_destroy(&t).code == DS_OK && t == NULL);

    /* 压力对拍: 随机交替插入/删除 2000 次(值域 0~999, 固定种子),
     * 每次删除后 bpt_verify 必须通过, 每 50 次核对键数/树高/查找抽查;
     * 结束后叶链中序遍历与插入成功的键集合(升序)逐一对拍,
     * 再全值域查找对拍 + 树高上界 n>0 时 h <= 4*log2(n+1) */
    {
        size_t expect_n = 0;
        size_t k = 0;

        memset(present, 0, sizeof present);
        seq_n = 0;
        srand(20260829u);
        for (i = 0; i < STRESS_OPS; ++i) {
            int v = rand() % SVAL;
            if (rand() % 2 == 0) {                  /* 插入 */
                DsResult r = bpt_insert(&t, v);
                if (present[v]) {
                    assert(r.code == DS_ERROR);     /* 重复拒绝 */
                } else {
                    assert(r.code == DS_OK);
                    present[v] = 1;
                    expect_n++;
                }
            } else {                                /* 删除 */
                DsResult r = bpt_delete(&t, v);
                if (present[v]) {
                    assert(r.code == DS_OK);
                    present[v] = 0;
                    expect_n--;
                } else {
                    assert(r.code == DS_NOT_FOUND); /* 未命中 */
                }
                if (t != NULL) {                    /* 每次删除后结构验证 */
                    assert(bpt_verify(t, &violations).code == DS_OK);
                    assert(violations == 0);
                }
            }
            if (i % 50 == 49) {                     /* 每 50 次: 计数/树高/查找抽查 */
                int s = 0;
                assert(bpt_count(t) == expect_n);
                assert(expect_n == 0 ||
                       (double)bpt_height(t) <=
                       4.0 * log((double)expect_n + 1.0) / log(2.0));
                for (s = 0; s < 20; ++s) {
                    int w = rand() % SVAL;
                    if (present[w]) {
                        assert(bpt_search(t, w).code == DS_OK);
                    } else {
                        assert(bpt_search(t, w).code == DS_NOT_FOUND);
                    }
                }
            }
        }

        /* 叶链中序遍历 与 插入成功键集合(升序) 逐一对拍 */
        seq_n = 0;
        assert(bpt_inorder(t, seq_collect).code == DS_OK);
        {
            size_t j = 0;
            for (k = 0; k < SVAL; ++k) {
                if (present[k]) {
                    assert(j < seq_n && seq[j] == (int)k);
                    ++j;
                }
            }
            assert(j == seq_n && seq_n == expect_n);
        }

        /* 全值域查找对拍: 在树必命中, 不在必未命中 */
        for (k = 0; k < SVAL; ++k) {
            if (present[k]) {
                assert(bpt_search(t, (int)k).code == DS_OK);
            } else {
                assert(bpt_search(t, (int)k).code == DS_NOT_FOUND);
            }
        }
        assert(bpt_count(t) == expect_n);
        assert(expect_n == 0 ||
               (double)bpt_height(t) <=
               4.0 * log((double)expect_n + 1.0) / log(2.0));
        printf("压力对拍 2000 次随机插删: 剩余键数=%d 高=%d 树高上界=%.1f\n",
               (int)expect_n, (int)bpt_height(t),
               expect_n > 0 ? 4.0 * log((double)expect_n + 1.0) / log(2.0)
                            : 0.0);
        assert(bpt_destroy(&t).code == DS_OK && t == NULL);
    }

    /* 清空对拍 1: 建满 0..999 后按升序逐个删除至空 —— 高频摘除子树
     * 最左空叶, 强制触发单孩内部结点级联收缩与祖先路由键重导;
     * 每删一次: verify + 键数 + 树高上界 + 未删键全量查找对拍
     * (路由键失准会立即暴露为假未命中) */
    {
        size_t w = 0;

        for (i = 0; i < SVAL; ++i) {
            assert(bpt_insert(&t, (int)i).code == DS_OK);
        }
        assert(bpt_count(t) == SVAL);
        for (i = 0; i < SVAL; ++i) {
            size_t left = SVAL - i - 1;
            assert(bpt_delete(&t, (int)i).code == DS_OK);
            assert(bpt_count(t) == left);
            if (t != NULL) {
                assert(bpt_verify(t, &violations).code == DS_OK);
                assert(violations == 0);
                assert((double)bpt_height(t) <=
                       4.0 * log((double)left + 1.0) / log(2.0));
            }
            for (w = i + 1; w < SVAL; ++w) {
                assert(bpt_search(t, (int)w).code == DS_OK);
            }
        }
        assert(t == NULL);
    }

    /* 清空对拍 2: 乱序插入 0..999, 再随机顺序清空 —— 覆盖任意位置
     * 空叶摘除与收缩的随机组合, 检查同清空对拍 1 */
    {
        int perm[SVAL];
        size_t w = 0;

        for (i = 0; i < SVAL; ++i) {
            perm[i] = (int)i;
        }
        srand(20260830u);
        for (i = SVAL - 1; i > 0; --i) {        /* 洗牌得插入序 */
            int jp = rand() % (i + 1);
            int tmp = perm[i];
            perm[i] = perm[jp];
            perm[jp] = tmp;
        }
        for (i = 0; i < SVAL; ++i) {
            assert(bpt_insert(&t, perm[i]).code == DS_OK);
        }
        assert(bpt_count(t) == SVAL);
        memset(present, 0, sizeof present);
        for (i = 0; i < SVAL; ++i) {
            present[perm[i]] = 1;
        }
        for (i = SVAL - 1; i > 0; --i) {        /* 再洗牌得删除序 */
            int jp = rand() % (i + 1);
            int tmp = perm[i];
            perm[i] = perm[jp];
            perm[jp] = tmp;
        }
        for (i = 0; i < SVAL; ++i) {
            size_t left = SVAL - i - 1;
            assert(bpt_delete(&t, perm[i]).code == DS_OK);
            present[perm[i]] = 0;
            assert(bpt_count(t) == left);
            if (t != NULL) {
                assert(bpt_verify(t, &violations).code == DS_OK);
                assert(violations == 0);
                assert((double)bpt_height(t) <=
                       4.0 * log((double)left + 1.0) / log(2.0));
            }
            for (w = 0; w < SVAL; ++w) {
                if (present[w]) {
                    assert(bpt_search(t, w).code == DS_OK);
                }
            }
        }
        assert(t == NULL);
        printf("清空对拍: 升序/乱序各删 1000 键至空, 每删验证 + 查找对拍通过\n");
    }

    printf("全部测试通过\n");

#ifdef BPT_LEAK_CHECK
    /* 泄漏检查: 编译加 -DBPT_LEAK_CHECK 启用(gcc -Wall -Wextra -std=c99
     * -DBPT_LEAK_CHECK *.c -o demo.exe), 比对 bplus.c 计数 wrapper 的
     * malloc/free 进出次数; 默认编译不定义此宏, 本块不参与、输出不变 */
    if (bpt_lc_in() == bpt_lc_out() && bpt_lc_in() > 0) {
        printf("[LEAK CHECK] HEAP OK: malloc=%u free=%u\n",
               (unsigned)bpt_lc_in(), (unsigned)bpt_lc_out());
    } else {
        printf("[LEAK CHECK] HEAP LEAK: malloc=%u free=%u\n",
               (unsigned)bpt_lc_in(), (unsigned)bpt_lc_out());
        return 1;
    }
#endif
    return 0;
}
