#ifndef BLK_SEARCH_H
#define BLK_SEARCH_H

/**
 * @file blk_search.h
 * @brief 分块查找（索引顺序查找）接口定义
 *
 * 结构: 数据块数组 + 索引表
 *   - 块间有序: 第 i+1 块所有元素 > 第 i 块所有元素（块最大关键字升序）
 *   - 块内无序: 块内允许任意排列
 * 查找两阶段:
 *   1. 索引表定块: 因 max_key 升序可采用折半
 *   2. 块内查找:   顺序遍历（块小，顺序即可，无需排序开销）
 */

#include <stddef.h>

/* 七值状态码（与全工程逐字一致） */
typedef enum {
    DS_OK = 0,          /* 操作成功 */
    DS_ERROR,           /* 一般性错误（未命中） */
    DS_NULL_PTR,        /* 空指针参数 */
    DS_OUT_OF_RANGE,    /* 位置/下标越界（含 block_size=0） */
    DS_OVERFLOW,        /* 索引表容量不足 */
    DS_EMPTY,           // 对空结构取元素
    DS_NOT_FOUND        // 查找未命中（业务正常结果，非异常）
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 分块索引项 */
typedef struct {
    int    max_key; /* 块内最大关键字（块间按此升序） */
    size_t start;   /* 块首在数据数组中的下标 */
    size_t len;     /* 块内元素个数 */
} BlockIndex;

DsResult blk_build_index(const int a[], size_t n, size_t block_size,
                         BlockIndex *idx, size_t idx_cap, size_t *nblocks);
DsResult blk_search(const int a[], const BlockIndex idx[], size_t nb,
                    int key, size_t *pos, size_t *cmp);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* BLK_SEARCH_H */
