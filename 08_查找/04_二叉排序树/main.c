/**
 * @file main.c
 * @brief 二叉排序树断言测试与退化演示
 *
 * 覆盖点: 插入/查找/删除三情况 / 重复插入拒绝 / 中序升序不变量 /
 * clear 后复用 / 有序插入退化演示(AVL 动机)
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bst.h"

enum { MAXN = 1000, COLLECT_CAP = MAXN * 2 };

/* 阶模型: 引用数组（只记存在性），线性查找作 oracle */
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

/* 中序收集（回调），供升序断言 */
static int   collect[COLLECT_CAP];
static size_t collect_n;

static void cb_collect(int value)
{
    if (collect_n < COLLECT_CAP) {
        collect[collect_n] = value;
    }
    collect_n++;
}

int main(void)
{
    BSTree t = NULL;
    BSTNode *node = NULL;
    RefSet refs;
    size_t i = 0;
    size_t k = 0;

    memset(&refs, 0, sizeof(refs));
    memset(collect, 0, sizeof(collect));

    printf("===== 二叉排序树（BST） =====\n");

    /* 参数错误 / 空树 */
    assert(bst_insert(NULL, 1).code == DS_NULL_PTR);
    assert(bst_delete(NULL, 1).code == DS_NULL_PTR);
    assert(bst_clear(NULL).code == DS_NULL_PTR);
    assert(bst_inorder(NULL, cb_collect).code == DS_OK);
    assert(bst_inorder(t, NULL).code == DS_NULL_PTR);
    assert(bst_search(t, 5, &node).code == DS_NOT_FOUND);
    assert(bst_delete(&t, 5).code == DS_NOT_FOUND);
    assert(bst_count(t) == 0 && bst_height(t) == 0);
    assert(bst_inorder(t, cb_collect).code == DS_OK && collect_n == 0);

    /* 小规模构造验证三种删除情况 */
    {
        static const int seed_keys[] = { 12, 3, 17, 9, 6, 15, 20, 2, 1 };
        for (i = 0; i < sizeof(seed_keys) / sizeof(seed_keys[0]); ++i) {
            ref_add(&refs, seed_keys[i]);
            assert(bst_insert(&t, seed_keys[i]).code == DS_OK);
        }
        assert(bst_count(t) == 9);
        assert(bst_insert(&t, 12).code == DS_ERROR);   /* 重复拒绝 */
        assert(bst_count(t) == 9);

        collect_n = 0;
        assert(bst_inorder(t, cb_collect).code == DS_OK);
        assert(collect_n == 9);
        for (i = 1; i < collect_n; ++i) {
            assert(collect[(int)i - 1] < collect[i]);
        }

        /* 1) 删叶子 1 */
        assert(bst_delete(&t, 1).code == DS_OK);
        ref_del(&refs, 1);
        assert(bst_search(t, 1, &node).code == DS_NOT_FOUND);
        /* 2) 删双孩子 12（中序前驱 9 顶替） */
        assert(bst_delete(&t, 12).code == DS_OK);
        ref_del(&refs, 12);
        assert(bst_search(t, 12, &node).code == DS_NOT_FOUND);
        assert(bst_search(t, 9, &node).code == DS_OK && node->data == 9);
        /* 3) 删双孩子 3（左子树含 2，前驱 2 顶替，前驱为叶子） */
        assert(bst_delete(&t, 3).code == DS_OK);
        ref_del(&refs, 3);
        /* 4) 删单孩子 17（右孩子 20 → 20 顶上） */
        assert(bst_delete(&t, 17).code == DS_OK);
        ref_del(&refs, 17);
        assert(bst_search(t, 20, &node).code == DS_OK);
        assert(bst_search(t, 17, &node).code == DS_NOT_FOUND);
        /* 5) 双孩子 9 */
        assert(bst_delete(&t, 9).code == DS_OK);
        ref_del(&refs, 9);

        assert(bst_count(t) == refs.n);
        assert(bst_delete(&t, 999).code == DS_NOT_FOUND);  /* 删除不存在 */

        /* 中序仍升序且与引用模型全集一致 */
        collect_n = 0;
        assert(bst_inorder(t, cb_collect).code == DS_OK);
        assert(collect_n == refs.n);
        for (i = 1; i < collect_n; ++i) {
            assert(collect[(int)i - 1] < collect[i]);
        }
        for (k = 0; k < refs.n; ++k) {
            assert(bst_search(t, refs.keys[k], &node).code == DS_OK);
        }
    }

    /* clear 复用 */
    assert(bst_clear(&t).code == DS_OK && t == NULL);
    for (i = 0; i < 50; ++i) {
        assert(bst_insert(&t, (int)i).code == DS_OK);
    }
    assert(bst_count(t) == 50);

    /* 退化演示: 有序插入 → 高度 = 结点数，说明需要 AVL/RB */
    {
        BSTree chain = NULL;
        size_t h = 0;
        for (i = 1; i <= 200; ++i) {
            assert(bst_insert(&chain, (int)i).code == DS_OK);
        }
        h = bst_height(chain);
        printf("有序插入 200 个元素 → 树高 = %d（= 结点数, 退化为单链）\n",
               (int)h);
        printf("随机插入期望高度仅约 1.39*log2(200) ≈ %d\n",
               (int)(1.39 * 7.64));
        printf("平衡树(AVL/RB)保证高度 O(log n)—— 见 05/06 模块\n");
        assert(bst_destroy(&chain).code == DS_OK);
    }

    assert(bst_destroy(&t).code == DS_OK);
    assert(t == NULL);
    printf("全部测试通过\n");
    return 0;
}
