#ifndef KMP_MATCH_H
#define KMP_MATCH_H

/**
 * @file kmp_match.h
 * @brief KMP 模式匹配算法（next 数组 + nextval 优化）接口定义
 *
 * 算法思想: BF 的失配回溯源于"已匹配前缀信息的浪费"。KMP 对模式串
 *           预处理出 next[j]（前 j 个字符的最长相等前后缀长度），
 *           失配时模式串"滑动"到 next[j]，主串指针 i 永不回退
 * 设计要点:
 *   1. next 数组采用教材 1-based 约定: next[1]=0，next[j] 为
 *      t[1..j-1] 的最长相等前后缀长度（本模块 next[0] 槽位弃用，
 *      与王道/严蔚敏教材页码记法直接对应）
 *   2. nextval 是 next 的进一步优化: 若 t[next[j]] == t[j]，
 *      则失配于 j 时跳到 next[j] 必然再次失配，可直接继承
 *      nextval[next[j]]，减少无效跳转
 * 复杂度: 预处理 O(m)；匹配 O(n)；总计 O(n+m)（对比 BF 最坏 O(n*m)）
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

/* next/nextval 数组容量上限（模式串最大长度，next[0] 槽位弃用） */
#define KMP_MAX_PATTERN 1024

DsResult kmp_get_next(const char *t, size_t next[]);
DsResult kmp_get_nextval(const char *t, const size_t next[], size_t nextval[]);
DsResult kmp_index(const char *s, const char *t, const size_t jump[], size_t *pos);
DsResult kmp_index_count(const char *s, const char *t, const size_t jump[],
                         size_t *pos, size_t *cmp_count);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* KMP_MATCH_H */
