/**
 * @file heap_string.c
 * @brief 堆分配串（动态存储的顺序串）接口实现
 *
 * 存储结构: 堆区缓冲区 ch，容量 ≥ length（+1 给 '\0'）
 * 设计要点:
 *   1. 统一的 ensure_capacity 辅助函数集中管理 realloc，
 *      消除散落的分配错误处理；realloc 失败原块不动，无泄漏
 *   2. concat 处理重叠：先算总长，若 dst 缓冲区会被 a/b 内容
 *      覆盖则先备份，再写入 —— 保证 hs_concat(s, s, s) 正确
 * 复杂度: 整串操作 O(n)
 */

#include <stdio.h>
#include <stdlib.h>

#include "heap_string.h"


/* 确保 s 的缓冲区至少能容纳 need 个字符（+'\0'） */
static DsStatus ensure_capacity(HeapString *s, size_t need)
{
    char *buf = NULL;

    /*
     * realloc(NULL, n) 等价 malloc(n)，首次分配与扩容统一走此路径。
     * 预留 1 字节给 '\0'，使缓冲区可作只读 C 字符串使用
     */
    buf = (char *)realloc(s->ch, need + 1);
    if (buf == NULL) {
        return DS_ERROR; /* realloc 失败时原缓冲区仍有效，状态未破坏 */
    }
    s->ch = buf;
    return DS_OK;
}

/* 内部辅助: 求 C 字符串长度 */
static size_t str_len(const char *cstr)
{
    size_t n = 0;

    while (cstr[n] != '\0') {
        n++;
    }
    return n;
}

DsResult hs_init(HeapString *s)
{
    if (s == NULL) {
        return (DsResult){DS_NULL_PTR, "串指针为空"};
    }

    s->ch = NULL;
    s->length = 0;
    return (DsResult){DS_OK, "初始化成功"};
}

DsResult hs_destroy(HeapString *s)
{
    if (s == NULL) {
        return (DsResult){DS_NULL_PTR, "串指针为空"};
    }

    free(s->ch);     /* free(NULL) 合法，重复 destroy 安全 */
    s->ch = NULL;
    s->length = 0;
    return (DsResult){DS_OK, "销毁成功"};
}

