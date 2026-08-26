#ifndef STATIC_STRING_H
#define STATIC_STRING_H

/**
 * @file static_string.h
 * @brief 定长顺序串（定长数组实现）接口定义
 *
 * 存储结构: data[0..length-1] 连续存放字符序列，'\0' 不参与存储
 * 设计要点:
 *   1. 与静态顺序表同源的定长实现，编译期确定容量，无动态内存，
 *      故无需 destroy；串与线性表的唯一区别是数据元素为字符且
 *      操作集以"整体复制/比较/连接"为主，而非单点存取
 *   2. 超长策略: 采用报错（DS_OVERFLOW）而非教材的静默截断，
 *      让调用方显式感知容量边界
 * 复杂度: 按位存取 O(1)；赋值/连接/求子串均为 O(n) 整块复制
 */

#include <stddef.h>

/* 状态码（按本模块所需裁剪，语义见各枚举项注释） */
typedef enum {
    DS_OK = 0,       /* 操作成功 */
    DS_ERROR,        /* 一般性错误（本模块用于定位未命中） */
    DS_NULL_PTR,     /* 空指针参数 */
    DS_OUT_OF_RANGE, /* 位置/长度越界 */
    DS_OVERFLOW      /* 超出定长容量 */
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 串最大容量（编译期定长，不含 '\0' 终止符——内部按长度管理） */
#define STR_MAX_SIZE 100

/* 定长顺序串：定长字符数组 + 当前长度 */
typedef struct {
    char   data[STR_MAX_SIZE]; /* 数据区：data[0] 存放第 1 个字符 */
    size_t length;             /* 当前字符个数，不含 '\0' */
} StaticString;

DsResult ss_assign(StaticString *s, const char *cstr);
size_t ss_length(const StaticString *s);
int ss_is_empty(const StaticString *s);
DsResult ss_clear(StaticString *s);
DsResult ss_copy(StaticString *dst, const StaticString *src);
DsResult ss_compare(const StaticString *a, const StaticString *b, int *result);
DsResult ss_concat(StaticString *dst, const StaticString *a, const StaticString *b);
DsResult ss_substring(StaticString *out, const StaticString *src, size_t pos, size_t len);
DsResult ss_index(const StaticString *src, const StaticString *pattern, size_t *pos);
DsResult ss_print(const StaticString *s);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* STATIC_STRING_H */
