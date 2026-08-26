/**
 * @file block_string.c
 * @brief 块链串（链式存储的串）接口实现
 *
 * 存储结构: BlockNode 链表，尾块不满时有效槽数 = length % CHUNK_SIZE
 * 设计要点:
 *   1. 核心"逻辑下标 -> 物理槽位"换算: 第 i 个字符（i 从 0 起）
 *      位于第 i/CHUNK_SIZE 块的第 i%CHUNK_SIZE 槽
 *   2. append_char 内部辅助统一"尾插一字符"：尾块满则开新块，
 *      assign/concat/substring 全部复用，避免三处重复块管理逻辑
 * 复杂度: 追加均摊 O(1)；取字符 O(块数+块内)
 */

#include <stdio.h>
#include <stdlib.h>

#include "block_string.h"


/* 内部辅助: 求 C 字符串长度 */
static size_t str_len(const char *cstr)
{
    size_t n = 0;

    while (cstr[n] != '\0') {
        n++;
    }
    return n;
}

/* 内部辅助: 向 s 尾部追加一字符（尾块满则开新块），失败返回 DS_ERROR */
static DsStatus append_char(BlockString *s, char c)
{
    size_t used = s->length % CHUNK_SIZE;

    if (s->head == NULL || used == 0) {
        /* 空串或尾块已满：开新块。注意 length>0 且 used==0 恰表明尾块满 */
        BlockNode *node = (BlockNode *)malloc(sizeof(BlockNode));

        if (node == NULL) {
            return DS_ERROR;
        }
        node->next = NULL;
        if (s->head == NULL) {
            s->head = node; /* 首块 */
        } else {
            s->tail->next = node;
        }
        s->tail = node;
    }
    s->tail->ch[used] = c;
    s->length++;
    return DS_OK;
}

DsResult bs_init(BlockString *s)
{
    if (s == NULL) {
        return (DsResult){DS_NULL_PTR, "串指针为空"};
    }

    s->head = NULL;
    s->tail = NULL;
    s->length = 0;
    return (DsResult){DS_OK, "初始化成功"};
}

DsResult bs_destroy(BlockString *s)
{
    BlockNode *p = NULL;

    if (s == NULL) {
        return (DsResult){DS_NULL_PTR, "串指针为空"};
    }

    /* 标准单链表逐结点释放：先记后继再 free，防止读已释放内存 */
    while (s->head != NULL) {
        p = s->head;
        s->head = s->head->next;
        free(p);
    }
    s->tail = NULL;
    s->length = 0;
    return (DsResult){DS_OK, "销毁成功"};
}

