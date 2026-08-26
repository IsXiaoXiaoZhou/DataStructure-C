/**
 * @file main.c
 * @brief 链队列模块测试与演示
 *
 * 测试策略: 覆盖 FIFO 顺序、删空后 rear 回位（继续入队出队仍正确，
 * 这是链队列最易错的边界）、超长序列滚动验证、状态码。
 */

#include <assert.h>
#include <stdio.h>

#include "link_queue.h"

int main(void)
{
    LinkQueue *queue = NULL;
    int value;
    int i;

    printf("=== 链队列模块（头结点 + front/rear 双指针） ===\n\n");

    /* 用例1: 新队列应为空、长度 0，is_full 恒 0（链式无容量上限） */
    assert(queue_init(&queue).code == DS_OK);
    assert(queue_is_empty(queue) == 1);
    assert(queue_is_full(queue) == 0);
    assert(queue_size(queue) == 0);

    /* 用例2: 空队列出队/取队头应返回 DS_EMPTY */
    assert(queue_dequeue(queue, &value).code == DS_EMPTY);
    assert(queue_front(queue, &value).code == DS_EMPTY);

    /* 用例3: 入队 3 个后按 FIFO 顺序出队 */
    assert(queue_enqueue(queue, 10).code == DS_OK);
    assert(queue_enqueue(queue, 20).code == DS_OK);
    assert(queue_enqueue(queue, 30).code == DS_OK);
    assert(queue_size(queue) == 3);
    assert(queue_front(queue, &value).code == DS_OK && value == 10);
    assert(queue_dequeue(queue, &value).code == DS_OK && value == 10);
    assert(queue_dequeue(queue, &value).code == DS_OK && value == 20);
    assert(queue_dequeue(queue, &value).code == DS_OK && value == 30);
    assert(queue_is_empty(queue) == 1);

    /* 用例4: 删空后 rear 已回位头结点，立即再入队应正常从队尾接入 */
    assert(queue_enqueue(queue, 40).code == DS_OK);
    assert(queue_front(queue, &value).code == DS_OK && value == 40);
    assert(queue_dequeue(queue, &value).code == DS_OK && value == 40);
    assert(queue_is_empty(queue) == 1);

    /* 用例5: 长序列滚动 —— 交叉入队出队 200 轮，顺序与计数始终正确 */
    for (i = 1; i <= 200; i++) {
        assert(queue_enqueue(queue, i).code == DS_OK);
        if (i % 3 == 0) {                    /* 每入 3 个出 1 个 */
            assert(queue_dequeue(queue, &value).code == DS_OK);
            assert(value == i / 3);          /* 第 i/3 次出队的恰是第 i/3 个元素 */
        }
    }
    assert(queue_size(queue) == 200 - 200 / 3);
    assert(queue_front(queue, &value).code == DS_OK && value == 200 / 3 + 1);

    /* 用例6: 全部出队至空，验证出队顺序连续无跳跃 */
    {
        int next_expect = 200 / 3 + 1;       /* 队头从 67 开始 */
        while (!queue_is_empty(queue)) {
            assert(queue_dequeue(queue, &value).code == DS_OK);
            assert(value == next_expect);
            next_expect++;
        }
        /* 67..200 连续出完，下一期望值应恰为 201 */
        assert(next_expect == 201);
    }

    /* 用例7: 空指针参数返回 DS_NULL_PTR；销毁 NULL 安全 */
    assert(queue_init(NULL).code == DS_NULL_PTR);
    assert(queue_enqueue(NULL, 1).code == DS_NULL_PTR);
    assert(queue_dequeue(NULL, &value).code == DS_NULL_PTR);
    assert(queue_front(NULL, &value).code == DS_NULL_PTR);
    assert(queue_size(NULL) == 0);
    queue_destroy(NULL);

    /* 用例8: 正常销毁 —— 全部结点释放后句柄被置为 NULL */
    queue_destroy(&queue);
    assert(queue == NULL);

    /* --- 演示段落: 配对入队出队展示 FIFO 行为 --- */
    printf("--- 演示: 依序入队 1..5，再全部出队 ---\n");
    assert(queue_init(&queue).code == DS_OK);
    for (i = 1; i <= 5; i++) {
        assert(queue_enqueue(queue, i).code == DS_OK);
        printf("入队: %d（当前长度 %lu）\n", i, (unsigned long)queue_size(queue));
    }
    while (queue_dequeue(queue, &value).code == DS_OK) {
        printf("出队: %d\n", value);
    }
    printf("出队后: 空=%d，长度=%lu\n",
           queue_is_empty(queue), (unsigned long)queue_size(queue));

    queue_destroy(&queue);
    assert(queue == NULL);
    printf("\n全部测试通过\n");
    return 0;
}
