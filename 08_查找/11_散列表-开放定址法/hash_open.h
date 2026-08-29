#ifndef HASH_OPEN_H
#define HASH_OPEN_H

/**
 * @file hash_open.h
 * @brief 开放定址法散列表接口定义
 *
 * 散列表 + 开放定址法（open addressing）:
 *   - 所有元素存在桶数组内，冲突时按探测序列寻找下一个空位
 *   - 本模块支持两种探测策略:
 *     1. 线性探测: H_i = (H(key) + i) % m
 *     2. 二次探测: H_i = (H(key) + di) % m，
 *        di = 1^2, -1^2, 2^2, -2^2, ...（交替正负，
 *        负偏移结果为负时加 m 的倍数转正）
 *   - 删除: 用"墓碑"（tombstone）标记已删位置
 * 复杂度: 插入/查找/删除 均 O(1/(1-α))（期望），α 为装填因子
 */

#include <stddef.h>

/* 七值状态码（与全工程逐字一致） */
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

/* 桶状态: 空 / 有效 / 墓碑（已删除） */
typedef enum {
    HO_EMPTY = 0,
    HO_OCCUPIED,
    HO_TOMBSTONE
} HOState;

typedef struct {
    int      key;
    HOState  state;
} HOBucket;

/* 探测策略 */
typedef enum {
    HO_LINEAR = 0,      /* 线性探测 */
    HO_QUADRATIC        /* 二次探测 */
} HOStrategy;

typedef struct {
    HOBucket   *table;      /* 桶数组 */
    int         m;          /* 桶数 */
    int         p;          /* 除留余数用的素数 */
    size_t      n;          /* 有效关键字数 */
    size_t      tomb;       /* 墓碑数 */
    HOStrategy  strategy;   /* 探测策略 */
} HashOpen;

DsResult ho_init(HashOpen *ht, int m, HOStrategy strategy);
DsResult ho_insert(HashOpen *ht, int key);
DsResult ho_search(const HashOpen *ht, int key, int *cmp);
DsResult ho_delete(HashOpen *ht, int key);
double ho_load_factor(const HashOpen *ht);
size_t ho_size(const HashOpen *ht);
DsResult ho_destroy(HashOpen *ht);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* HASH_OPEN_H */
