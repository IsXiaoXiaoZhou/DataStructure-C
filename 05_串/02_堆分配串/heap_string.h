#ifndef HEAP_STRING_H
#define HEAP_STRING_H

/**
 * @file heap_string.h
 * @brief 堆分配串（动态存储的顺序串）接口定义
 *
 * 存储结构: 堆区动态数组 ch[0..length-1]，按需 realloc 扩容，无上限
 * 设计要点:
 *   1. 与定长顺序串的区别仅在"容量来自堆"；逻辑结构与操作集一致，
 *      但赋值/连接永不溢出，代价是必须 destroy 释放并防泄漏
 *   2. 存储分配失败统一返回 DS_ERROR（区别于结构性错误）
 * 复杂度: 按位存取 O(1)；整串操作 O(n)；扩容均摊 O(1)
 */

#include <stddef.h>

/* 状态码（按本模块所需裁剪，语义见各枚举项注释） */
typedef enum {
    DS_OK = 0,       /* 操作成功 */
    DS_ERROR,        /* 一般性错误（本模块用于分配失败/定位未命中） */
    DS_NULL_PTR,     /* 空指针参数 */
    DS_OUT_OF_RANGE  /* 位置/长度越界 */
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 堆分配串：动态字符数组 + 当前长度（容量由分配器管理，内部记录） */
typedef struct {
    char  *ch;    /* 堆区缓冲区，可为 NULL（空串零分配） */
    size_t length; /* 当前字符个数，不含 '\0' */
} HeapString;

DsResult hs_init(HeapString *s);
DsResult hs_destroy(HeapString *s);
DsResult hs_assign(HeapString *s, const char *cstr);
DsResult hs_copy(HeapString *dst, const HeapString *src);
size_t hs_length(const HeapString *s);
int hs_is_empty(const HeapString *s);
DsResult hs_clear(HeapString *s);
DsResult hs_compare(const HeapString *a, const HeapString *b, int *result);
DsResult hs_concat(HeapString *dst, const HeapString *a, const HeapString *b);
DsResult hs_substring(HeapString *out, const HeapString *src, size_t pos, size_t len);
DsResult hs_to_cstr(const HeapString *s, char **cstr);
DsResult hs_print(const HeapString *s);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* HEAP_STRING_H */
