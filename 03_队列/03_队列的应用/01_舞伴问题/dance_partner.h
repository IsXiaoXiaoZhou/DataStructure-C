/**
 * @file dance_partner.h
 * @brief 舞伴问题（队列的典型应用）接口定义
 *
 * 问题描述: 舞会开始时舞者按到达顺序入场，男/女各自排成一队。
 * 每支舞曲开始，两队队头各出一人配成舞伴；某队先空则
 * 另一队的剩余者等待下一轮 —— 先到先配对正是队列 FIFO 的体现。
 *
 * 设计要点:
 *   1. 模块内自包含 —— 内部实现两套 static 数组队列
 *      （男队/女队），不依赖任何外部队列模块
 *   2. 结果可断言 —— DanceResult 带回配对数与剩余者名单，
 *      便于 main.c 做自动化测试（打印仅为演示）
 * 复杂度: 分队入队 + 轮流出队各扫一遍，O(n)
 */

#ifndef DANCE_PARTNER_H
#define DANCE_PARTNER_H

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

/* 最多同时到场人数与姓名缓冲区长度（按 UTF-8 汉字预留） */
#define DANCE_MAX_COUNT 32
#define DANCE_NAME_LEN  16

/* 舞者记录：姓名 + 性别（'M' 男 / 'F' 女） */
typedef struct {
    char name[DANCE_NAME_LEN];
    char sex;
} Dancer;

/* 配对结果：带回配对数与剩余者姓名，供调用方断言/检查 */
typedef struct {
    size_t pair_count;                                  /* 配成舞伴的对数 */
    char  leftover[DANCE_MAX_COUNT][DANCE_NAME_LEN];    /* 未配到舞伴的姓名 */
    size_t leftover_count;                              /* 未配到舞伴的人数 */
} DanceResult;

DsResult dance_partner(const Dancer *dancers, size_t count, DanceResult *result);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* DANCE_PARTNER_H */
