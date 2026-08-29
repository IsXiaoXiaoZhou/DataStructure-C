#ifndef HASH_FUNC_H
#define HASH_FUNC_H

/**
 * @file hash_func.h
 * @brief 五种散列函数接口定义
 *
 * 散列函数 H(key) 将关键字映射到地址空间 [0, m)，
 * 目标: 分布均匀、计算简单、冲突少。
 *
 * 本模块实现五种经典散列函数:
 *   1. 直接定址法: H(key) = key（或 a·key + b），恒等映射、零冲突；
 *      适用于关键字集合小且连续的场景，表长需覆盖关键字取值范围
 *      （函数内不做 % m 压缩，调用方直接以 key 为桶号）
 *   2. 除留余数法: H(key) = key % p，p 为 <= m 的最大素数
 *   3. 数字分析法: 取关键字十进制特定位组合
 *   4. 平方取中法: 取 key^2 的中间若干位
 *   5. 折叠法: 将关键字分段叠加
 */

#include <stddef.h>

/* 六值状态码（与全工程逐字一致） */
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

int hf_direct(int key, int m);
int hf_division(int key, int m);
int hf_digit_analysis(int key, int m);
int hf_mid_square(int key, int m);
int hf_folding(int key, int m);

typedef int (*HashFunc)(int key, int m);

DsResult hf_distribution(HashFunc hf, const int keys[], size_t n,
                         int m, int buckets[], double *chi2);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* HASH_FUNC_H */
