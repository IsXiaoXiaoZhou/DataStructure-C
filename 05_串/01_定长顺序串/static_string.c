/**
 * @file static_string.c
 * @brief 定长顺序串（定长数组实现）接口实现
 *
 * 存储结构: data[0..length-1] 连续存放字符序列
 * 设计要点:
 *   1. 所有"整串产出"操作（assign/concat/substring/copy）先检查
 *      容量再写入，杜绝半途截断留下脏数据
 *   2. 长度域自管理，'\0' 不入串 —— 打印时按长度输出，不依赖终止符
 * 复杂度: 按位存取 O(1)；整串复制类操作 O(n)
 */

#include <stdio.h>

#include "static_string.h"


/* 内部辅助: 求 C 字符串长度（与 strlen 等价，避免额外依赖） */
static size_t str_len(const char *cstr)
{
    size_t n = 0;

    while (cstr[n] != '\0') {
        n++;
    }
    return n;
}

DsResult ss_assign(StaticString *s, const char *cstr)
{
    size_t n = 0;
    size_t i = 0;

    if (s == NULL || cstr == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    n = str_len(cstr);
    if (n > STR_MAX_SIZE) {
        /* 定长容量边界：宁可报错也不静默截断，让调用方感知 */
        return (DsResult){DS_OVERFLOW, "赋值超长，超出定长容量"};
    }

    for (i = 0; i < n; i++) {
        s->data[i] = cstr[i];
    }
    s->length = n;
    return (DsResult){DS_OK, "赋值成功"};
}

size_t ss_length(const StaticString *s)
{
    /* 前置条件: s 非 NULL；防御性返回 0 而非崩溃 */
    return (s == NULL) ? 0 : s->length;
}

int ss_is_empty(const StaticString *s)
{
    return (s == NULL) || (s->length == 0);
}

DsResult ss_clear(StaticString *s)
{
    if (s == NULL) {
        return (DsResult){DS_NULL_PTR, "串指针为空"};
    }

    /* 定长实现清空只需长度归零，残留数据不可达即无意义 */
    s->length = 0;
    return (DsResult){DS_OK, "清空成功"};
}

DsResult ss_copy(StaticString *dst, const StaticString *src)
{
    size_t i = 0;

    if (dst == NULL || src == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    for (i = 0; i < src->length; i++) {
        dst->data[i] = src->data[i];
    }
    dst->length = src->length;
    return (DsResult){DS_OK, "复制成功"};
}

DsResult ss_compare(const StaticString *a, const StaticString *b, int *result)
{
    size_t n = 0;
    size_t i = 0;

    if (a == NULL || b == NULL || result == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    /* 逐位比较，先比公共前缀，前缀全同再比长度（短者小） */
    n = (a->length < b->length) ? a->length : b->length;
    for (i = 0; i < n; i++) {
        if (a->data[i] != b->data[i]) {
            *result = (int)(unsigned char)a->data[i] - (int)(unsigned char)b->data[i];
            return (DsResult){DS_OK, "比较完成"};
        }
    }
    *result = (a->length < b->length) ? -1 : ((a->length > b->length) ? 1 : 0);
    return (DsResult){DS_OK, "比较完成"};
}

DsResult ss_concat(StaticString *dst, const StaticString *a, const StaticString *b)
{
    size_t i = 0;

    if (dst == NULL || a == NULL || b == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (a->length + b->length > STR_MAX_SIZE) {
        return (DsResult){DS_OVERFLOW, "连接后总长超出定长容量"};
    }

    /* 两段各自整块复制，a 在前 b 在后 */
    for (i = 0; i < a->length; i++) {
        dst->data[i] = a->data[i];
    }
    for (i = 0; i < b->length; i++) {
        dst->data[a->length + i] = b->data[i];
    }
    dst->length = a->length + b->length;
    return (DsResult){DS_OK, "连接成功"};
}

DsResult ss_substring(StaticString *out, const StaticString *src, size_t pos, size_t len)
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

    /* data 下标从 0 起，第 pos 个字符位于 data[pos-1] */
    for (i = 0; i < len; i++) {
        out->data[i] = src->data[pos - 1 + i];
    }
    out->length = len;
    return (DsResult){DS_OK, "求子串成功"};
}

DsResult ss_index(const StaticString *src, const StaticString *pattern, size_t *pos)
{
    size_t i = 0;
    size_t j = 0;

    if (src == NULL || pattern == NULL || pos == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    /* 空模式约定在位置 1 命中（与教材 Index(S, T, 1) 约定一致） */
    if (pattern->length == 0) {
        *pos = 1;
        return (DsResult){DS_OK, "空模式约定命中位置1"};
    }

    /* i 指向主串当前对齐起点，j 为已匹配位数计数器 */
    for (i = 0; i + pattern->length <= src->length; i++) {
        for (j = 0; j < pattern->length; j++) {
            if (src->data[i + j] != pattern->data[j]) {
                break;
            }
        }
        if (j == pattern->length) {
            *pos = i + 1;
            return (DsResult){DS_OK, "定位成功"};
        }
    }

    *pos = 0;
    return (DsResult){DS_ERROR, "未找到匹配位置"};
}

DsResult ss_print(const StaticString *s)
{
    size_t i = 0;

    if (s == NULL) {
        return (DsResult){DS_NULL_PTR, "串指针为空"};
    }

    /* 按长度输出而非依赖 '\0'，长度域才是串的权威信息 */
    for (i = 0; i < s->length; i++) {
        putchar(s->data[i]);
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
        case DS_OVERFLOW:    return "超出定长容量";
        default:             return "未知状态码";
    }
}
