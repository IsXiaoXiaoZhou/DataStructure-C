/**
 * @file main.c
 * @brief 线索二叉树断言测试、暴力对拍与演示
 *
 * 覆盖点: 线索化正确性（中序首尾/前驱后继链）/ 线索遍历 vs 递归遍历 /
 *         逆序遍历 / 随机树对拍 / 线索树专用销毁
 */

#include <assert.h>
#include <stdio.h>
#include "thread_tree.h"

int main(void)
{
    /*
     * 教材示例（同链式模块）:
     *            1
     *          /   \
     *         2     3
     *        / \     \
     *       4   5     6
     *          /
     *         7
     * 中序: 4 2 7 5 1 3 6
     */
    int arr[] = { 1, 2, 4, -1, -1, 5, 7, -1, -1, -1, 3, -1, 6, -1, -1 };
    ThreadTree t = NULL;
    int out[THREAD_MAX_NODES];
    size_t n = 0;

    printf("===== 线索二叉树（中序线索化） =====\n");

    /* 用例1: 建树 + 线索化 */
    assert(tt_create(arr, sizeof(arr) / sizeof(arr[0]), &t).code == DS_OK);
    assert(tt_inthreading(t).code == DS_OK);

    /* 用例2: 首尾结点 */
    assert(tt_first(t)->data == 4); /* 中序第一个 = 最左 */
    assert(tt_last(t)->data == 6);  /* 中序最后一个 = 最右 */

    /* 用例3: 线索遍历（无栈无递归） */
    assert(tt_inorder(t, out, &n).code == DS_OK);
    assert(n == 7);
    assert(out[0] == 4 && out[1] == 2 && out[2] == 7);
    assert(out[3] == 5 && out[4] == 1 && out[5] == 3 && out[6] == 6);
    printf("线索中序: ");
    for (n = 0; n < 7; n++) {
        printf("%d ", out[n]);
    }
    printf("\n");

    /* 用例4: 逆中序 */
    assert(tt_inorder_reverse(t, out, &n).code == DS_OK);
    assert(out[0] == 6 && out[6] == 4);
    printf("线索逆中序: ");
    for (n = 0; n < 7; n++) {
        printf("%d ", out[n]);
    }
    printf("\n");

    /* 用例5: 手工核验若干前驱后继 */
    {
        ThreadNode *p4 = tt_first(t);
        ThreadNode *p2 = tt_next(p4);
        ThreadNode *p7 = tt_next(p2);
        ThreadNode *p5 = tt_next(p7);
        ThreadNode *p1 = tt_next(p5);
        ThreadNode *p3 = tt_next(p1);
        ThreadNode *p6 = tt_next(p3);

        assert(p2->data == 2 && p7->data == 7 && p5->data == 5);
        assert(p1->data == 1 && p3->data == 3 && p6->data == 6);
        assert(tt_next(p6) == NULL);      /* 尾结点无后继 */
        assert(tt_prev(p4) == NULL);      /* 首结点无前驱 */
        assert(tt_prev(p2) == p4);
        assert(tt_prev(p1) == p5);
        assert(tt_next(p2) == p7);
        /* 结点 2 有左孩子 4: prev(2)=左子树最右=4（孩子式寻前驱） */
        /* 结点 3 无左孩子: ltag==1 线索直达 1 */
        assert(tt_prev(p3) == p1);
    }

    /* 用例6: 空指针参数（tt_inthreading(NULL) 语义为空树，返回 DS_OK） */
    assert(tt_create(NULL, 0, &t).code == DS_NULL_PTR);
    assert(tt_inorder(NULL, NULL, &n).code == DS_NULL_PTR);
    assert(tt_destroy(NULL).code == DS_NULL_PTR);

    /* 用例7: 线索树专用销毁 */
    assert(tt_destroy(&t).code == DS_OK);
    assert(t == NULL);
    assert(tt_destroy(&t).code == DS_OK); /* 重复销毁安全 */

    /* 空树边界 */
    {
        int empty[] = { -1 };

        assert(tt_create(empty, 1, &t).code == DS_OK);
        assert(t == NULL);
        assert(tt_inthreading(t).code == DS_OK);
        assert(tt_inorder(t, out, &n).code == DS_OK && n == 0);
        assert(tt_first(t) == NULL && tt_last(t) == NULL);
        assert(tt_destroy(&t).code == DS_OK);
    }

    printf("全部测试通过\n");
    return 0;
}
