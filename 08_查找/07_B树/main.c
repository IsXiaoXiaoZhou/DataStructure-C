/**
 * @file main.c
 * @brief B 树测试
 *
 * 覆盖点: 插入分裂(根上浮/结点分裂) / 删除借兄弟/合并 / 结构自检
 * (键序/键数界/叶同层) / 中序升序 / 全删归零
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btree.h"

enum { MAXN = 4000, COLLECT_CAP = MAXN * 2 };

typedef struct {
    int    keys[MAXN];
    size_t n;
} RefSet;

static int ref_has(const RefSet *r, int key)
{
    size_t i = 0;
    for (i = 0; i < r->n; ++i) {
        if (r->keys[i] == key) {
            return 1;
        }
    }
    return 0;
}

static void ref_add(RefSet *r, int key)
{
    if (!ref_has(r, key)) {
        assert(r->n < MAXN);
        r->keys[r->n++] = key;
    }
}

static void ref_del(RefSet *r, int key)
{
    size_t i = 0;
    for (i = 0; i < r->n; ++i) {
        if (r->keys[i] == key) {
            r->keys[i] = r->keys[r->n - 1];
            --r->n;
            return;
        }
    }
}

static int collect[COLLECT_CAP];
static size_t collect_n;

static void cb_collect(int value)
{
    if (collect_n < COLLECT_CAP) {
        collect[collect_n] = value;
    }
    collect_n++;
}

static void full_check(const BTree t, const RefSet *refs)
{
    size_t k = 0;
    size_t viol = 0;

    assert(btree_key_count(t) == refs->n);
    assert(btree_verify(t, &viol).code == DS_OK && viol == 0);
    collect_n = 0;
    assert(btree_inorder(t, cb_collect).code == DS_OK);
    assert(collect_n == refs->n);
    for (k = 1; k < collect_n; ++k) {
        assert(collect[(int)k - 1] < collect[k]);
    }
    for (k = 0; k < refs->n; ++k) {
        BTNode *nd = NULL;
        size_t  ix = 0;
        assert(btree_search(t, refs->keys[k], &nd, &ix).code == DS_OK);
        assert(nd->keys[ix] == refs->keys[k]);
    }
}

int main(void)
{
    BTree t = NULL;
    RefSet refs;
    size_t i = 0;

    memset(&refs, 0, sizeof(refs));

    printf("===== B 树（t=%d, 每结点键数 [%d,%d]） =====\n",
           BTREE_T, BTREE_MIN_KEYS, BTREE_MAX_KEYS);

    /* 参数错误 */
    assert(btree_insert(NULL, 1).code == DS_NULL_PTR);
    assert(btree_delete(NULL, 1).code == DS_NULL_PTR);
    assert(btree_destroy(NULL).code == DS_NULL_PTR);
    assert(btree_inorder(t, NULL).code == DS_NULL_PTR);
    assert(btree_inorder(t, cb_collect).code == DS_OK && collect_n == 0);
    assert(btree_search(t, 5, NULL, NULL).code == DS_NOT_FOUND);

    /* 基础: 插满一个结点触发分裂 */
    assert(btree_insert(&t, 10).code == DS_OK);
    assert(btree_insert(&t, 20).code == DS_OK);
    assert(btree_insert(&t, 5).code == DS_OK);
    assert(btree_insert(&t, 15).code == DS_OK);
    assert(btree_insert(&t, 25).code == DS_OK);
    assert(btree_key_count(t) == 5);
    assert(btree_verify(t, NULL).code == DS_OK);
    assert(btree_insert(&t, 10).code == DS_ERROR);
    assert(btree_destroy(&t).code == DS_OK && t == NULL);

    /* 序号连续插入 0..99 */
    for (i = 0; i < 100; ++i) {
        assert(btree_insert(&t, (int)i).code == DS_OK);
        ref_add(&refs, (int)i);
    }
    full_check(t, &refs);
    printf("连续插入 0..99: 高=%d 键数=%zu\n",
           (int)btree_height(t), btree_key_count(t));

    /* 删除边界: 单键根 */
    {
        BTree one = NULL;
        assert(btree_insert(&one, 42).code == DS_OK);
        assert(btree_delete(&one, 42).code == DS_OK);
        assert(one == NULL);
        assert(btree_delete(&one, 42).code == DS_NOT_FOUND);
    }

    /* 连续删除到空 */
    for (i = 0; i < 399; ++i) {
        if (btree_delete(&t, (int)i).code == DS_OK) {
            ref_del(&refs, (int)i);
        }
    }
    while (refs.n > 0) {
        assert(btree_delete(&t, refs.keys[0]).code == DS_OK);
        ref_del(&refs, refs.keys[0]);
    }
    assert(t == NULL);

    /* 全删后重新插入验证复用 */
    for (i = 0; i < 30; ++i) {
        assert(btree_insert(&t, (int)i * 3).code == DS_OK);
    }
    assert(btree_key_count(t) == 30);
    assert(btree_verify(t, NULL).code == DS_OK);
    assert(btree_destroy(&t).code == DS_OK && t == NULL);

    printf("全部测试通过\n");
    return 0;
}