DsResult hs_assign(HeapString *s, const char *cstr)
{
    size_t n = 0;
    size_t i = 0;

    if (s == NULL || cstr == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    n = str_len(cstr);
    if (ensure_capacity(s, n) != DS_OK) {
        return (DsResult){DS_ERROR, "内存分配失败"};
    }
    for (i = 0; i < n; i++) {
        s->ch[i] = cstr[i];
    }
    s->ch[n] = '\0'; /* 尾置终止符，支持只读 C 字符串访问 */
    s->length = n;
    return (DsResult){DS_OK, "赋值成功"};
}

DsResult hs_copy(HeapString *dst, const HeapString *src)
{
    size_t i = 0;

    if (dst == NULL || src == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    if (ensure_capacity(dst, src->length) != DS_OK) {
        return (DsResult){DS_ERROR, "内存分配失败"};
    }
    for (i = 0; i < src->length; i++) {
        dst->ch[i] = src->ch[i];
    }
    dst->ch[src->length] = '\0';
    dst->length = src->length;
    return (DsResult){DS_OK, "复制成功"};
}

size_t hs_length(const HeapString *s)
{
    return (s == NULL) ? 0 : s->length;
}

int hs_is_empty(const HeapString *s)
{
    return (s == NULL) || (s->length == 0);
}

DsResult hs_clear(HeapString *s)
{
    if (s == NULL) {
        return (DsResult){DS_NULL_PTR, "串指针为空"};
    }

    /* 保留缓冲区复用（下次 assign 免一次分配），仅逻辑清空 */
    s->length = 0;
    if (s->ch != NULL) {
        s->ch[0] = '\0';
    }
    return (DsResult){DS_OK, "清空成功"};
}

DsResult hs_compare(const HeapString *a, const HeapString *b, int *result)
{
    size_t n = 0;
    size_t i = 0;

    if (a == NULL || b == NULL || result == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    n = (a->length < b->length) ? a->length : b->length;
    for (i = 0; i < n; i++) {
        if (a->ch[i] != b->ch[i]) {
            *result = (int)(unsigned char)a->ch[i] - (int)(unsigned char)b->ch[i];
            return (DsResult){DS_OK, "比较完成"};
        }
    }
    *result = (a->length < b->length) ? -1 : ((a->length > b->length) ? 1 : 0);
    return (DsResult){DS_OK, "比较完成"};
}

DsResult hs_concat(HeapString *dst, const HeapString *a, const HeapString *b)
{
    size_t total = 0;
    size_t i = 0;

    if (dst == NULL || a == NULL || b == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    total = a->length + b->length;

    /*
     * 重叠防护: 先扩容 realloc 会使 dst->ch 指向新块，但若 dst
     * 与 a/b 同串，旧块在 realloc 时已被释放——因此先在旧缓冲区
     * 尚有效时备份内容，再写入新块
     */
    {
        char *backup_a = NULL;
        char *backup_b = NULL;

        if (a->length > 0) {
            backup_a = (char *)malloc(a->length);
            if (backup_a == NULL) {
                return (DsResult){DS_ERROR, "备份内存分配失败"};
            }
            for (i = 0; i < a->length; i++) {
                backup_a[i] = a->ch[i];
            }
        }
        if (b->length > 0) {
            backup_b = (char *)malloc(b->length);
            if (backup_b == NULL) {
                free(backup_a);
                return (DsResult){DS_ERROR, "备份内存分配失败"};
            }
            for (i = 0; i < b->length; i++) {
                backup_b[i] = b->ch[i];
            }
        }

        if (ensure_capacity(dst, total) != DS_OK) {
            free(backup_a);
            free(backup_b);
            return (DsResult){DS_ERROR, "内存分配失败"};
        }

        for (i = 0; i < a->length; i++) {
            dst->ch[i] = backup_a[i];
        }
        for (i = 0; i < b->length; i++) {
            dst->ch[a->length + i] = backup_b[i];
        }
        free(backup_a);
        free(backup_b);
    }

    dst->ch[total] = '\0';
    dst->length = total;
    return (DsResult){DS_OK, "连接成功"};
}

DsResult hs_substring(HeapString *out, const HeapString *src, size_t pos, size_t len)
{
    size_t i = 0;

    if (out == NULL || src == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (pos < 1 || pos > src->length) {
        return (DsResult){DS_OUT_OF_RANGE, "起始位置越界，合法范围 [1, length]"};
    }
    if (len > src->length - pos + 1) {
        return (DsResult){DS_OUT_OF_RANGE, "子串长度越界"};
    }

    if (ensure_capacity(out, len) != DS_OK) {
        return (DsResult){DS_ERROR, "内存分配失败"};
    }
    for (i = 0; i < len; i++) {
        out->ch[i] = src->ch[pos - 1 + i];
    }
    out->ch[len] = '\0';
    out->length = len;
    return (DsResult){DS_OK, "求子串成功"};
}

DsResult hs_to_cstr(const HeapString *s, char **cstr)
{
    size_t i = 0;

    if (s == NULL || cstr == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    *cstr = (char *)malloc(s->length + 1);
    if (*cstr == NULL) {
        return (DsResult){DS_ERROR, "内存分配失败"};
    }
    for (i = 0; i < s->length; i++) {
        (*cstr)[i] = s->ch[i];
    }
    (*cstr)[s->length] = '\0';
    return (DsResult){DS_OK, "导出C字符串成功"};
}

DsResult hs_print(const HeapString *s)
{
    size_t i = 0;

    if (s == NULL) {
        return (DsResult){DS_NULL_PTR, "串指针为空"};
    }

    for (i = 0; i < s->length; i++) {
        putchar(s->ch[i]);
    }
    putchar('\n');
    return (DsResult){DS_OK, "打印完成"};
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "位置/长度越界";
        default:             return "未知状态码";
    }
}
