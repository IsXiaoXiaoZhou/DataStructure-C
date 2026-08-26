/**
 * @file main.c
 * @brief 循环链表（尾指针 rear 表示）断言测试与演示（含约瑟夫环）
 *
 * 覆盖点: 创建销毁 / 头尾插（O(1)）/ 按位插删取 / 单结点自环 /
 * 空表接管 / 两环 O(1) 合并（含自合并防御）/ 查找 / clear+traverse /
 * 约瑟夫环出圈序列
 */

#include <assert.h>
#include <stdio.h>
#include "circular_linked_list.h"

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

/*
 * 约瑟夫环问题求解：n 人编号 1..n 围坐一圈，从 1 号起报数，
 * 报到 m 者出圈，其下一位从 1 重新报数，直至圈空。
 * 出圈顺序写入 out_seq（调用方保证容量不小于 n）。
 *
 * 算法要点（循环链表天然贴合本问题）:
 *   1. 编号 i 依次尾插入环，首元即报数起点
 *   2. "从当前位置 cur 的人起报 1" 等价于在表中数第 m 人:
 *      出圈位置 = ((cur-1) + (m-1)) % 剩余人数 + 1
 *   3. 删除出圈者后，其下一位恰好顶替同一位置，成为下一轮
 *      报数起点 —— 故 cur 直接更新为出圈位置即可
 */
static void josephus_solve(int n, int m, int *out_seq)
{
    CircularLinkedList *ring = NULL;
    int    out_value = 0;
    size_t cur = 1;      /* 下一轮报数起点在当前表中的位置 */
    size_t out_pos = 0;
    int    i = 0;
    int    count = 0;

    assert(list_init(&ring).code == DS_OK);
    for (i = 1; i <= n; i++) {
        assert(list_push_back(ring, i).code == DS_OK);
    }

    while (list_length(ring) > 0) {
        /*
         * cur 为 size_t（无符号）、m 为 int（有符号）：直接混合
         * 相减会触发有符号/无符号混算告警且隐式转换语义不直观；
         * 显式 (size_t)m 表明报数偏移按无符号模运算计算，消除告警
         */
        out_pos = (cur + (size_t)m - 2) % list_length(ring) + 1;
        assert(list_remove_at(ring, out_pos, &out_value).code == DS_OK);
        out_seq[count] = out_value;
        count++;
        cur = out_pos;   /* 出圈者的继任者顶替其位置 */
    }

    list_destroy(&ring);
}

