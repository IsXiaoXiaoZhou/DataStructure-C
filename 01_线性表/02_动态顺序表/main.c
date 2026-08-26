/**
 * @file main.c
 * @brief 动态顺序表断言测试与演示
 *
 * 覆盖点: 创建销毁 / 增删改查 / 边界位置 / 倍增扩容（SPEC 必测项:
 * 初始容量 4 插入 10 个元素）/ 查找命中与未命中
 */

#include <assert.h>
#include <stdio.h>
#include "dynamic_seq_list.h"

int main(void)
{
    SeqList *list = NULL;
    int      value = 0;
    size_t   pos = 0;
    size_t   i = 0;

    printf("===== 动态顺序表（可扩容数组实现） =====\n");

    /* 用例1: 创建后应为空表、容量等于构造参数 */
    assert(seqlist_init(&list, 4).code == DS_OK);
    assert(list != NULL);
    assert(seqlist_length(list) == 0);
    assert(seqlist_capacity(list) == 4);
    assert(seqlist_is_empty(list) == 1);

    /* 用例2: 空指针参数应返回 DS_NULL_PTR */
    assert(seqlist_init(NULL, 4).code == DS_NULL_PTR);
    assert(seqlist_insert(NULL, 1, 10).code == DS_NULL_PTR);
    assert(seqlist_get(list, 1, NULL).code == DS_NULL_PTR);
    assert(seqlist_find(NULL, 1, &pos).code == DS_NULL_PTR);

    /* 用例3: 空表上取/改/删应返回 DS_EMPTY */
    assert(seqlist_get(list, 1, &value).code == DS_EMPTY);
    assert(seqlist_set(list, 1, 10).code == DS_EMPTY);
    assert(seqlist_delete(list, 1, &value).code == DS_EMPTY);

    /* 用例4: SPEC 必测 —— 初始容量 4，插入 10 个元素验证倍增扩容
     * 第 5 个元素插入时 4→8；第 9 个插入时 8→16，容量只增不减 */
    for (i = 0; i < 10; i++) {
        assert(seqlist_insert(list, i + 1, (int)((i + 1) * 11)).code == DS_OK);
    }
    assert(seqlist_length(list) == 10);
    assert(seqlist_capacity(list) == 16);

    /* 用例5: 扩容后原有数据必须完好（realloc 搬迁正确性） */
    for (i = 0; i < 10; i++) {
        assert(seqlist_get(list, i + 1, &value).code == DS_OK);
        assert(value == (int)((i + 1) * 11));
    }

    /* 用例6: 头部插入使后方元素整体后移 */
    assert(seqlist_insert(list, 1, 7).code == DS_OK);   /* 7 11 22 ... 110 */
    assert(seqlist_get(list, 1, &value).code == DS_OK && value == 7);
    assert(seqlist_get(list, 2, &value).code == DS_OK && value == 11);

    /* 用例7: 非法位置插入应返回 DS_OUT_OF_RANGE（当前 length=11，上限 12） */
    assert(seqlist_insert(list, 0, 99).code == DS_OUT_OF_RANGE);
    assert(seqlist_insert(list, 13, 99).code == DS_OUT_OF_RANGE);

    /* 用例8: 删除首元素并验证前移 */
    assert(seqlist_delete(list, 1, &value).code == DS_OK && value == 7);
    assert(seqlist_get(list, 1, &value).code == DS_OK && value == 11);
    assert(seqlist_delete(list, 0, &value).code == DS_OUT_OF_RANGE);
    assert(seqlist_delete(list, 11, &value).code == DS_OUT_OF_RANGE);

    /* 用例9: set 修改后 get 应取到新值 */
    assert(seqlist_set(list, 3, 888).code == DS_OK);
    assert(seqlist_get(list, 3, &value).code == DS_OK && value == 888);
    assert(seqlist_set(list, 99, 1).code == DS_OUT_OF_RANGE);

    /* 用例10: find 命中返回位置，未命中返回 DS_NOT_FOUND 且 pos 置 0 */
    assert(seqlist_find(list, 888, &pos).code == DS_OK && pos == 3);
    assert(seqlist_find(list, 110, &pos).code == DS_OK && pos == 10);
    assert(seqlist_find(list, 4321, &pos).code == DS_NOT_FOUND && pos == 0);

    /* 演示: 打印扩容后的表状态 */
    printf("初始容量 4，插入 10 个元素后:\n");
    assert(seqlist_print(list).code == DS_OK);

    /* 用例11: 销毁后指针置 NULL，重复销毁安全 */
    seqlist_destroy(&list);
    assert(list == NULL);
    seqlist_destroy(&list);   /* 再次销毁应为无害空操作 */
    seqlist_destroy(NULL);    /* NULL 入参也应安全 */

    /* 用例12: 构造参数传 0 时取默认容量 8 */
    assert(seqlist_init(&list, 0).code == DS_OK);
    assert(seqlist_capacity(list) == 8);
    seqlist_destroy(&list);

    printf("全部测试通过\n");
    return 0;
}
