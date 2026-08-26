/**
 * @file seq_queue.h
 * @brief 循环顺序队列（定长数组 + 牺牲一格判满）接口定义
 *
 * 存储结构: data[0..MAX-1] 视为首尾相接的环
 *   front ---- 指向队头元素
 *   rear  ---- 指向队尾元素的下一格（队尾留空一格）
 *   判空: front == rear
 *   判满: (rear + 1) % MAX == front   （实际容量 MAX-1）
 *
 * 设计要点:
 *   1. 牺牲一格法 —— 若 rear 直接指向队尾元素，则"空"与"满"
 *      的判定条件都退化为 front == rear 而无法区分；约定 rear
 *      指向下一可写位置后，满时恰剩一格空位，两种状态得以区分，
 *      代价是容量减少 1
 *   2. 对比 tag 法 —— 另一种方案是增设标志域 tag（0 空 / 1 满），
 *      front == rear 时再查 tag 区分空满，容量可利用满 MAX 格，
 *      但每次入队出队都要维护 tag，判空判满多一次比较，
 *      且不变量更多、更易写错；教学上多采用牺牲一格法求简
 *   3. 定长数组无堆依赖 —— 与顺序栈同构：编译期定长数组 +
 *      头尾下标、无动态内存，按"定长数组无不变量例外条款"
 *      直接在头文件暴露结构体，调用方声明栈对象即可用；
 *      教学上让 front/rear 随入队出队环形移动的存储形象
 *      一目了然（下标取模约定仅靠 API 维护即可保证）
 * 复杂度: 入队/出队/取队头均为 O(1)
 */

#ifndef SEQ_QUEUE_H
#define SEQ_QUEUE_H

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

#define SEQQUEUE_MAX_SIZE 100      /* 数组物理大小，实际容量为 99 */

/* 循环队列：定长环形数据区 + 头尾下标（与顺序栈同构的栈对象） */
typedef struct {
    int data[SEQQUEUE_MAX_SIZE];   /* 环形存储 */
    size_t front;                  /* 队头元素下标 */
    size_t rear;                   /* 队尾元素下一格下标 */
} SeqQueue;

DsResult queue_init(SeqQueue *queue);
void queue_destroy(SeqQueue *queue);
DsResult queue_enqueue(SeqQueue *queue, int value);
DsResult queue_dequeue(SeqQueue *queue, int *value);
DsResult queue_front(const SeqQueue *queue, int *value);
int queue_is_empty(const SeqQueue *queue);
int queue_is_full(const SeqQueue *queue);
size_t queue_size(const SeqQueue *queue);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* SEQ_QUEUE_H */
