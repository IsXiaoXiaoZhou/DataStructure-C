/**
 * @file seq_queue.c
 * @brief 循环顺序队列实现
 *
 * 核心不变量: rear 指向队尾元素的下一格，因此
 *   空 <=> front == rear
 *   满 <=> (rear + 1) % MAX == front
 * 所有下标移动必须 % SEQQUEUE_MAX_SIZE 才能落到环上。
 * 队列是调用方持有的栈对象，本文件无任何堆分配。
 */

#include "seq_queue.h"


DsResult queue_init(SeqQueue *queue)
{
    if (queue == NULL) {
        return (DsResult){DS_NULL_PTR, "队列指针为空"};
    }

    /* front == rear 即空队列，两个下标都从 0 出发 */
    queue->front = 0;
    queue->rear = 0;
    return (DsResult){DS_OK, "队列初始化成功"};
}

void queue_destroy(SeqQueue *queue)
{
    if (queue == NULL) {
        return;
    }

    /* 定长数组无堆内存可释放，复位下标即回到初始空队列状态 */
    queue->front = 0;
    queue->rear = 0;
}

DsResult queue_enqueue(SeqQueue *queue, int value)
{
    if (queue == NULL) {
        return (DsResult){DS_NULL_PTR, "队列指针为空"};
    }
    if ((queue->rear + 1) % SEQQUEUE_MAX_SIZE == queue->front) {
        return (DsResult){DS_OVERFLOW, "队列已满，无法入队"};      /* 空余的最后一格是为区分空满保留的 */
    }

    queue->data[queue->rear] = value;
    queue->rear = (queue->rear + 1) % SEQQUEUE_MAX_SIZE;  /* 取模实现回绕 */
    return (DsResult){DS_OK, "入队成功"};
}

DsResult queue_dequeue(SeqQueue *queue, int *value)
{
    if (queue == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (queue->front == queue->rear) {
        return (DsResult){DS_EMPTY, "队列为空，无法出队"};
    }

    *value = queue->data[queue->front];
    queue->front = (queue->front + 1) % SEQQUEUE_MAX_SIZE;
    return (DsResult){DS_OK, "出队成功"};
}

DsResult queue_front(const SeqQueue *queue, int *value)
{
    if (queue == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (queue->front == queue->rear) {
        return (DsResult){DS_EMPTY, "队列为空，无法取队头"};
    }

    *value = queue->data[queue->front];
    return (DsResult){DS_OK, "取队头成功"};
}

int queue_is_empty(const SeqQueue *queue)
{
    if (queue == NULL) {
        return 1;
    }
    return queue->front == queue->rear;
}

int queue_is_full(const SeqQueue *queue)
{
    if (queue == NULL) {
        return 0;
    }
    return (queue->rear + 1) % SEQQUEUE_MAX_SIZE == queue->front;
}

size_t queue_size(const SeqQueue *queue)
{
    if (queue == NULL) {
        return 0;
    }
    /*
     * 环上 rear 可能已回绕到 front 前方（rear < front），
     * 先加一个 MAX 保证差值非负，再取模折叠回 [0, MAX-1]
     */
    return (queue->rear + SEQQUEUE_MAX_SIZE - queue->front) % SEQQUEUE_MAX_SIZE;
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:         return "操作成功";
        case DS_ERROR:      return "一般性错误";
        case DS_NULL_PTR:   return "空指针参数";
        case DS_OUT_OF_RANGE: return "位置/下标越界";
        case DS_OVERFLOW:   return "空间已满或内存分配失败";
        case DS_EMPTY:      return "对空结构取元素";
        case DS_NOT_FOUND:  return "查找未命中";
        default:            return "未知状态码";
    }
}
