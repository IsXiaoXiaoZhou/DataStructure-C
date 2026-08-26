/**
 * @file kmp_match.c
 * @brief KMP 模式匹配算法接口实现
 *
 * 核心不变量: 匹配循环中 s[i-j..i-1] == t[0..j-1] 恒成立
 *            （主串最近 j 个字符与模式前缀已匹配），
 *            失配时 j = jump[j] 恰好维持该不变量且 i 不回退
 * 设计要点:
 *   1. get_next 自匹配: 模式串自己和自己做一次 KMP，
 *      next[j+1] 由 next[j]+1 或沿 next 链回退得到
 *   2. next/nextval[0] 弃用（教材 1-based 记法），实现内部
 *      字符串访问统一减 1 换回 0-based
 * 复杂度: 均摊分析 —— i 与 j 各自至多增加 m/n 次，跳跃总量
 *         不超过增加量，故匹配 O(n)、预处理 O(m)
 */

#include "kmp_match.h"

#include <stddef.h>


/* 内部辅助: 求 C 字符串长度 */
static size_t str_len(const char *cstr)
{
    size_t n = 0;

    while (cstr[n] != '\0') {
        n++;
    }
    return n;
}

DsResult kmp_get_next(const char *t, size_t next[])
{
    size_t m = 0;
    size_t i = 1; /* i: 当前待求位（1-based，next[i] 即将产生） */
    size_t j = 0; /* j: t[1..i-1] 的最长相等前后缀长度（0-based 恰为下一比较位） */

    if (t == NULL || next == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    m = str_len(t);
    if (m > KMP_MAX_PATTERN) {
        return (DsResult){DS_ERROR, "模式串超出容量上限"};
    }

    next[1] = 0; /* 教材约定: 首字符失配时 j 归 0（配合 j==0 时 i++） */

    while (i < m) {
        /*
         * 不变量: t[1..i-1] 的前后缀匹配进行到第 j 位，
         * 即 t[1..j] == t[i-j..i-1]（教材 1-based 记法）
         */
        if (j == 0 || t[i - 1] == t[j - 1]) {
            i++;
            j++;
            next[i] = j; /* 两端相等（或已回退到头），前后缀长度 +1 */
        } else {
            j = next[j]; /* 沿 next 链回退: 对次长相等前后缀再试 */
        }
    }
    return (DsResult){DS_OK, "next数组计算成功"};
}

DsResult kmp_get_nextval(const char *t, const size_t next[], size_t nextval[])
{
    size_t m = 0;
    size_t j = 0;

    if (t == NULL || next == NULL || nextval == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    m = str_len(t);
    nextval[1] = 0;

    for (j = 2; j <= m; j++) {
        if (t[next[j] - 1] == t[j - 1]) {
            /* 跳到 next[j] 后必然再失配于同一字符，直接继承更短的等价跳转 */
            nextval[j] = nextval[next[j]];
        } else {
            nextval[j] = next[j];
        }
    }
    return (DsResult){DS_OK, "nextval数组计算成功"};
}

/* 内部核心: 通用 KMP 匹配体，cmp 非 NULL 时统计比较次数 */
static DsResult match_core(const char *s, const char *t, const size_t jump[],
                           size_t *pos, size_t *cmp)
{
    size_t n = 0;
    size_t m = 0;
    size_t i = 0; /* 主串游标（0-based，只进不退） */
    size_t j = 0; /* 已匹配字符数（= 模式下一比较位 0-based；对 jump 表为 1-based 位号） */

    n = str_len(s);
    m = str_len(t);

    if (m == 0) {
        *pos = 1;
        if (cmp != NULL) {
            *cmp = 0;
        }
        return (DsResult){DS_OK, "空模式约定命中位置1"};
    }

    while (i < n && j < m) {
        if (cmp != NULL) {
            (*cmp)++;
        }
        if (s[i] == t[j]) {
            i++;
            j++;
        } else if (j == 0) {
            /* 模式第 1 字符就失配: 主串右移一格，j 保持 0 */
            i++;
        } else {
            /*
             * 位号换算: 0-based 已匹配 j 个 == 失配于教材 1-based 位 j+1，
             * 跳转目标 jump[j+1] 是 1-based 新位号，再减 1 回 0-based。
             * 主串 i 原地不动 —— 免回溯的标志。
             * 注意 nextval 的优化链可能把值归 0（整段重来: i 右移、j 归 0），
             * 此时不可做 j1-1，否则 size_t 下溢
             */
            size_t j1 = jump[j + 1];

            if (j1 == 0) {
                i++;
                j = 0;
            } else {
                j = j1 - 1;
            }
        }
    }

    if (j == m) {
        *pos = i - m + 1;
        return (DsResult){DS_OK, "匹配成功"};
    }
    *pos = 0;
    return (DsResult){DS_ERROR, "未找到匹配位置"};
}

DsResult kmp_index(const char *s, const char *t, const size_t jump[], size_t *pos)
{
    if (s == NULL || t == NULL || jump == NULL || pos == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    return match_core(s, t, jump, pos, NULL);
}

DsResult kmp_index_count(const char *s, const char *t, const size_t jump[],
                         size_t *pos, size_t *cmp_count)
{
    if (s == NULL || t == NULL || jump == NULL || pos == NULL || cmp_count == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *cmp_count = 0;
    return match_core(s, t, jump, pos, cmp_count);
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
