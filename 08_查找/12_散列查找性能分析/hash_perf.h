#ifndef HASH_PERF_H
#define HASH_PERF_H

/**
 * @file hash_perf.h
 * @brief 散列查找性能分析接口定义
 *
 * 本模块提供三种散列结构的 ASL 精确计算:
 *   1. 拉链法 (separate chaining)
 *   2. 线性探测 (linear probing)
 *   3. 二次探测 (quadratic probing)
 *
 * 理论公式（拉链法）:
 *   成功 ASL ≈ 1 + α/2
 *   不成功 ASL ≈ α
 *
 * 理论公式（开放定址，α < 1）:
 *   成功 ASL ≈ (1/α) * ln(1/(1-α))
 *   不成功 ASL ≈ 1/(1-α)
 */

#include <stddef.h>

/* 七值状态码（与全工程逐字一致） */
typedef enum {
    DS_OK = 0,
    DS_ERROR,
    DS_NULL_PTR,
    DS_OUT_OF_RANGE,
    DS_OVERFLOW,
    DS_EMPTY,
    DS_NOT_FOUND        // 查找未命中（业务正常结果，非异常）
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

DsResult hp_chain_asl(const int keys[], size_t n, int m,
                      double *asl_s, double *asl_f);

typedef enum {
    HP_LINEAR = 0,
    HP_QUADRATIC
} HPStrategy;

DsResult hp_open_asl(const int keys[], size_t n, int m,
                     HPStrategy strategy, double *asl_s, double *asl_f);

DsResult hp_alpha_scan(const int keys[], size_t n, int m,
                       double alpha_start, double alpha_end, int steps);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* HASH_PERF_H */
