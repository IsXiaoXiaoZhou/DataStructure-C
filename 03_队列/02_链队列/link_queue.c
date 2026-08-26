/**
 * @file link_queue.c
 * @brief 链队列实现（带头结点，front/rear 双指针）
 *
 * 正确性核心是维护头文件声明的三条不变量:
 * 头结点恒在 / rear 恒指尾 / 删空时 rear 回位 front。
 */

#include "link_queue.h"

#include <stdlib.h>


/* 链结点：数据 + 后继，仅本文件可见 */
typedef struct QueueNode {
    int value;
    struct QueueNode *next;
} QueueNode;

struct LinkQueue {
    QueueNode *front;   /* 恒指向头结点（不存数据） */
    QueueNode *rear;    /* 恒指向链尾结点，空队时即头结点 */
    size_t length;      /* 元素个数，使 queue_size 免遍历 */
};

/* 新建结点的小工厂：入队两处都要用，收敛到一处防漏初始化 next */
static QueueNode *node_new(int value)
{
    QueueNode *node = malloc(sizeof *node);
    if (node != NULL) {
        node->value = value;
        node->next = NULL;
    }
    return node;
}

DsResult queue_init(LinkQueue **queue)
{
    QueueNode *head = NULL;

    if (queue == NULL) {
        return (DsResult){DS_NULL_PTR, "队列二级指针为空"};
    }

    /* 头结点在队列存在期间始终占位，判空判满不依赖它 */
    head = node_new(0);
    if (head == NULL) {
        return (DsResult){DS_OVERFLOW, "头结点内存分配失败"};
    }

    *queue = malloc(sizeof **queue);
    if (*queue == NULL) {
        free(head);
        return (DsResult){DS_OVERFLOW, "队列结构体内存分配失败"};
    }

    (*queue)->front = head;
    (*queue)->rear = head;      /* 空队: rear 与 front 同指头结点 */
    (*queue)->length = 0;
    return (DsResult){DS_OK, "链队列创建成功"};
}

void queue_destroy(LinkQueue **queue)
{
    QueueNode *cur = NULL;
    QueueNode *next = NULL;

    if (queue == NULL || *queue == NULL) {
        return;
    }

    /* 注意：从头结点（而非首元结点）出发逐个释放，头结点也是堆内存 */
    cur = (*queue)->front;
    while (cur != NULL) {
        next = cur->next;
        free(cur);
        cur = next;
    }
    free(*queue);
    *queue = NULL;              /* 句柄归零，杜绝悬空指针继续使用 */
}

DsResult queue_enqueue(LinkQueue *queue, int value)
{
    QueueNode *node = NULL;

    if (queue == NULL) {
        return (DsResult){DS_NULL_PTR, "队列指针为空"};
    }

    node = node_new(value);
    if (node == NULL) {
        return (DsResult){DS_OVERFLOW, "结点内存分配失败"};
    }

    /* 尾插三步：接在 rear 后、rear 前移、计数加一（顺序无依赖但缺一不可） */
    queue->rear->next = node;
    queue->rear = node;
    queue->length++;
    return (DsResult){DS_OK, "入队成功"};
}

DsResult queue_dequeue(LinkQueue *queue, int *value)
{
    QueueNode *first = NULL;

    if (queue == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (queue->front == queue->rear) {
        return (DsResult){DS_EMPTY, "队列为空，无法出队"};        /* 空队: 只剩头结点 */
    }

    first = queue->front->next;         /* 真正的队头元素在头结点之后 */
    *value = first->value;

    queue->front->next = first->next;
    if (first == queue->rear) {
        /*
         * 删除的是最后一个结点：rear 原指向它，删除后必须拉回
         * 头结点，否则 rear 悬空指向已释放内存（野指针），
         * 下一次入队就会写已释放内存 —— 本模块最关键的一步
         */
        queue->rear = queue->front;
    }
    free(first);
    queue->length--;
    return (DsResult){DS_OK, "出队成功"};
}

DsResult queue_front(const LinkQueue *queue, int *value)
{
    if (queue == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (queue->front == queue->rear) {
        return (DsResult){DS_EMPTY, "队列为空，无法取队头"};
    }

    *value = queue->front->next->value;
    return (DsResult){DS_OK, "取队头成功"};
}

int queue_is_empty(const LinkQueue *queue)
{
    if (queue == NULL) {
        return 1;
    }
    return queue->front == queue->rear;
}

int queue_is_full(const LinkQueue *queue)
{
    /* 链式存储按需结点分配，无容量上限；保留接口为与顺序队列同名同形 */
    (void)queue;
    return 0;
}

size_t queue_size(const LinkQueue *queue)
{
    if (queue == NULL) {
        return 0;
    }
    return queue->length;
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
