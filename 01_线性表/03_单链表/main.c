/**
 * @file main.c
 * @brief 单链表（带头结点）断言测试与演示
 *
 * 覆盖点: 创建销毁 / 头尾插 / 按位插删 / 按位取值 / 查找 /
 * 三指针反转 / clear 清空复用 / traverse 回调遍历 / 边界与异常返回
 */

#include <assert.h>
#include <stdio.h>
#include "singly_linked_list.h"

/* traverse 测试用计数回调：记录访问次数与先后顺序 */
static int visit_count = 0;
static int visit_seq[16] = {0};

static void visit_record(int value)
{
    if (visit_count < 16) {
        visit_seq[visit_count] = value;
    }
    visit_count++;
}

int main(void)
{
    SinglyLinkedList *list = NULL;
    int   value = 0;
    size_t pos = 0;
    size_t i = 0;

    printf("===== 单链表（带头结点） =====\n");

    /* 用例1: 创建后应为空表 */
    assert(list_init(&list).code == DS_OK);
    assert(list != NULL);
    assert(list_length(list) == 0);
    assert(list_is_empty(list) == 1);

    /* 用例2: 空指针参数应返回 DS_NULL_PTR */
    assert(list_init(NULL).code == DS_NULL_PTR);
    assert(list_push_back(NULL, 1).code == DS_NULL_PTR);
    assert(list_get(NULL, 1, &value).code == DS_NULL_PTR);
    assert(list_find(list, 1, NULL).code == DS_NULL_PTR);
    assert(list_reverse(NULL).code == DS_NULL_PTR);

    /* 用例3: 空表上删/取应返回 DS_EMPTY */
    assert(list_pop_front(list, &value).code == DS_EMPTY);
    assert(list_remove_at(list, 1, &value).code == DS_EMPTY);
    assert(list_get(list, 1, &value).code == DS_EMPTY);

    /* 用例4: 尾插 3 个元素后长度应为 3 且顺序不变 */
    assert(list_push_back(list, 10).code == DS_OK);
    assert(list_push_back(list, 20).code == DS_OK);
    assert(list_push_back(list, 30).code == DS_OK);
    assert(list_length(list) == 3);
    assert(list_is_empty(list) == 0);
    assert(list_get(list, 1, &value).code == DS_OK && value == 10);
    assert(list_get(list, 3, &value).code == DS_OK && value == 30);

    /* 用例5: 头插应成为新首元 */
    assert(list_push_front(list, 5).code == DS_OK);   /* 5 10 20 30 */
    assert(list_get(list, 1, &value).code == DS_OK && value == 5);

    /* 用例6: 中间位置插入（第 3 位）与删除 */
    assert(list_insert_at(list, 3, 15).code == DS_OK); /* 5 10 15 20 30 */
    assert(list_get(list, 3, &value).code == DS_OK && value == 15);
    assert(list_remove_at(list, 3, &value).code == DS_OK && value == 15);
    assert(list_length(list) == 4);

    /* 用例7: 非法位置应返回 DS_OUT_OF_RANGE */
    assert(list_insert_at(list, 0, 99).code == DS_OUT_OF_RANGE);
    assert(list_insert_at(list, 6, 99).code == DS_OUT_OF_RANGE);
    assert(list_remove_at(list, 0, &value).code == DS_OUT_OF_RANGE);
    assert(list_remove_at(list, 5, &value).code == DS_OUT_OF_RANGE);
    assert(list_get(list, 5, &value).code == DS_OUT_OF_RANGE);

    /* 用例8: 头删逐个弹出并验证顺序（队列式出队效果） */
    assert(list_pop_front(list, &value).code == DS_OK && value == 5);
    assert(list_pop_front(list, &value).code == DS_OK && value == 10);
    assert(list_pop_front(list, &value).code == DS_OK && value == 20);
    assert(list_pop_front(list, &value).code == DS_OK && value == 30);
    assert(list_is_empty(list) == 1);

    /* 用例9: 三指针反转 —— 构造 1..6 后反转应为 6..1 */
    for (i = 1; i <= 6; i++) {
        assert(list_push_back(list, (int)i).code == DS_OK);
    }
    assert(list_reverse(list).code == DS_OK);
    for (i = 1; i <= 6; i++) {
        assert(list_get(list, i, &value).code == DS_OK);
        assert(value == (int)(7 - i));
    }
    assert(list_length(list) == 6);

    /* 用例10: 单元素与空表反转应保持不变 */
    list_destroy(&list);
    assert(list_init(&list).code == DS_OK);
    assert(list_reverse(list).code == DS_OK);          /* 空表反转 */
    assert(list_is_empty(list) == 1);
    assert(list_push_back(list, 42).code == DS_OK);
    assert(list_reverse(list).code == DS_OK);          /* 单元素反转 */
    assert(list_get(list, 1, &value).code == DS_OK && value == 42);

    /* 用例11: find 命中返回位置，未命中返回 DS_NOT_FOUND 且 pos 置 0 */
    assert(list_push_back(list, 43).code == DS_OK);
    assert(list_push_back(list, 44).code == DS_OK);
    assert(list_find(list, 44, &pos).code == DS_OK && pos == 3);
    assert(list_find(list, 42, &pos).code == DS_OK && pos == 1);
    assert(list_find(list, 999, &pos).code == DS_NOT_FOUND && pos == 0);

    /* 演示: 打印反转前与反转后 */
    printf("反转前: ");
    assert(list_print(list).code == DS_OK);
    assert(list_reverse(list).code == DS_OK);
    printf("反转后: ");
    assert(list_print(list).code == DS_OK);

    /* 用例13: traverse 回调遍历 —— 计数回调验证访问次数与顺序
     * （当前表为 44 43 42） */
    visit_count = 0;
    assert(list_traverse(list, visit_record).code == DS_OK);
    assert(visit_count == 3);
    assert(visit_seq[0] == 44 && visit_seq[1] == 43 && visit_seq[2] == 42);
    assert(list_traverse(list, NULL).code == DS_NULL_PTR);
    assert(list_traverse(NULL, visit_record).code == DS_NULL_PTR);

    /* 用例14: clear 释放全部数据结点保留表壳，清空后可继续插入 */
    assert(list_clear(list).code == DS_OK);
    assert(list_length(list) == 0);
    assert(list_is_empty(list) == 1);
    visit_count = 0;
    assert(list_traverse(list, visit_record).code == DS_OK);   /* 空表访问 0 次 */
    assert(visit_count == 0);
    assert(list_clear(NULL).code == DS_NULL_PTR);
    assert(list_push_back(list, 66).code == DS_OK);            /* 表壳仍可用 */
    assert(list_get(list, 1, &value).code == DS_OK && value == 66);

    /* 用例15: 销毁后指针置 NULL，重复销毁安全 */
    list_destroy(&list);
    assert(list == NULL);
    list_destroy(&list);
    list_destroy(NULL);

    printf("全部测试通过\n");
    return 0;
}
