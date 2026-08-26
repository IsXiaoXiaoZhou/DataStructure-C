/**
 * @file main.c
 * @brief 静态链表（游标实现）断言测试与演示
 *
 * 覆盖点: 初始化 / 按位插删取 / 空表与越界 / 分量回收复用
 *（删除后再插入应优先复用刚归还的分量）/ 备用链耗尽溢出 /
 * destroy 栈对象复位语义 / clear 等价重新 init
 */

#include <assert.h>
#include <stdio.h>
#include "static_linked_list.h"

int main(void)
{
    StaticLinkedList list;
    int    value = 0;
    size_t i = 0;

    printf("===== 静态链表（游标实现） =====\n");

    /* 用例1: 初始化后应为空表 */
    assert(list_init(&list).code == DS_OK);
    assert(list_length(&list) == 0);

    /* 用例2: 空指针参数应返回 DS_NULL_PTR */
    assert(list_init(NULL).code == DS_NULL_PTR);
    assert(list_insert_at(NULL, 1, 10).code == DS_NULL_PTR);
    assert(list_remove_at(NULL, 1, &value).code == DS_NULL_PTR);
    assert(list_get(NULL, 1, &value).code == DS_NULL_PTR);
    assert(list_get(&list, 1, NULL).code == DS_NULL_PTR);
    assert(list_print(NULL).code == DS_NULL_PTR);

    /* 用例3: 空表上删/取应返回 DS_EMPTY */
    assert(list_remove_at(&list, 1, &value).code == DS_EMPTY);
    assert(list_get(&list, 1, &value).code == DS_EMPTY);

    /* 用例4: 依次插入 1..5，位置顺序即值顺序 */
    for (i = 1; i <= 5; i++) {
        assert(list_insert_at(&list, i, (int)(i * 10)).code == DS_OK);
    }
    assert(list_length(&list) == 5);
    for (i = 1; i <= 5; i++) {
        assert(list_get(&list, i, &value).code == DS_OK);
        assert(value == (int)(i * 10));
    }

    /* 用例5: 中间位置插入与删除（游标链接正确性） */
    assert(list_insert_at(&list, 3, 25).code == DS_OK);   /* 10 20 25 30 40 50 */
    assert(list_get(&list, 3, &value).code == DS_OK && value == 25);
    assert(list_remove_at(&list, 3, &value).code == DS_OK && value == 25);
    assert(list_length(&list) == 5);

    /* 用例6: 头插与尾插（pos 边界） */
    assert(list_insert_at(&list, 1, 5).code == DS_OK);    /* 5 10 20 30 40 50 */
    assert(list_insert_at(&list, 7, 60).code == DS_OK);   /* 5 10 20 30 40 50 60 */
    assert(list_get(&list, 1, &value).code == DS_OK && value == 5);
    assert(list_get(&list, 7, &value).code == DS_OK && value == 60);
    assert(list_remove_at(&list, 1, &value).code == DS_OK && value == 5);
    assert(list_remove_at(&list, 6, &value).code == DS_OK && value == 60);
    assert(list_length(&list) == 5);

    /* 用例7: 非法位置应返回 DS_OUT_OF_RANGE */
    assert(list_insert_at(&list, 0, 99).code == DS_OUT_OF_RANGE);
    assert(list_insert_at(&list, 7, 99).code == DS_OUT_OF_RANGE);
    assert(list_remove_at(&list, 0, &value).code == DS_OUT_OF_RANGE);
    assert(list_remove_at(&list, 6, &value).code == DS_OUT_OF_RANGE);
    assert(list_get(&list, 6, &value).code == DS_OUT_OF_RANGE);

    /* 用例8: 删除的分量应被回收复用 —— 先删两个再插两个，长度恢复 */
    assert(list_remove_at(&list, 2, &value).code == DS_OK && value == 20);
    assert(list_remove_at(&list, 2, &value).code == DS_OK && value == 30);
    assert(list_length(&list) == 3);
    assert(list_insert_at(&list, 2, 200).code == DS_OK);
    assert(list_insert_at(&list, 3, 300).code == DS_OK);
    assert(list_length(&list) == 5);
    assert(list_get(&list, 1, &value).code == DS_OK && value == 10);
    assert(list_get(&list, 2, &value).code == DS_OK && value == 200);
    assert(list_get(&list, 3, &value).code == DS_OK && value == 300);
    assert(list_get(&list, 4, &value).code == DS_OK && value == 40);
    assert(list_get(&list, 5, &value).code == DS_OK && value == 50);

    /* 演示: 打印游标链真实布局（下标即"模拟地址"） */
    printf("当前数据链（分量下标展示游标模拟效果）:\n");
    assert(list_print(&list).code == DS_OK);

    /* 用例9: 填满全部可用分量后插入应返回 DS_OVERFLOW
     * 可用数据分量 = MAX-2 个（space[0] 与 space[MAX-1] 被两个链头占用） */
    list_init(&list);
    for (i = 0; i < STATICLIST_MAX_SIZE - 2; i++) {
        assert(list_insert_at(&list, i + 1, (int)(i + 1)).code == DS_OK);
    }
    assert(list_length(&list) == (size_t)(STATICLIST_MAX_SIZE - 2));
    assert(list_insert_at(&list, 1, 0).code == DS_OVERFLOW);
    assert(list_get(&list, 1, &value).code == DS_OK && value == 1);

    /* 用例10: 满表删除 1 个后又能插入（回收复用路径） */
    assert(list_remove_at(&list, 1, &value).code == DS_OK && value == 1);
    assert(list_insert_at(&list, 1, 100).code == DS_OK);
    assert(list_get(&list, 1, &value).code == DS_OK && value == 100);

    /* 用例11: 逐个删空后表应回到空状态 */
    while (list_length(&list) > 0) {
        assert(list_remove_at(&list, 1, &value).code == DS_OK);
    }
    assert(list_get(&list, 1, &value).code == DS_EMPTY);
    assert(list_remove_at(&list, 1, &value).code == DS_EMPTY);

    /* 用例12: destroy 为栈对象复位语义 —— 无堆资源，销毁即重置为初始态 */
    assert(list_insert_at(&list, 1, 55).code == DS_OK);   /* 先放一个元素 */
    list_destroy(&list);
    assert(list_length(&list) == 0);                 /* 复位后回到空表 */
    assert(list_insert_at(&list, 1, 66).code == DS_OK);   /* 复位后可直接复用 */
    assert(list_get(&list, 1, &value).code == DS_OK && value == 66);
    list_destroy(NULL);                              /* NULL 入参安全 */

    /* 用例13: clear 重置备用链与数据链，等价重新 init，清空后可继续插入 */
    assert(list_clear(&list).code == DS_OK);
    assert(list_length(&list) == 0);
    assert(list_clear(NULL).code == DS_NULL_PTR);
    assert(list_insert_at(&list, 1, 77).code == DS_OK);
    assert(list_get(&list, 1, &value).code == DS_OK && value == 77);

    /* 演示: 重建常规表展示最终状态 */
    assert(list_init(&list).code == DS_OK);
    for (i = 1; i <= 4; i++) {
        assert(list_insert_at(&list, i, (int)(i * 111)).code == DS_OK);
    }
    printf("重建表演示:\n");
    assert(list_print(&list).code == DS_OK);

    printf("全部测试通过\n");
    return 0;
}
