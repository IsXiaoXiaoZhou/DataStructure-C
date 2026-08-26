/**
 * @file main.c
 * @brief 双链表（带头结点）断言测试与演示
 *
 * 覆盖点: 创建销毁 / 头尾插（tail 不变量）/ 按位插删 /
 * 删空后 tail 回退 / prior/next 相邻元素（prev/next 一步跳）/
 * 反转 / 正逆双向打印 / 查找 / clear 清空复用 / traverse 回调遍历
 */

#include <assert.h>
#include <stdio.h>
#include "doubly_linked_list.h"

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
    DoublyLinkedList *list = NULL;
    int    value = 0;
    size_t pos = 0;
    size_t i = 0;

    printf("===== 双链表（带头结点） =====\n");

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
    assert(list_print_reverse(NULL).code == DS_NULL_PTR);

    /* 用例3: 空表上删/取应返回 DS_EMPTY */
    assert(list_pop_front(list, &value).code == DS_EMPTY);
    assert(list_remove_at(list, 1, &value).code == DS_EMPTY);
    assert(list_get(list, 1, &value).code == DS_EMPTY);

    /* 用例4: 尾插 3 个（tail O(1) 路径）后顺序不变 */
    assert(list_push_back(list, 10).code == DS_OK);
    assert(list_push_back(list, 20).code == DS_OK);
    assert(list_push_back(list, 30).code == DS_OK);
    assert(list_length(list) == 3);
    assert(list_get(list, 1, &value).code == DS_OK && value == 10);
    assert(list_get(list, 3, &value).code == DS_OK && value == 30);

    /* 用例5: 头插应成为新首元，且原首元 prev 正确回接 */
    assert(list_push_front(list, 5).code == DS_OK);    /* 5 10 20 30 */
    assert(list_get(list, 1, &value).code == DS_OK && value == 5);
    assert(list_get(list, 2, &value).code == DS_OK && value == 10);

    /* 用例6: 中间位置插入（双指针四向接链）与删除 */
    assert(list_insert_at(list, 3, 15).code == DS_OK); /* 5 10 15 20 30 */
    assert(list_get(list, 3, &value).code == DS_OK && value == 15);
    assert(list_remove_at(list, 3, &value).code == DS_OK && value == 15);

    /* 用例7: 尾部插入走 tail 快路径，删尾结点 tail 应回退 */
    assert(list_insert_at(list, 5, 40).code == DS_OK); /* 5 10 20 30 40 */
    assert(list_get(list, 5, &value).code == DS_OK && value == 40);
    assert(list_remove_at(list, 5, &value).code == DS_OK && value == 40);
    assert(list_remove_at(list, 4, &value).code == DS_OK && value == 30);
    assert(list_length(list) == 3);

    /* 用例8: 非法位置应返回 DS_OUT_OF_RANGE */
    assert(list_insert_at(list, 0, 99).code == DS_OUT_OF_RANGE);
    assert(list_insert_at(list, 5, 99).code == DS_OUT_OF_RANGE);
    assert(list_remove_at(list, 0, &value).code == DS_OUT_OF_RANGE);
    assert(list_remove_at(list, 4, &value).code == DS_OUT_OF_RANGE);

    /* 用例9: 逆序遍历链路 —— print_reverse 走 prev 链应倒序输出 */
    printf("正序与逆序打印（验证 prev 链路）:\n");
    assert(list_print(list).code == DS_OK);
    assert(list_print_reverse(list).code == DS_OK);

    /* 用例10: 反转 1..6 应得 6..1，且正逆两向链路同时正确 */
    list_destroy(&list);
    assert(list_init(&list).code == DS_OK);
    for (i = 1; i <= 6; i++) {
        assert(list_push_back(list, (int)i).code == DS_OK);
    }
    assert(list_reverse(list).code == DS_OK);
    for (i = 1; i <= 6; i++) {
        assert(list_get(list, i, &value).code == DS_OK);
        assert(value == (int)(7 - i));
    }
    printf("反转后:\n");
    assert(list_print(list).code == DS_OK);
    assert(list_print_reverse(list).code == DS_OK);

    /* 用例11: 删空后 tail 应回退到头结点，再尾插链路仍正确 */
    while (list_length(list) > 0) {
        assert(list_remove_at(list, list_length(list), &value).code == DS_OK);
    }
    assert(list_is_empty(list) == 1);
    assert(list_push_back(list, 77).code == DS_OK);
    assert(list_get(list, 1, &value).code == DS_OK && value == 77);
    assert(list_pop_front(list, &value).code == DS_OK && value == 77);
    assert(list_is_empty(list) == 1);

    /* 用例12: find 命中返回位置，未命中返回 DS_NOT_FOUND 且 pos 置 0 */
    for (i = 1; i <= 3; i++) {
        assert(list_push_back(list, (int)(i * 100)).code == DS_OK);
    }
    assert(list_find(list, 200, &pos).code == DS_OK && pos == 2);
    assert(list_find(list, 301, &pos).code == DS_NOT_FOUND && pos == 0);

    /* 演示: 最终状态打印 */
    assert(list_print(list).code == DS_OK);

    /* 用例13: prior_elem / next_elem —— 借 prev/next 一步取相邻元素
     * （当前表为 100 200 300） */
    assert(list_prior_elem(list, 2, &value).code == DS_OK && value == 100);
    assert(list_prior_elem(list, 3, &value).code == DS_OK && value == 200);
    assert(list_next_elem(list, 1, &value).code == DS_OK && value == 200);
    assert(list_next_elem(list, 2, &value).code == DS_OK && value == 300);
    assert(list_prior_elem(list, 1, &value).code == DS_OUT_OF_RANGE);  /* 首元无前驱 */
    assert(list_next_elem(list, 3, &value).code == DS_OUT_OF_RANGE);   /* 尾元无后继 */
    assert(list_prior_elem(list, 0, &value).code == DS_OUT_OF_RANGE);
    assert(list_prior_elem(list, 4, &value).code == DS_OUT_OF_RANGE);
    assert(list_prior_elem(NULL, 2, &value).code == DS_NULL_PTR);
    assert(list_next_elem(list, 2, NULL).code == DS_NULL_PTR);

    /* 用例14: traverse 回调遍历 —— 计数回调验证访问次数与顺序 */
    visit_count = 0;
    assert(list_traverse(list, visit_record).code == DS_OK);
    assert(visit_count == 3);
    assert(visit_seq[0] == 100 && visit_seq[1] == 200 &&
           visit_seq[2] == 300);
    assert(list_traverse(list, NULL).code == DS_NULL_PTR);
    assert(list_traverse(NULL, visit_record).code == DS_NULL_PTR);

    /* 用例15: clear 释放全部数据结点保留表壳（tail 复位指回头结点） */
    assert(list_clear(list).code == DS_OK);
    assert(list_length(list) == 0);
    assert(list_is_empty(list) == 1);
    visit_count = 0;
    assert(list_traverse(list, visit_record).code == DS_OK);   /* 空表访问 0 次 */
    assert(visit_count == 0);
    assert(list_clear(NULL).code == DS_NULL_PTR);
    /* 清空后空表取相邻元素返回 DS_EMPTY */
    assert(list_prior_elem(list, 1, &value).code == DS_EMPTY);
    assert(list_next_elem(list, 1, &value).code == DS_EMPTY);
    /* clear 后可继续插入 —— 验证 tail 已正确复位指回头结点 */
    assert(list_push_back(list, 66).code == DS_OK);
    assert(list_push_front(list, 55).code == DS_OK);
    assert(list_get(list, 1, &value).code == DS_OK && value == 55);
    assert(list_get(list, 2, &value).code == DS_OK && value == 66);

    /* 用例16: 销毁后指针置 NULL，重复销毁安全 */
    list_destroy(&list);
    assert(list == NULL);
    list_destroy(&list);
    list_destroy(NULL);

    printf("全部测试通过\n");
    return 0;
}
