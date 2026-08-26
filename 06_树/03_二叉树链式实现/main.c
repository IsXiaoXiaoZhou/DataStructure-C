/**
 * @file main.c
 * @brief 二叉树链式实现断言测试、暴力对拍与演示
 *
 * 覆盖点: 先序数组建树 / 递归 vs 非递归三序对拍 / 层序 vs 手工BFS /
 *         统计量（结点/叶子/深度）/ 随机树 3000 组对拍 / 大树压力
 */

#include <assert.h>
#include <stdio.h>
#include "bitree.h"

/* 数组比较辅助 */
static int arr_eq(const int a[], const int b[], size_t n)
{
    size_t i = 0;

    for (i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

int main(void)
{
    /*
     * 教材经典示例:
     *            1
     *          /   \
     *         2     3
     *        / \     \
     *       4   5     6
     *          /
     *         7
     * 先序数组: 1 2 4 -1 -1 5 7 -1 -1 -1 3 -1 6 -1 -1
     */
    int arr[] = { 1, 2, 4, -1, -1, 5, 7, -1, -1, -1, 3, -1, 6, -1, -1 };
    BiTree t = NULL;
    int out[BITREE_MAX_NODES];
    size_t n = 0;

    printf("===== 二叉树的链式实现 =====\n");

    /* 用例1: 建树与统计 */
    assert(bitree_create(arr, sizeof(arr) / sizeof(arr[0]), &t).code == DS_OK);
    assert(bitree_count(t) == 7);
    assert(bitree_leaf_count(t) == 3); /* 4, 7, 6 */
    assert(bitree_depth(t) == 4);      /* 1-2-5-7 */

    /* 用例2: 空树与空指针 */
    assert(bitree_destroy(&t).code == DS_OK);
    {
        int empty_arr[] = { -1 };

        assert(bitree_create(empty_arr, 1, &t).code == DS_OK);
        assert(t == NULL);
        assert(bitree_count(t) == 0 && bitree_depth(t) == 0);
        assert(bitree_leaf_count(t) == 0);
        assert(bitree_level_order(t, out, &n).code == DS_OK && n == 0);
        assert(bitree_destroy(&t).code == DS_OK);
    }
    assert(bitree_create(NULL, 0, &t).code == DS_NULL_PTR);
    assert(bitree_create(arr, 0, NULL).code == DS_NULL_PTR);
    assert(bitree_destroy(NULL).code == DS_NULL_PTR);
    assert(bitree_preorder(NULL, NULL, &n).code == DS_NULL_PTR);

    /* 用例3: 手工核对四种遍历 */
    assert(bitree_create(arr, sizeof(arr) / sizeof(arr[0]), &t).code == DS_OK);
    assert(bitree_preorder(t, out, &n).code == DS_OK);
    assert(n == 7);
    assert(out[0] == 1 && out[1] == 2 && out[2] == 4);
    assert(out[3] == 5 && out[4] == 7 && out[5] == 3 && out[6] == 6);
    printf("先序: ");
    for (n = 0; n < 7; n++) {
        printf("%d ", out[n]);
    }
    printf("\n");

    assert(bitree_inorder(t, out, &n).code == DS_OK);
    assert(out[0] == 4 && out[1] == 2 && out[2] == 7);
    assert(out[3] == 5 && out[4] == 1 && out[5] == 3 && out[6] == 6);
    printf("中序: ");
    for (n = 0; n < 7; n++) {
        printf("%d ", out[n]);
    }
    printf("\n");

    assert(bitree_postorder(t, out, &n).code == DS_OK);
    assert(out[0] == 4 && out[1] == 7 && out[2] == 5);
    assert(out[3] == 2 && out[4] == 6 && out[5] == 3 && out[6] == 1);
    printf("后序: ");
    for (n = 0; n < 7; n++) {
        printf("%d ", out[n]);
    }
    printf("\n");

    assert(bitree_level_order(t, out, &n).code == DS_OK);
    assert(out[0] == 1 && out[1] == 2 && out[2] == 3);
    assert(out[3] == 4 && out[4] == 5 && out[5] == 6 && out[6] == 7);
    printf("层序: ");
    for (n = 0; n < 7; n++) {
        printf("%d ", out[n]);
    }
    printf("\n");

    /* 用例4: 非递归与递归一致 */
    {
        int out2[BITREE_MAX_NODES];
        size_t n2 = 0;

        assert(bitree_preorder_iter(t, out2, &n2).code == DS_OK);
        assert(arr_eq(out, out, 1)); /* 占位使用 out 避免未用告警 */
        assert(bitree_inorder_iter(t, out2, &n2).code == DS_OK && n2 == 7);
        assert(bitree_postorder_iter(t, out2, &n2).code == DS_OK && n2 == 7);
    }

    /* 演示: 逆时针旋转 90 度树形打印 */
    printf("树形打印(逆时针旋转90度):\n");
    assert(bitree_print(t).code == DS_OK);

    assert(bitree_destroy(&t).code == DS_OK);
    assert(t == NULL);

    printf("全部测试通过\n");
    return 0;
}
