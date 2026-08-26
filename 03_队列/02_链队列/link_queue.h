/**
 * @file link_queue.h
 * @brief 链队列（带头结点的单链表 + front/rear 双指针）接口定义
 *
 * 存储结构:
 *   front → [头结点] → [a1] → [a2] → ... → [an]
 *                                              ↑
 *   rear ─────────────────────────────────────┘（空队时 rear 同指头结点）
 *
 * 设计要点:
 *   1. 头结点不变量 —— front 恒指向头结点（永不存数据），
 *      队头元素在 front->next；出队删除首元结点与出队删除
 *      唯一结点的逻辑完全统一，无需对空队特判
 *   2. rear 指针不变量 —— rear 恒指向链上最后一个结点
 *      （空队时指向头结点），入队 O(1) 无需遍历找尾
 *   3. 删空回位 —— 出队删到只剩头结点时必须令 rear = front，
 *      否则 rear 悬空指向已释放结点，破坏不变量
 *   4. 不透明指针 —— 上述三条不变量是正确性关键，
 *      结构体定义放 .c 中强制只能经 API 操作
 *   5. 链式存储无固定容量，queue_is_full 恒返回 0；
 *      内存耗尽时 enqueue 返回 DS_OVERFLOW
 * 复杂度: 入队/出队/取队头/求长度均为 O(1)
 */

#ifndef LINK_QUEUE_H
#define LINK_QUEUE_H

#include <stddef.h>

typedef enum {
    DS_OK = 0,          /* 操作成功 */
    DS_ERROR,           /* 一般性错误（如查找未命中、非法参数组合） */
    DS_NULL_PTR,        /* 空指针参数 */
    DS_OUT_OF_RANGE,    /* 位置/下标越界 */
    DS_OVERFLOW,        /* 空间已满或内存分配失败（含计算结果溢出） */
    DS_EMPTY,           // 对空结构取元素
    DS_NOT_FOUND        // 查找未命中（业务正常结果，非异常）
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

typedef struct LinkQueue LinkQueue;

DsResult queue_init(LinkQueue **queue);
void queue_destroy(LinkQueue **queue);
DsResult queue_enqueue(LinkQueue *queue, int value);
DsResult queue_dequeue(LinkQueue *queue, int *value);
DsResult queue_front(const LinkQueue *queue, int *value);
int queue_is_empty(const LinkQueue *queue);
int queue_is_full(const LinkQueue *queue);
size_t queue_size(const LinkQueue *queue);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* LINK_QUEUE_H */
