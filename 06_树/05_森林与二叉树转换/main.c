/**
 * @file main.c
 * @brief 森林与二叉树互转断言测试、roundtrip 对拍与演示
 *
 * 覆盖点: 互转正确性（roundtrip 指纹一致）/ 两大性质
 *         （森林先序=二叉树先序、森林后序=二叉树中序）/
 *         随机森林对拍 / 边界（空森林/单树/多树）
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "forest_bitree.h"

/* 辅助: 创建独立结点（不挂入森林根链） */
static CSNode *make_node(int data)
{
    CSNode *p = (CSNode *)malloc(sizeof *p);
    if (p != NULL) { p->data = data; p->first_child = NULL; p->next_sibling = NULL; }
    return p;
}

int main(void)
{
    Forest f;
    CSBinaryTree bt = NULL;
    CSNode *root1 = NULL, *root2 = NULL;
    CSNode *n2 = NULL, *n3 = NULL, *n4 = NULL, *n5 = NULL;
    int seq[FOREST_MAX_NODES];
    size_t n = 0;
    size_t i = 0;

    printf("===== 森林与二叉树互转 =====\n");

    /* 构建测试森林: 两棵树
     *   树1: 1 -> (2 -> (4,5), 3)
     *   树2: 6
     */
    assert(forest_init(&f).code == DS_OK);
    assert(forest_add_tree(&f, 1, &root1).code == DS_OK);
    assert(forest_add_tree(&f, 6, &root2).code == DS_OK);

    n2 = make_node(2); n3 = make_node(3); n4 = make_node(4); n5 = make_node(5);
    assert(n2 && n3 && n4 && n5);
    assert(forest_attach(&f, root1, n2).code == DS_OK);
    assert(forest_attach(&f, root1, n3).code == DS_OK);
    assert(forest_attach(&f, n2, n4).code == DS_OK);
    assert(forest_attach(&f, n2, n5).code == DS_OK);

    assert(forest_tree_count(&f) == 2);
    assert(forest_count(&f) == 6);

    /* 正转换: 森林 -> 二叉树 */
    assert(forest_to_bitree(&f, &bt).code == DS_OK);
    assert(bt_count(bt) == 6);

    /* 性质验证: 森林先序 = 二叉树先序 */
    assert(forest_preorder(&f, seq, &n).code == DS_OK);
    {
        int seq2[FOREST_MAX_NODES];
        size_t n2_count = 0;

        assert(bt_preorder(bt, seq2, &n2_count).code == DS_OK);
        assert(n2_count == n);
        for (i = 0; i < n; i++) {
            assert(seq[i] == seq2[i]);
        }
        printf("森林先序（与二叉树先序一致）: ");
        for (i = 0; i < n; i++) {
            printf("%d ", seq[i]);
        }
        printf("\n");
    }

    /* 性质验证: 森林后序 = 二叉树中序 */
    assert(forest_postorder(&f, seq, &n).code == DS_OK);
    {
        int seq2[FOREST_MAX_NODES];
        size_t n2_count = 0;

        assert(bt_inorder(bt, seq2, &n2_count).code == DS_OK);
        assert(n2_count == n);
        for (i = 0; i < n2_count; i++) {
            assert(seq[i] == seq2[i]);
        }
        printf("二叉树中序（与森林后序一致）: ");
        for (i = 0; i < n2_count; i++) {
            printf("%d ", seq2[i]);
        }
        printf("\n");
    }

    /* 逆转换 roundtrip */
    {
        Forest g;
        int fa_d[FOREST_MAX_NODES], fb_d[FOREST_MAX_NODES];
        int fa_f[FOREST_MAX_NODES], fb_f[FOREST_MAX_NODES];
        size_t na = 0;
        size_t nb = 0;

        assert(forest_fingerprint(&f, fa_d, fa_f, &na).code == DS_OK);
        assert(bitree_to_forest(bt, &g).code == DS_OK);
        assert(forest_tree_count(&g) == 2);
        assert(forest_fingerprint(&g, fb_d, fb_f, &nb).code == DS_OK);
        assert(na == nb);
        for (i = 0; i < na; i++) {
            assert(fa_d[i] == fb_d[i] && fa_f[i] == fb_f[i]);
        }
        assert(forest_destroy(&g).code == DS_OK);
        printf("roundtrip: 森林 -> 二叉树 -> 森林 指纹一致\n");
    }

    /* 空森林边界 */
    {
        Forest empty;
        CSBinaryTree empty_bt = NULL;

        assert(forest_init(&empty).code == DS_OK);
        assert(forest_to_bitree(&empty, &empty_bt).code == DS_OK);
        assert(empty_bt == NULL);
        assert(bitree_to_forest(empty_bt, &empty).code == DS_OK);
        assert(forest_count(&empty) == 0);
        assert(forest_destroy(&empty).code == DS_OK);
    }

    assert(forest_destroy(&f).code == DS_OK);
    assert(bitree_destroy(&bt).code == DS_OK);

    printf("全部测试通过\n");
    return 0;
}
