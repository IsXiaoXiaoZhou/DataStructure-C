#ifndef HASH_CHAIN_H
#define HASH_CHAIN_H

/**
 * @file hash_chain.h
 * @brief 拉链法散列表接口定义
 *
 * 散列表 + 链地址法（separate chaining）:
 *   - 桶数组 table[0..m-1]，每个桶是一条单链表
 *   - 散列函数: 除留余数法 H(key) = key % p (p 为 <= m 的素数)
 *   - 插入: 头插法 O(1)，不插重复键
 *   - 查找: 沿链遍历，成功/不成功比较次数可精确统计
 *   - 删除: 定位后摘链释放
 * 复杂度: 插入/查找/删除 均 O(1 + α)（期望）
 */

#include <stddef.h>

/* 六值状态码（与全工程逐字一致） */
typedef enum {
    DS_OK = 0,
    DS_ERROR,
    DS_NULL_PTR,
    DS_OUT_OF_RANGE,
    DS_OVERFLOW,
    DS_EMPTY,
    DS_NOT_FOUND        // 查找未命中（业务正常结果，非异常）
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

typedef struct HCNode {
    int             key;
    struct HCNode  *next;
} HCNode;

typedef struct {
    HCNode **table;     /* 桶数组，table[i] 为链表头 */
    int      m;         /* 桶数 */
    int      p;         /* 除留余数用的素数 (<= m) */
    size_t   n;         /* 当前关键字总数 */
} HashChain;

DsResult hc_init(HashChain *ht, int m);
DsResult hc_insert(HashChain *ht, int key);
DsResult hc_search(const HashChain *ht, int key, int *cmp);
DsResult hc_delete(HashChain *ht, int key);
double hc_load_factor(const HashChain *ht);
size_t hc_size(const HashChain *ht);
DsResult hc_destroy(HashChain *ht);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* HASH_CHAIN_H */
