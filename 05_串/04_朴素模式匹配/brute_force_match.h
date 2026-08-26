#ifndef BRUTE_FORCE_MATCH_H
#define BRUTE_FORCE_MATCH_H

/**
 * @file brute_force_match.h
 * @brief 朴素模式匹配算法（BF / Brute-Force）接口定义
 *
 * 算法思想: 依次把模式串对齐主串的每个起点（1..n-m+1），
 *           逐字符比较；失配则主串指针"回溯"到对齐起点+1，
 *           模式串指针归 1，重新开始
 * 设计要点:
 *   1. 教材 Index(S, T, pos) 的直接实现，无任何预处理——
 *      这正是与 KMP 的对比基线（KMP 免回溯，见 05 模块）
 *   2. 提供 *_count 变体统计字符比较次数，用于
 *      实测量化"主串指针回溯"带来的重复比较代价
 * 复杂度: 最好 O(n)（首起点即命中）；最坏 O((n-m+1)*m)，
 *         如 S="aaabaaab", T="aaab" 类高重复串
 */

#include <stddef.h>

/* 状态码（按本模块所需裁剪，语义见各枚举项注释） */
typedef enum {
    DS_OK = 0,     /* 操作成功（命中） */
    DS_ERROR,      /* 未命中 */
    DS_NULL_PTR    /* 空指针参数 */
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

DsResult bf_index(const char *s, const char *t, size_t *pos);
DsResult bf_index_count(const char *s, const char *t, size_t *pos, size_t *cmp_count);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* BRUTE_FORCE_MATCH_H */
