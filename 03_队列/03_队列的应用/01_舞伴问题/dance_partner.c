/**
 * @file dance_partner.c
 * @brief 舞伴问题实现：内部两套 static 数组队列
 *
 * 队列采用"数组 + 头尾下标"的静态实现:
 *   enqueue: data[tail++] = x
 *   dequeue: x = data[head++]
 * 本场景每轮只入队一次、出队一次，无复用回绕需求，
 * 故用线性下标即可；环形取模版见循环顺序队列模块。
 */

#include "dance_partner.h"

#include <stdio.h>


/* 男/女各一套独立队列，容量按最多到场人数预留 */
static char male_queue[DANCE_MAX_COUNT][DANCE_NAME_LEN];
static size_t male_head, male_tail;      /* [head, tail) 为队中元素 */

static char female_queue[DANCE_MAX_COUNT][DANCE_NAME_LEN];
static size_t female_head, female_tail;

DsResult dance_partner(const Dancer *dancers, size_t count, DanceResult *result)
{
    size_t i;

    if (dancers == NULL || result == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (count > DANCE_MAX_COUNT) {
        return (DsResult){DS_OUT_OF_RANGE, "人数超出上限"};
    }

    /*
     * 阶段一: 按性别分队 —— 到达顺序即入队顺序，FIFO 保证先到先配。
     * 姓名复制一律用 snprintf 而非 strcpy：strcpy 不感知目标容量，
     * 一旦源串缺少 '\0' 终止符或长于目标缓冲区，写入即越界；
     * snprintf 以 sizeof 限定写入字节数并保证结尾 '\0'，
     * 超长姓名被安全截断为 DANCE_NAME_LEN - 1 个字符。
     */
    male_head = male_tail = 0;
    female_head = female_tail = 0;
    for (i = 0; i < count; i++) {
        if (dancers[i].sex == 'M') {
            snprintf(male_queue[male_tail], sizeof male_queue[male_tail],
                     "%s", dancers[i].name);
            male_tail++;
        } else if (dancers[i].sex == 'F') {
            snprintf(female_queue[female_tail],
                     sizeof female_queue[female_tail],
                     "%s", dancers[i].name);
            female_tail++;
        }
        /* 其他性别标记视为无效记录，跳过不计 */
    }

    result->pair_count = 0;
    result->leftover_count = 0;

    /* 阶段二: 轮流队头出队配对，任一队先空则停止 */
    while (male_head < male_tail && female_head < female_tail) {
        printf("第 %lu 对: %-8s(男)  ↔  %-8s(女)\n",
               (unsigned long)(result->pair_count + 1),
               male_queue[male_head], female_queue[female_head]);
        male_head++;
        female_head++;
        result->pair_count++;
    }

    /* 阶段三: 未出队者即剩余者，从队头到队尾依次报告（同样防越界写） */
    while (male_head < male_tail) {
        snprintf(result->leftover[result->leftover_count],
                 sizeof result->leftover[result->leftover_count],
                 "%s", male_queue[male_head]);
        male_head++;
        result->leftover_count++;
    }
    while (female_head < female_tail) {
        snprintf(result->leftover[result->leftover_count],
                 sizeof result->leftover[result->leftover_count],
                 "%s", female_queue[female_head]);
        female_head++;
        result->leftover_count++;
    }

    if (result->leftover_count > 0) {
        printf("本场剩余 %lu 人未能配对:",
               (unsigned long)result->leftover_count);
        for (i = 0; i < result->leftover_count; i++) {
            printf(" %s", result->leftover[i]);
        }
        printf("\n");
    } else {
        printf("所有人都配到了舞伴\n");
    }

    return (DsResult){DS_OK, "配对完成"};
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