DsResult bs_assign(BlockString *s, const char *cstr)
{
    size_t n = 0;
    size_t i = 0;

    if (s == NULL || cstr == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    /* 先释放旧内容再填充，失败时 s 为空串（不残留半截数据） */
    bs_clear(s);

    n = str_len(cstr);
    for (i = 0; i < n; i++) {
        if (append_char(s, cstr[i]) != DS_OK) {
            bs_clear(s);
            return (DsResult){DS_ERROR, "结块分配失败"};
        }
    }
    return (DsResult){DS_OK, "赋值成功"};
}

DsResult bs_copy(BlockString *dst, const BlockString *src)
{
    const BlockNode *p = NULL;
    size_t i = 0;

    if (dst == NULL || src == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    bs_clear(dst);

    /* 逐块逐槽读源串，经 append_char 重建块结构（不共享结点） */
    for (p = src->head; p != NULL; p = p->next) {
        for (i = 0; i < CHUNK_SIZE; i++) {
            if (dst->length == src->length) {
                return (DsResult){DS_OK, "复制成功"}; /* 源串尾块无效槽到此为止 */
            }
            if (append_char(dst, p->ch[i]) != DS_OK) {
                bs_clear(dst);
                return (DsResult){DS_ERROR, "结块分配失败"};
            }
        }
    }
    return (DsResult){DS_OK, "复制成功"};
}

size_t bs_length(const BlockString *s)
{
    return (s == NULL) ? 0 : s->length;
}

int bs_is_empty(const BlockString *s)
{
    return (s == NULL) || (s->length == 0);
}

DsResult bs_get(const BlockString *s, size_t pos, char *value)
{
    const BlockNode *p = NULL;
    size_t block_idx = 0;
    size_t slot_idx = 0;

    if (s == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (pos < 1 || pos > s->length) {
        return (DsResult){DS_OUT_OF_RANGE, "位置越界，合法范围 [1, length]"};
    }

    /* 逻辑下标换算: 第 pos 个字符 -> (pos-1)/CHUNK_SIZE 块 (pos-1)%CHUNK_SIZE 槽 */
    block_idx = (pos - 1) / CHUNK_SIZE;
    slot_idx = (pos - 1) % CHUNK_SIZE;

    p = s->head;
    while (block_idx-- > 0) {
        p = p->next;
    }
    *value = p->ch[slot_idx];
    return (DsResult){DS_OK, "取字符成功"};
}

DsResult bs_concat(BlockString *dst, const BlockString *a, const BlockString *b)
{
    size_t i = 0;

    if (dst == NULL || a == NULL || b == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    bs_clear(dst);

    /* 逐字符追加：a、b 结点不共享，dst 独立成链 */
    for (i = 1; i <= a->length; i++) {
        char c = 0;

        if (bs_get(a, i, &c).code != DS_OK || append_char(dst, c) != DS_OK) {
            bs_clear(dst);
            return (DsResult){DS_ERROR, "连接失败"};
        }
    }
    for (i = 1; i <= b->length; i++) {
        char c = 0;

        if (bs_get(b, i, &c).code != DS_OK || append_char(dst, c) != DS_OK) {
            bs_clear(dst);
            return (DsResult){DS_ERROR, "连接失败"};
        }
    }
    return (DsResult){DS_OK, "连接成功"};
}

DsResult bs_substring(BlockString *out, const BlockString *src, size_t pos, size_t len)
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

    bs_clear(out);

    for (i = 0; i < len; i++) {
        char c = 0;

        if (bs_get(src, pos + i, &c).code != DS_OK || append_char(out, c) != DS_OK) {
            bs_clear(out);
            return (DsResult){DS_ERROR, "求子串失败"};
        }
    }
    return (DsResult){DS_OK, "求子串成功"};
}

DsResult bs_compare(const BlockString *a, const BlockString *b, int *result)
{
    size_t n = 0;
    size_t i = 0;

    if (a == NULL || b == NULL || result == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    n = (a->length < b->length) ? a->length : b->length;
    for (i = 1; i <= n; i++) {
        char ca = 0;
        char cb = 0;

        bs_get(a, i, &ca);
        bs_get(b, i, &cb);
        if (ca != cb) {
            *result = (int)(unsigned char)ca - (int)(unsigned char)cb;
            return (DsResult){DS_OK, "比较完成"};
        }
    }
    *result = (a->length < b->length) ? -1 : ((a->length > b->length) ? 1 : 0);
    return (DsResult){DS_OK, "比较完成"};
}

DsResult bs_clear(BlockString *s)
{
    if (s == NULL) {
        return (DsResult){DS_NULL_PTR, "串指针为空"};
    }

    return bs_destroy(s); /* 链式存储无法保留结块复用，clear 即 destroy */
}

DsResult bs_print(const BlockString *s)
{
    const BlockNode *p = NULL;
    size_t remain = 0;
    size_t take = 0;

    if (s == NULL) {
        return (DsResult){DS_NULL_PTR, "串指针为空"};
    }

    remain = s->length;
    for (p = s->head; p != NULL; p = p->next) {
        take = (remain < CHUNK_SIZE) ? remain : CHUNK_SIZE; /* 尾块有效槽数 */
        {
            size_t i = 0;

            for (i = 0; i < take; i++) {
                putchar(p->ch[i]);
            }
        }
        remain -= take;
        if (p->next != NULL) {
            putchar('|'); /* 块边界示意 */
        }
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
        case DS_OVERFLOW:    return "无符号减法下溢";
        default:             return "未知状态码";
    }
}
