/**
 * @file main.c
 * @brief 静态顺序表断言测试与演示
 *
 * 覆盖点: 初始化 / 增删改查 / 边界位置 / 满表溢出 / 查找命中与未命中
 */

#include <assert.h>
#include <stdio.h>
#include "static_seq_list.h"

int main(void)
{
    SeqList list;
    int     value = 0;
    size_t  pos = 0;
    size_t  i = 0;

    printf("===== 静态顺序表（定长数组实现） =====\n");

    /* 用例1: 初始化后应为空表、长度为 0 */
    assert(seqlist_init(&list).code == DS_OK);
    assert(seqlist_length(&list) == 0);
    assert(seqlist_is_empty(&list) == 1);

    /* 用例2: 空指针参数应返回 DS_NULL_PTR */
    assert(seqlist_init(NULL).code == DS_NULL_PTR);
    assert(seqlist_get(&list, 1, NULL).code == DS_NULL_PTR);
    assert(seqlist_find(NULL, 1, &pos).code == DS_NULL_PTR);

    /* 用例3: 空表上取/改/删应返回 DS_EMPTY */
    assert(seqlist_get(&list, 1, &value).code == DS_EMPTY);
    assert(seqlist_set(&list, 1, 10).code == DS_EMPTY);
    assert(seqlist_delete(&list, 1, &value).code == DS_EMPTY);

    /* 用例4: 尾插 3 个元素后长度应为 3，且顺序保持 */
    assert(seqlist_insert(&list, 1, 10).code == DS_OK);
    assert(seqlist_insert(&list, 2, 20).code == DS_OK);
    assert(seqlist_insert(&list, 3, 30).code == DS_OK);
    assert(seqlist_length(&list) == 3);
    assert(seqlist_is_empty(&list) == 0);

    /* 用例5: 头部与中间插入应使后方元素整体后移 */
    assert(seqlist_insert(&list, 1, 5).code == DS_OK);    /* 5 10 20 30 */
    assert(seqlist_insert(&list, 3, 15).code == DS_OK);   /* 5 10 15 20 30 */
    assert(seqlist_get(&list, 1, &value).code == DS_OK && value == 5);
    assert(seqlist_get(&list, 3, &value).code == DS_OK && value == 15);
    assert(seqlist_get(&list, 5, &value).code == DS_OK && value == 30);

    /* 用例6: 非法位置插入应返回 DS_OUT_OF_RANGE */
    assert(seqlist_insert(&list, 0, 99).code == DS_OUT_OF_RANGE);
    assert(seqlist_insert(&list, 7, 99).code == DS_OUT_OF_RANGE);
    assert(seqlist_length(&list) == 5);

    /* 用例7: 删除首元素，后方元素整体前移 */
    assert(seqlist_delete(&list, 1, &value).code == DS_OK && value == 5);
    assert(seqlist_get(&list, 1, &value).code == DS_OK && value == 10);
    assert(seqlist_length(&list) == 4);

    /* 用例8: 删除非法位置应返回 DS_OUT_OF_RANGE */
    assert(seqlist_delete(&list, 0, &value).code == DS_OUT_OF_RANGE);
    assert(seqlist_delete(&list, 5, &value).code == DS_OUT_OF_RANGE);

    /* 用例9: set 修改后 get 应取到新值 */
    assert(seqlist_set(&list, 2, 999).code == DS_OK);
    assert(seqlist_get(&list, 2, &value).code == DS_OK && value == 999);
    assert(seqlist_set(&list, 9, 1).code == DS_OUT_OF_RANGE);

    /* 用例10: find 命中返回位置 1 起，未命中返回 DS_NOT_FOUND 且 pos 置 0 */
    assert(seqlist_find(&list, 999, &pos).code == DS_OK && pos == 2);
    assert(seqlist_find(&list, 10, &pos).code == DS_OK && pos == 1);
    assert(seqlist_find(&list, 12345, &pos).code == DS_NOT_FOUND && pos == 0);

    /* 用例11: 填满 100 个元素后，插入应返回 DS_OVERFLOW */
    seqlist_init(&list);
    for (i = 0; i < SEQLIST_MAX_SIZE; i++) {
        assert(seqlist_insert(&list, i + 1, (int)(i + 1)).code == DS_OK);
    }
    assert(seqlist_length(&list) == SEQLIST_MAX_SIZE);
    assert(seqlist_insert(&list, 1, 0).code == DS_OVERFLOW);
    assert(seqlist_insert(&list, SEQLIST_MAX_SIZE + 1, 0).code == DS_OVERFLOW);

    /* 用例12: 满表状态下 get 边界值仍正确 */
    assert(seqlist_get(&list, 1, &value).code == DS_OK && value == 1);
    assert(seqlist_get(&list, SEQLIST_MAX_SIZE, &value).code == DS_OK &&
           value == (int)SEQLIST_MAX_SIZE);

    /* 演示 */
    printf("满表插入溢出测试通过，满表内容:\n");
    assert(seqlist_print(&list).code == DS_OK);

    seqlist_init(&list);
    for (i = 0; i < 8; i++) {
        assert(seqlist_insert(&list, i + 1, (int)((i + 1) * 10)).code == DS_OK);
    }
    printf("常规表演示:\n");
    assert(seqlist_print(&list).code == DS_OK);

    printf("全部测试通过\n");
    return 0;
}
