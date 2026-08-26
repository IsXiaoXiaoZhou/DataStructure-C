/**
 * @file main.c
 * @brief B+ 树测试（叶链 + 范围查询 + 插入分裂 + 删除）
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "bplus.h"

enum { RNG = 300, CAP = 6000 };

static int ref[CAP];
static size_t ref_n;

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
    assert(bpt_search(NULL, 1).code == DS_ERROR);
    assert(bpt_inorder(t, NULL).code == DS_NULL_PTR);

    /* 基础: 首键建根 + 查找 + 删除 */
    assert(bpt_insert(&t, 42).code == DS_OK);
    assert(bpt_search(t, 42).code == DS_OK);
    assert(bpt_search(t, 99).code == DS_ERROR);
    assert(bpt_count(t) == 1);
    assert(bpt_insert(&t, 42).code == DS_ERROR);
    assert(bpt_delete(&t, 42).code == DS_OK);
    assert(t == NULL);
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

    printf("全部测试通过\n");
    return 0;
}