int main(void)
{
    CircularLinkedList *a = NULL;
    CircularLinkedList *b = NULL;
    int    value = 0;
    size_t pos = 0;
    size_t i = 0;
    int    seq[8] = {0};
    const int expect[8] = {3, 6, 1, 5, 2, 8, 4, 7};

    printf("===== 循环链表（尾指针 rear 表示） =====\n");

    /* 用例1: 创建后应为空表 */
    assert(list_init(&a).code == DS_OK);
    assert(list_length(a) == 0);

    /* 用例2: 空指针参数应返回 DS_NULL_PTR */
    assert(list_init(NULL).code == DS_NULL_PTR);
    assert(list_push_back(NULL, 1).code == DS_NULL_PTR);
    assert(list_push_front(NULL, 1).code == DS_NULL_PTR);
    assert(list_get(NULL, 1, &value).code == DS_NULL_PTR);
    assert(list_remove_at(NULL, 1, &value).code == DS_NULL_PTR);
    assert(list_merge(NULL, a).code == DS_NULL_PTR);
    assert(list_merge(a, NULL).code == DS_NULL_PTR);

    /* 用例3: 空表上删/取应返回 DS_EMPTY */
    assert(list_remove_at(a, 1, &value).code == DS_EMPTY);
    assert(list_get(a, 1, &value).code == DS_EMPTY);

    /* 用例4: 空表头插 1 个应自成环，取值正确 */
    assert(list_push_front(a, 10).code == DS_OK);
    assert(list_length(a) == 1);
    assert(list_get(a, 1, &value).code == DS_OK && value == 10);

    /* 用例5: 头插、尾插混合后环序应为 5 10 20 30 */
    assert(list_push_back(a, 20).code == DS_OK);
    assert(list_push_back(a, 30).code == DS_OK);
    assert(list_push_front(a, 5).code == DS_OK);
    assert(list_length(a) == 4);
    assert(list_get(a, 1, &value).code == DS_OK && value == 5);
    assert(list_get(a, 4, &value).code == DS_OK && value == 30);

    /* 用例6: 非法位置应返回 DS_OUT_OF_RANGE */
    assert(list_remove_at(a, 0, &value).code == DS_OUT_OF_RANGE);
    assert(list_remove_at(a, 5, &value).code == DS_OUT_OF_RANGE);
    assert(list_get(a, 5, &value).code == DS_OUT_OF_RANGE);

    /* 用例7: 删除中间、删除单结点（环回空）再重建 */
    assert(list_remove_at(a, 2, &value).code == DS_OK && value == 10);
    assert(list_remove_at(a, 1, &value).code == DS_OK && value == 5);
    assert(list_remove_at(a, 1, &value).code == DS_OK && value == 20);
    assert(list_remove_at(a, 1, &value).code == DS_OK && value == 30);
    assert(list_length(a) == 0);
    assert(list_push_back(a, 99).code == DS_OK);
    assert(list_get(a, 1, &value).code == DS_OK && value == 99);
    list_destroy(&a);

    /* 用例8: 两环 O(1) 合并 —— a=[1..3], b=[4..6] 合并后为 1..6 */
    assert(list_init(&a).code == DS_OK);
    assert(list_init(&b).code == DS_OK);
    for (i = 1; i <= 3; i++) {
        assert(list_push_back(a, (int)i).code == DS_OK);
    }
    for (i = 4; i <= 6; i++) {
        assert(list_push_back(b, (int)i).code == DS_OK);
    }
    assert(list_merge(a, b).code == DS_OK);
    assert(list_length(a) == 6);
    assert(list_length(b) == 0);
    for (i = 1; i <= 6; i++) {
        assert(list_get(a, i, &value).code == DS_OK);
        assert(value == (int)i);
    }
    printf("两环合并后:\n");
    assert(list_print(a).code == DS_OK);

    /* 用例8b: 自合并防御 —— merge(a,a) 返回 DS_ERROR 且 a 完好
     * （自合并会使结点全部泄漏且表被清空，实现已在入口拦截） */
    assert(list_merge(a, a).code == DS_ERROR);
    assert(list_length(a) == 6);
    for (i = 1; i <= 6; i++) {
        assert(list_get(a, i, &value).code == DS_OK);
        assert(value == (int)i);
    }

    /* 用例9: b 空表合并应无副作用；a 空表接管非空 b */
    assert(list_merge(a, b).code == DS_OK);
    assert(list_length(a) == 6);
    list_destroy(&a);
    assert(list_init(&a).code == DS_OK);
    assert(list_push_back(b, 7).code == DS_OK);
    assert(list_merge(a, b).code == DS_OK);
    assert(list_length(a) == 1);
    assert(list_get(a, 1, &value).code == DS_OK && value == 7);
    list_destroy(&a);
    list_destroy(&b);

    /* 用例10: 按位插入（含空表单结点自环）/ 查找 / 判空 */
    assert(list_init(&a).code == DS_OK);
    /* 空表按位插入：唯一结点必须自成环 */
    assert(list_insert_at(a, 1, 50).code == DS_OK);
    assert(list_length(a) == 1);
    assert(list_get(a, 1, &value).code == DS_OK && value == 50);
    /* 头插 / 中插 / 尾插（pos==length+1 走 rear 快路径） */
    assert(list_insert_at(a, 1, 10).code == DS_OK);    /* 10 50 */
    assert(list_insert_at(a, 2, 30).code == DS_OK);    /* 10 30 50 */
    assert(list_insert_at(a, 4, 70).code == DS_OK);    /* 10 30 50 70 */
    assert(list_length(a) == 4);
    assert(list_get(a, 1, &value).code == DS_OK && value == 10);
    assert(list_get(a, 2, &value).code == DS_OK && value == 30);
    assert(list_get(a, 3, &value).code == DS_OK && value == 50);
    assert(list_get(a, 4, &value).code == DS_OK && value == 70);
    /* 非法位置与空指针 */
    assert(list_insert_at(a, 0, 9).code == DS_OUT_OF_RANGE);
    assert(list_insert_at(a, 6, 9).code == DS_OUT_OF_RANGE);
    assert(list_insert_at(NULL, 1, 9).code == DS_NULL_PTR);
    /* find 与顺序表语义一致：命中返回位置，未命中 DS_NOT_FOUND 且 pos 置 0 */
    assert(list_find(a, 30, &pos).code == DS_OK && pos == 2);
    assert(list_find(a, 70, &pos).code == DS_OK && pos == 4);
    assert(list_find(a, 999, &pos).code == DS_NOT_FOUND && pos == 0);
    assert(list_find(NULL, 1, &pos).code == DS_NULL_PTR);
    assert(list_find(a, 1, NULL).code == DS_NULL_PTR);
    /* 判空 */
    assert(list_is_empty(a) == 0);
    assert(list_is_empty(NULL) == 1);

    /* 用例11: traverse 回调遍历与 clear 清空（保留表壳） */
    visit_count = 0;
    assert(list_traverse(a, visit_record).code == DS_OK);
    assert(visit_count == 4);
    assert(visit_seq[0] == 10 && visit_seq[1] == 30 &&
           visit_seq[2] == 50 && visit_seq[3] == 70);
    assert(list_traverse(a, NULL).code == DS_NULL_PTR);
    assert(list_traverse(NULL, visit_record).code == DS_NULL_PTR);
    assert(list_clear(a).code == DS_OK);
    assert(list_length(a) == 0);
    assert(list_is_empty(a) == 1);
    visit_count = 0;
    assert(list_traverse(a, visit_record).code == DS_OK);   /* 空表访问 0 次 */
    assert(visit_count == 0);
    assert(list_clear(NULL).code == DS_NULL_PTR);
    assert(list_push_back(a, 88).code == DS_OK);            /* clear 后可继续插入 */
    assert(list_get(a, 1, &value).code == DS_OK && value == 88);
    list_destroy(&a);

    /* 用例12: 约瑟夫环 n=8, m=3 出圈序列应为 3 6 1 5 2 8 4 7 */
    josephus_solve(8, 3, seq);
    for (i = 0; i < 8; i++) {
        assert(seq[i] == expect[i]);
    }
    printf("约瑟夫环 n=8, m=3 出圈顺序: ");
    for (i = 0; i < 8; i++) {
        printf("%d ", seq[i]);
    }
    printf("\n");

    /* 用例13: 销毁后指针置 NULL，重复销毁安全 */
    list_destroy(&a);
    assert(a == NULL);
    list_destroy(&a);
    list_destroy(NULL);

    printf("全部测试通过\n");
    return 0;
}
