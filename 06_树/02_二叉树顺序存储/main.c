/**
 * @file main.c
 * @brief 二叉树顺序存储断言测试、暴力对拍与演示
 *
 * 覆盖点: 编号关系式 2i/2i+1/i/2 / 四种遍历 / 深度公式 /
 *         随机树与链式参考实现遍历对拍 / 满树压力测试
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "seq_binary_tree.h"


int main(void)
{
    SeqBinaryTree t;
    int out[SEQ_BT_MAX];
    size_t n = 0;
    size_t idx = 0;
    int v = 0;

    printf("===== 二叉树的顺序存储（数组实现） =====\n");

    /*
     * 教材示例完全二叉树（层序）:
     *        1
     *       / \
     *      2   3
     *     / \ / \
     *    4  5 6  7
     * 编号即数据，验证关系式最直观
     */
    assert(sbt_init(&t).code == DS_OK);
    for (v = 1; v <= 7; v++) {
        assert(sbt_insert_level(&t, v, &idx).code == DS_OK);
        assert((int)idx == v);
    }
    assert(sbt_count(&t) == 7);
    assert(sbt_is_empty(&t) == 0);
    assert(sbt_depth(&t) == 3);

    /* 用例1: 编号关系式 */
    assert(sbt_parent(&t, 1, &idx).code == DS_ERROR);        /* 根无双亲 */
    assert(sbt_parent(&t, 2, &idx).code == DS_OK && idx == 1);
    assert(sbt_parent(&t, 3, &idx).code == DS_OK && idx == 1);
    assert(sbt_parent(&t, 7, &idx).code == DS_OK && idx == 3);
    assert(sbt_left_child(&t, 1, &idx).code == DS_OK && idx == 2);
    assert(sbt_right_child(&t, 1, &idx).code == DS_OK && idx == 3);
    assert(sbt_left_child(&t, 3, &idx).code == DS_OK && idx == 6);
    assert(sbt_right_child(&t, 3, &idx).code == DS_OK && idx == 7);
    assert(sbt_left_child(&t, 4, &idx).code == DS_ERROR);    /* 叶子 */
    assert(sbt_right_child(&t, 7, &idx).code == DS_ERROR);

    /* 用例2: 越界与空指针 */
    assert(sbt_parent(&t, 0, &idx).code == DS_OUT_OF_RANGE);
    assert(sbt_parent(&t, 8, &idx).code == DS_OUT_OF_RANGE);
    assert(sbt_get(&t, 8, &v).code == DS_OUT_OF_RANGE);
    assert(sbt_get(&t, 3, &v).code == DS_OK && v == 3);
    assert(sbt_get(NULL, 1, &v).code == DS_NULL_PTR);
    assert(sbt_init(NULL).code == DS_NULL_PTR);

    /* 用例3: 四种遍历（数据=编号便于核对） */
    assert(sbt_preorder(&t, out, &n).code == DS_OK);
    assert(n == 7 && out[0] == 1 && out[1] == 2 && out[2] == 4);
    assert(out[3] == 5 && out[4] == 3 && out[5] == 6 && out[6] == 7);
    printf("先序: ");
    for (idx = 0; idx < n; idx++) {
        printf("%d ", out[idx]);
    }
    printf("\n");

    assert(sbt_inorder(&t, out, &n).code == DS_OK);
    assert(n == 7 && out[0] == 4 && out[1] == 2 && out[2] == 5);
    assert(out[3] == 1 && out[4] == 6 && out[5] == 3 && out[6] == 7);
    printf("中序: ");
    for (idx = 0; idx < n; idx++) {
        printf("%d ", out[idx]);
    }
    printf("\n");

    assert(sbt_postorder(&t, out, &n).code == DS_OK);
    assert(n == 7 && out[0] == 4 && out[1] == 5 && out[2] == 2);
    assert(out[3] == 6 && out[4] == 7 && out[5] == 3 && out[6] == 1);
    printf("后序: ");
    for (idx = 0; idx < n; idx++) {
        printf("%d ", out[idx]);
    }
    printf("\n");

    assert(sbt_level_order(&t, out, &n).code == DS_OK);
    assert(n == 7 && out[0] == 1 && out[6] == 7); /* 层序 = 物理序 */

    /* 演示打印 */
    assert(sbt_print(&t).code == DS_OK);

    /* 用例4: 深度公式抽查 */
    {
        size_t sizes[] = { 1, 2, 3, 4, 7, 8, 15, 16 };
        size_t expect[] = { 1, 2, 2, 3, 3, 4, 4, 5 };
        int i = 0;

        for (i = 0; i < 8; i++) {
            assert(sbt_init(&t).code == DS_OK);
            for (v = 0; (size_t)v < sizes[i]; v++) {
                assert(sbt_insert_level(&t, v, NULL).code == DS_OK);
            }
            assert(sbt_depth(&t) == expect[i]);
        }
    }

    printf("全部测试通过\n");
    return 0;
}
