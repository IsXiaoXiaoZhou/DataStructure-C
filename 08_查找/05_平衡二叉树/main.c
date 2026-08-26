/**
 * @file main.c
 * @brief 平衡二叉树（AVL）测试
 *
 * 覆盖点: LL/RR/LR/RL 四种旋转逐一构造验证 / 有序插入不退化 /
 * 中序升序不变量 / clear 复用
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "avl.h"

enum { COLLECT_CAP = 4000 };

static int collect[COLLECT_CAP];
static size_t collect_n;

static void cb_collect(int value)
{
    if (collect_n < COLLECT_CAP) {
        collect[collect_n] = value;
    }
    collect_n++;
}

/* 插入 [lo, hi] 递增整数的 AVL 高度 vs 理论界 */
static void ordered_insert_demo(void)
{
    AVLTree t = NULL;
    size_t i = 0;

    for (i = 1; i <= 1000; ++i) {
        assert(avl_insert(&t, (int)i).code == DS_OK);
    }
    printf("有序插入 1..1000: 高度 = %d   (AVL 不退化)\n",
           (int)avl_height(t));
    printf("  log2(1000)≈%d, AVL最坏高≈%d, BST会高到 1000\n",
           (int)(log(1000.0) / log(2.0)),
           (int)(1.44 * (log(1002.0) / log(2.0)) - 1.33));
    assert(avl_verify(t, NULL).code == DS_OK);
    assert(avl_destroy(&t).code == DS_OK && t == NULL);
}

int main(void)
{
    AVLTree t = NULL;
    AVLNode *node = NULL;
    size_t i = 0;

    printf("===== 平衡二叉树（AVL） =====\n");

    /* 参数错误 */
    assert(avl_insert(NULL, 1).code == DS_NULL_PTR);
    assert(avl_delete(NULL, 1).code == DS_NULL_PTR);
    assert(avl_clear(NULL).code == DS_NULL_PTR);
    assert(avl_inorder(NULL, cb_collect).code == DS_OK);
    assert(avl_inorder(t, (AVLVisit)NULL).code == DS_NULL_PTR);
    assert(avl_search(t, 5, &node).code == DS_NOT_FOUND);
    assert(avl_count(t) == 0 && avl_height(t) == 0);

    /* 四旋转逐一构造验证 */
    {
        /* LL: 3,2,1 → 右旋, 根=2 */
        AVLTree a = NULL;
        assert(avl_insert(&a, 3).code == DS_OK);
        assert(avl_insert(&a, 2).code == DS_OK);
        assert(avl_insert(&a, 1).code == DS_OK);
        assert(a->data == 2 && a->left->data == 1 && a->right->data == 3);
        assert(avl_verify(a, NULL).code == DS_OK);
        assert(avl_destroy(&a).code == DS_OK);
    }
    {
        /* RR: 1,2,3 → 左旋, 根=2 */
        AVLTree a = NULL;
        assert(avl_insert(&a, 1).code == DS_OK);
        assert(avl_insert(&a, 2).code == DS_OK);
        assert(avl_insert(&a, 3).code == DS_OK);
        assert(a->data == 2 && a->left->data == 1 && a->right->data == 3);
        assert(avl_verify(a, NULL).code == DS_OK);
        assert(avl_destroy(&a).code == DS_OK);
    }
    {
        /* LR: 3,1,2 → 先左旋(1,2)再右旋, 根=2, 左=1, 右=3 */
        AVLTree a = NULL;
        assert(avl_insert(&a, 3).code == DS_OK);
        assert(avl_insert(&a, 1).code == DS_OK);
        assert(avl_insert(&a, 2).code == DS_OK);
        assert(a->data == 2 && a->left->data == 1 && a->right->data == 3);
        assert(avl_verify(a, NULL).code == DS_OK);
        assert(avl_destroy(&a).code == DS_OK);
    }
    {
        /* RL: 1,3,2 → 先右旋(3,2)再左旋, 根=2, 左=1, 右=3 */
        AVLTree a = NULL;
        assert(avl_insert(&a, 1).code == DS_OK);
        assert(avl_insert(&a, 3).code == DS_OK);
        assert(avl_insert(&a, 2).code == DS_OK);
        assert(a->data == 2 && a->left->data == 1 && a->right->data == 3);
        assert(avl_verify(a, NULL).code == DS_OK);
        assert(avl_destroy(&a).code == DS_OK);
    }
    printf("LL / RR / LR / RL 四旋转构造验证通过\n");

    /* 有序插入演示 AVL 抗退化 */
    ordered_insert_demo();

    /* clear 复用 */
    for (i = 0; i < 30; ++i) {
        assert(avl_insert(&t, (int)i).code == DS_OK);
    }
    assert(avl_count(t) == 30 && avl_verify(t, NULL).code == DS_OK);
    assert(avl_destroy(&t).code == DS_OK && t == NULL);

    printf("全部测试通过\n");
    return 0;
}
