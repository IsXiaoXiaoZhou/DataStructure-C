/**
 * @file main.c
 * @brief 循环顺序队列模块测试与演示
 *
 * 测试策略: 覆盖空/满/越界状态码、FIFO 顺序、以及循环回绕后
 * 元素仍按入队顺序出队（环形存储最易错之处）。
 * 队列为栈对象：声明后经 queue_init 复位即可用，无堆分配。
 */

#include <assert.h>
#include <stdio.h>

#include "seq_queue.h"

int main(void)
{
    SeqQueue queue;                  /* 栈对象：定长数组内嵌，无堆依赖 */
    int value;
    int i;

    printf("=== 循环顺序队列模块（牺牲一格判满，容量 %d） ===\n\n",
           SEQQUEUE_MAX_SIZE - 1);

    /* 用例1: 新队列应为空、不满、长度 0 */
    assert(queue_init(&queue).code == DS_OK);
    assert(queue_is_empty(&queue) == 1);
    assert(queue_is_full(&queue) == 0);
    assert(queue_size(&queue) == 0);

    /* 用例2: 空队列出队/取队头应返回 DS_EMPTY */
    assert(queue_dequeue(&queue, &value).code == DS_EMPTY);
    assert(queue_front(&queue, &value).code == DS_EMPTY);

    /* 用例3: 入队至满 —— 恰能装 SEQQUEUE_MAX_SIZE-1 个，再入报 DS_OVERFLOW */
    for (i = 1; i <= SEQQUEUE_MAX_SIZE - 1; i++) {
        assert(queue_enqueue(&queue, i * 10).code == DS_OK);
    }
    assert(queue_is_full(&queue) == 1);
    assert(queue_is_empty(&queue) == 0);
    assert(queue_size(&queue) == (size_t)(SEQQUEUE_MAX_SIZE - 1));
    assert(queue_enqueue(&queue, 999).code == DS_OVERFLOW);

    /* 用例4: 满队时队头应为首个入队元素 10 */
    assert(queue_front(&queue, &value).code == DS_OK);
    assert(value == 10);

    /* 用例5: 出队 50 个后长度减为 49，可再入队 50 个 */
    for (i = 0; i < 50; i++) {
        assert(queue_dequeue(&queue, &value).code == DS_OK);
        assert(value == (i + 1) * 10);   /* FIFO: 按入队顺序出队 */
    }
    assert(queue_size(&queue) == 49);
    for (i = 0; i < 50; i++) {
        assert(queue_enqueue(&queue, 1000 + i).code == DS_OK);  /* rear 回绕使用空格 */
    }
    assert(queue_is_full(&queue) == 1);
    assert(queue_enqueue(&queue, 777).code == DS_OVERFLOW);

    /* 用例6: 全部出队 —— 前段为初始入队的尾部，后段为回绕段，顺序不乱 */
    for (i = 51; i <= SEQQUEUE_MAX_SIZE - 1; i++) {
        assert(queue_dequeue(&queue, &value).code == DS_OK);
        assert(value == i * 10);          /* 初始段的第 51..99 个 */
    }
    for (i = 0; i < 50; i++) {
        assert(queue_dequeue(&queue, &value).code == DS_OK);
        assert(value == 1000 + i);        /* 回绕段依入队顺序出队 */
    }
    assert(queue_is_empty(&queue) == 1);
    assert(queue_dequeue(&queue, &value).code == DS_EMPTY);

    /* 用例7: 出空后 front 追平 rear，队列可继续正常复用 */
    assert(queue_enqueue(&queue, 5).code == DS_OK);
    assert(queue_dequeue(&queue, &value).code == DS_OK && value == 5);
    assert(queue_is_empty(&queue) == 1);

    /* 用例8: 空指针参数返回 DS_NULL_PTR */
    assert(queue_init(NULL).code == DS_NULL_PTR);
    assert(queue_enqueue(NULL, 1).code == DS_NULL_PTR);
    assert(queue_dequeue(NULL, &value).code == DS_NULL_PTR);
    assert(queue_front(NULL, &value).code == DS_NULL_PTR);
    assert(queue_size(NULL) == 0);
    queue_destroy(NULL);                   /* 销毁 NULL 应安全空操作 */

    /* --- 演示段落: 入队至满 -> 出队 -> 回绕再入队 --- */
    printf("--- 演示: 连续入队 %d 个 -> 出队 3 个 -> 再入队 3 个（rear 环形回绕） ---\n",
           SEQQUEUE_MAX_SIZE - 1);
    for (i = 1; i <= SEQQUEUE_MAX_SIZE - 1; i++) {
        assert(queue_enqueue(&queue, i).code == DS_OK);
    }
    printf("入队 %d 个后: 满=%d, 长度=%lu\n",
           SEQQUEUE_MAX_SIZE - 1, queue_is_full(&queue),
           (unsigned long)queue_size(&queue));
    for (i = 0; i < 3; i++) {
        assert(queue_dequeue(&queue, &value).code == DS_OK);
        printf("出队: %d\n", value);
    }
    for (i = 100; i <= 102; i++) {
        assert(queue_enqueue(&queue, i).code == DS_OK);   /* 复用队头腾出的格子 */
    }
    assert(queue_front(&queue, &value).code == DS_OK);
    printf("再入队 3 个后: 满=%d, 长度=%lu，队头=%d\n",
           queue_is_full(&queue), (unsigned long)queue_size(&queue), value);

    queue_destroy(&queue);                 /* 复位字段，无 free */
    assert(queue_is_empty(&queue) == 1);   /* 复位后即为空队列 */
    printf("\n全部测试通过\n");
    return 0;
}
