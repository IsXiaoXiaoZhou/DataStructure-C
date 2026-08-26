/**
 * @file brute_force_match.c
 * @brief 朴素模式匹配算法（BF）接口实现
 *
 * 算法框架: start 对齐起点（0-based），j 模式内偏移。
 *           失配时 start 前进 1、j 归零 —— 等价于教材的
 *           "i 回溯 i-j+2, j 归 1"（1-based 记法）
 * 设计要点:
 *   1. 单一实现体 + 开关参数避免两份代码漂移：
 *      cmp 为 NULL 时即普通版
 *   2. 主串指针 i 永不回退的写法（用 start+j 计算实际位置），
 *      与教材"i 回溯"写法等价，但省去显式回退赋值
 * 复杂度: O(n*m)
 */

#include "brute_force_match.h"

#include <stddef.h>


/*
 * 核心匹配实现（内部辅助）。
 * cmp 非 NULL 时累加比较次数（教学统计版），为 NULL 时跳过计数。
 */
static DsResult match_core(const char *s, const char *t, size_t *pos,
                           size_t *cmp)
{
    size_t slen = 0;
    size_t tlen = 0;
    size_t i = 0;
    size_t start = 0;

    /* 求长度 */
    while (s[slen] != '\0') slen++;
    while (t[tlen] != '\0') tlen++;

    /* 空模式约定命中位置 1 */
    if (tlen == 0) {
        *pos = 1;
        return (DsResult){DS_OK, "空模式约定命中位置1"};
    }

    /* 模式长于主串不可能命中 */
    if (tlen > slen) {
        *pos = 0;
        return (DsResult){DS_ERROR, "模式串长于主串，无法匹配"};
    }

    for (start = 0; start <= slen - tlen; start++) {
        for (i = 0; i < tlen; i++) {
            if (cmp != NULL) (*cmp)++;
            if (s[start + i] != t[i]) {
                break;
            }
        }
        if (i == tlen) {
            *pos = start + 1; /* 转为 1-based */
            return (DsResult){DS_OK, "匹配成功"};
        }
    }

    *pos = 0;
    return (DsResult){DS_ERROR, "未找到匹配位置"};
}

DsResult bf_index(const char *s, const char *t, size_t *pos)
{
    if (s == NULL || t == NULL || pos == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    return match_core(s, t, pos, NULL);
}

DsResult bf_index_count(const char *s, const char *t, size_t *pos, size_t *cmp_count)
{
    if (s == NULL || t == NULL || pos == NULL || cmp_count == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *cmp_count = 0;
    return match_core(s, t, pos, cmp_count);
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "未命中";
        case DS_NULL_PTR:    return "空指针参数";
        default:             return "未知状态码";
    }
}
