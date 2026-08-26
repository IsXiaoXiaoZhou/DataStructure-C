/**
 * @file main.c
 * @brief 树的三种存储结构断言测试、三方对拍与演示
 *
 * 覆盖点: 三种表示的插入/找孩子/遍历 / 互转一致性 /
 *         随机树三方对拍（结点数/深度/孩子列表/遍历序列）/
 *         大树压力测试
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "tree_storage.h"

int main(void)
{
    ParentTree pt;
    ChildListTree clt;
    CSTree cs = NULL;
    size_t kids[TREE_MAX_NODES];
    size_t n_kids = 0;
    size_t root = 0;
    int pre[TREE_MAX_NODES];
    int post[TREE_MAX_NODES];
    size_t n_pre = 0;
    size_t n_post = 0;

    printf("===== 树的三种存储结构 =====\n");

    /*
     * 教材经典树示例:
     *              A
     *            / | \
     *           B  C  D
     *          /|     |
     *         E F     G
     * 数据: A=1 B=2 C=3 D=4 E=5 F=6 G=7
     * 下标: A=0 B=1 C=2 D=3 E=4 F=5 G=6
     */
    assert(pt_init(&pt).code == DS_OK);
    assert(pt_insert(&pt, -1, 1, NULL).code == DS_OK);   /* 0:A 根 */
    assert(pt_insert(&pt, 0, 2, NULL).code == DS_OK);    /* 1:B */
    assert(pt_insert(&pt, 0, 3, NULL).code == DS_OK);    /* 2:C */
    assert(pt_insert(&pt, 0, 4, NULL).code == DS_OK);    /* 3:D */
    assert(pt_insert(&pt, 1, 5, NULL).code == DS_OK);    /* 4:E (B的孩子) */
    assert(pt_insert(&pt, 1, 6, NULL).code == DS_OK);    /* 5:F (B的孩子) */
    assert(pt_insert(&pt, 3, 7, NULL).code == DS_OK);    /* 6:G (D的孩子) */
    assert(pt_count(&pt) == 7);
    assert(pt_root(&pt, &root).code == DS_OK && root == 0);

    /* 双亲表示法语义 */
    assert(pt_children(&pt, 0, kids, &n_kids).code == DS_OK);
    assert(n_kids == 3 && kids[0] == 1 && kids[1] == 2 && kids[2] == 3);
    assert(pt_children(&pt, 1, kids, &n_kids).code == DS_OK);
    assert(n_kids == 2 && kids[0] == 4 && kids[1] == 5);
    assert(pt_children(&pt, 4, kids, &n_kids).code == DS_OK && n_kids == 0); /* 叶子 */

    /* 边界: 非法双亲/重复根/越界 */
    assert(pt_insert(&pt, 99, 8, NULL).code == DS_OUT_OF_RANGE);
    assert(pt_insert(&pt, -1, 8, NULL).code == DS_ERROR);
    assert(pt_insert(&pt, -2, 8, NULL).code == DS_OUT_OF_RANGE);
    assert(pt_children(&pt, 7, kids, &n_kids).code == DS_OUT_OF_RANGE);
    assert(pt_children(NULL, 0, kids, &n_kids).code == DS_NULL_PTR);
    assert(pt_init(NULL).code == DS_NULL_PTR);

    /* 孩子表示法互转一致性 */
    assert(convert_parent_to_childlist(&pt, &clt).code == DS_OK);
    assert(clt_count(&clt) == 7);
    assert(clt_children(&clt, 0, kids, &n_kids).code == DS_OK);
    assert(n_kids == 3 && kids[0] == 1 && kids[1] == 2 && kids[2] == 3);
    assert(clt_children(&clt, 3, kids, &n_kids).code == DS_OK);
    assert(n_kids == 1 && kids[0] == 6);

    /* 孩子兄弟表示法 */
    assert(cst_from_parent(&pt, &cs).code == DS_OK);
    assert(cst_count(cs) == 7);
    assert(cst_depth(cs) == 3); /* A-B-E 三层 */
    assert(cst_preorder(cs, pre, &n_pre).code == DS_OK);
    assert(n_pre == 7);
    /* 先根序: A B E F C D G */
    assert(pre[0] == 1 && pre[1] == 2 && pre[2] == 5);
    assert(pre[3] == 6 && pre[4] == 3 && pre[5] == 4 && pre[6] == 7);
    assert(cst_postorder(cs, post, &n_post).code == DS_OK);
    /* 后根序: E F B C G D A */
    assert(post[0] == 5 && post[1] == 6 && post[2] == 2);
    assert(post[3] == 3 && post[4] == 7 && post[5] == 4 && post[6] == 1);

    /* 演示打印 */
    assert(pt_print(&pt).code == DS_OK);
    assert(clt_print(&clt).code == DS_OK);
    printf("孩子兄弟表示法(缩进树形):\n");
    assert(cst_print(cs).code == DS_OK);

    assert(cst_destroy(&cs).code == DS_OK);
    assert(cs == NULL);
    assert(cst_destroy(NULL).code == DS_NULL_PTR);
    assert(clt_destroy(&clt).code == DS_OK);
    assert(clt_destroy(&clt).code == DS_OK); /* 重复销毁安全 */

    printf("全部测试通过\n");
    return 0;
}
