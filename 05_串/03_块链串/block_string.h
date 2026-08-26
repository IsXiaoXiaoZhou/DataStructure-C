#ifndef BLOCK_STRING_H
#define BLOCK_STRING_H

/**
 * @file block_string.h
 * @brief 块链串（链式存储的串）接口定义
 *
 * 存储结构: CHUNK_SIZE 大小的结块串成的单链表，尾块允许不满
 *           [a b c d] -> [e f # #] -> NULL, length = 6
 * 设计要点:
 *   1. 结块增大存储密度（每结点存多字符而非单字符），
 *      CHUNK_SIZE 是"存储密度 vs 结点开销"的权衡参数
 *   2. 头尾双指针 + 长度域：尾指针使尾插 O(1)
 *   3. '#' 仅是演示中未满尾块的示意填充，本实现以
 *      length 与块内有效位数管理有效性，不写入填充符
 * 复杂度: 按序遍历 O(n)；定位第 i 块 O(n/CHUNK_SIZE)
 */

#include <stddef.h>

/* 状态码（按本模块所需裁剪，语义见各枚举项注释） */
typedef enum {
    DS_OK = 0,       /* 操作成功 */
    DS_ERROR,        /* 一般性错误（定位未命中） */
    DS_NULL_PTR,     /* 空指针参数 */
    DS_OUT_OF_RANGE, /* 位置/长度越界 */
    DS_OVERFLOW      /* 无符号减法下溢保护（本模块防御性保留） */
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 结块大小：每个链表结点存放的字符数 */
#define CHUNK_SIZE 4

/* 结块：CHUNK_SIZE 个字符 + 后继指针 */
typedef struct BlockNode {
    char ch[CHUNK_SIZE];      /* 字符槽位，无效槽不读 */
    struct BlockNode *next;   /* 下一结块 */
} BlockNode;

/* 块链串：头尾指针 + 总长度 */
typedef struct {
    BlockNode *head;  /* 首结块，空串为 NULL */
    BlockNode *tail;  /* 尾结块（尾插 O(1)），空串为 NULL */
    size_t length;    /* 有效字符总数 */
} BlockString;

DsResult bs_init(BlockString *s);
DsResult bs_destroy(BlockString *s);
DsResult bs_assign(BlockString *s, const char *cstr);
DsResult bs_copy(BlockString *dst, const BlockString *src);
size_t bs_length(const BlockString *s);
int bs_is_empty(const BlockString *s);
DsResult bs_get(const BlockString *s, size_t pos, char *value);
DsResult bs_concat(BlockString *dst, const BlockString *a, const BlockString *b);
DsResult bs_substring(BlockString *out, const BlockString *src, size_t pos, size_t len);
DsResult bs_compare(const BlockString *a, const BlockString *b, int *result);
DsResult bs_clear(BlockString *s);
DsResult bs_print(const BlockString *s);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* BLOCK_STRING_H */
