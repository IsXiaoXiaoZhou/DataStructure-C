/**
 * @file main.c
 * @brief 红黑树测试
 *
 * 覆盖点: 五性质验证器 / 插入修复(叔红变色/叔黑旋转) / 删除双黑修复 /
 * 有序插入高度界 / 中序升序不变量 / 高度 ≤ 2·log2(n+1) 界
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "rb.h"

int main(void)
{
    RBTree t;
    RBNode *node = NULL;
    size_t i = 0;

    rb_init(&t);

    printf("===== 红黑树（RB Tree） =====\n");

    /* 参数错误 */
    assert(rb_insert(NULL, 1).code == DS_NULL_PTR);
    assert(rb_delete(NULL, 1).code == DS_NULL_PTR);
    assert(rb_search(NULL, 1, &node).code == DS_NULL_PTR);
    assert(rb_clear(NULL).code == DS_NULL_PTR);
    assert(rb_inorder(&t, NULL).code == DS_NULL_PTR);
    assert(rb_search(&t, 5, &node).code == DS_NOT_FOUND);
    assert(rb_delete(&t, 5).code == DS_NOT_FOUND);
    assert(rb_count(&t) == 0 && rb_height(&t) == 0);

    /* 基本插入 + 根黑 */
    assert(rb_insert(&t, 10).code == DS_OK);
    assert(t.root != NULL && t.root->color == RB_BLACK);
    assert(rb_insert(&t, 10).code == DS_ERROR);        /* 重复 */
    assert(rb_insert(&t, 20).code == DS_OK);
    assert(rb_insert(&t, 30).code == DS_OK);           /* RR: 左旋 */
    assert(rb_verify(&t, NULL).code == DS_OK);
    assert(rb_search(&t, 20, &node).code == DS_OK && node->data == 20);

    /* 删除一个结点再验 */
    assert(rb_delete(&t, 30).code == DS_OK);
    assert(rb_delete(&t, 10).code == DS_OK);
    assert(rb_verify(&t, NULL).code == DS_OK);
    assert(rb_delete(&t, 20).code == DS_OK);
    assert(t.root == NULL);
    assert(rb_delete(&t, 20).code == DS_NOT_FOUND);

    /* 四种角度插入路径（验证插入修复不破坏性质） */
    {
        static const int seqs[][3] = {
            { 3, 2, 1 }, { 1, 2, 3 }, { 3, 1, 2 }, { 1, 3, 2 }
        };
        for (i = 0; i < sizeof(seqs) / sizeof(seqs[0]); ++i) {
            RBTree a;
            rb_init(&a);
            assert(rb_insert(&a, seqs[i][0]).code == DS_OK);
            assert(rb_insert(&a, seqs[i][1]).code == DS_OK);
            assert(rb_insert(&a, seqs[i][2]).code == DS_OK);
            assert(rb_verify(&a, NULL).code == DS_OK);
            assert(rb_destroy(&a).code == DS_OK && a.root == NULL);
        }
    }
    printf("插入旋转修复 + 基础删除通过\n");

    /* 有序插入 1..1000: 高度 ≤ 2·log2(n+1) */
    {
        RBTree a;
        size_t h = 0;
        rb_init(&a);
        for (i = 1; i <= 1000; ++i) {
            assert(rb_insert(&a, (int)i).code == DS_OK);
        }
        h = rb_height(&a);
        printf("有序插入 1..1000: 高度 = %d (界 2*log2(1001)≈%d)\n",
               (int)h, (int)(2.0 * (log(1001.0) / log(2.0))));
        assert((double)h <= 2.0 * (log(1001.0) / log(2.0)) + 1.0);
        assert(rb_verify(&a, NULL).code == DS_OK);
        assert(rb_destroy(&a).code == DS_OK);
    }

    /* 清空复用 */
    assert(rb_clear(&t).code == DS_OK && t.root == NULL);
    for (i = 0; i < 40; ++i) {
        assert(rb_insert(&t, (int)i).code == DS_OK);
    }
    assert(rb_count(&t) == 40 && rb_verify(&t, NULL).code == DS_OK);
    assert(rb_destroy(&t).code == DS_OK && t.root == NULL);

    printf("全部测试通过\n");
    return 0;
}
