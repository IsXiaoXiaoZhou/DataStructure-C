/**
 * @file hash_open.c
 * @brief 开放定址法散列表实现（线性探测 + 二次探测 + 墓碑删除）
 */

#include <stdlib.h>
#include <string.h>

#include "hash_open.h"


static int is_prime(int n)
{
    int i = 0;
    if (n < 2) return 0;
    if (n < 4) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    for (i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return 0;
    }
    return 1;
}

static int prev_prime(int n)
{
    while (n > 2 && !is_prime(n)) --n;
    return n;
}

static int hash_key(const HashOpen *ht, int key)
{
    int h = key % ht->p;
    if (h < 0) h += ht->p;
    return h;
}

static int probe(const HashOpen *ht, int key, int i)
{
    int h0 = hash_key(ht, key);
    if (ht->strategy == HO_LINEAR) {
        return (h0 + i) % ht->m;
    }
    /* 教材二次探测: di = 1^2, -1^2, 2^2, -2^2, ...（交替正负）
     * i=0 -> d=0; i=1,2 -> ±1; i=3,4 -> ±4; i=5,6 -> ±9 ...
     * 负偏移按 (H + di) mod m 计算，结果为负时加 m 转正 */
    {
        int k = (i + 1) / 2;              /* 第 k 个平方项 */
        int d = ((i % 2 == 1) ? 1 : -1) * k * k;
        int idx = (h0 + d) % ht->m;

        if (idx < 0) {
            idx += ht->m;
        }
        return idx;
    }
}

DsResult ho_init(HashOpen *ht, int m, HOStrategy strategy)
{
    int i = 0;

    if (ht == NULL) return (DsResult){DS_NULL_PTR, "散列表指针为空"};
    if (m <= 0) m = 13;

    ht->m = m;
    ht->p = prev_prime(m);
    if (ht->p < 2) ht->p = 2;
    ht->n = 0;
    ht->tomb = 0;
    ht->strategy = strategy;
    ht->table = (HOBucket *)malloc((size_t)m * sizeof(HOBucket));
    if (ht->table == NULL) return (DsResult){DS_OVERFLOW, "桶数组内存分配失败"};
    for (i = 0; i < m; ++i) {
        ht->table[i].key = 0;
        ht->table[i].state = HO_EMPTY;
    }
    return (DsResult){DS_OK, "散列表初始化成功"};
}

DsResult ho_insert(HashOpen *ht, int key)
{
    int first_tomb = -1;
    int i = 0;

    if (ht == NULL || ht->table == NULL) return (DsResult){DS_NULL_PTR, "散列表指针为空"};

    for (i = 0; i < ht->m; ++i) {
        int idx = probe(ht, key, i);

        if (ht->table[idx].state == HO_EMPTY) {
            int pos = first_tomb >= 0 ? first_tomb : idx;
            ht->table[pos].key = key;
            ht->table[pos].state = HO_OCCUPIED;
            ht->n++;
            if (first_tomb >= 0) ht->tomb--;
            return (DsResult){DS_OK, "插入成功"};
        }
        if (ht->table[idx].state == HO_TOMBSTONE) {
            if (first_tomb < 0) first_tomb = idx;
        }
        if (ht->table[idx].state == HO_OCCUPIED && ht->table[idx].key == key) {
            return (DsResult){DS_ERROR, "关键字已存在"};
        }
    }
    if (first_tomb >= 0) {
        ht->table[first_tomb].key = key;
        ht->table[first_tomb].state = HO_OCCUPIED;
        ht->n++;
        ht->tomb--;
        return (DsResult){DS_OK, "插入成功（复用墓碑位）"};
    }
    return (DsResult){DS_OVERFLOW, "散列表已满"};
}

DsResult ho_search(const HashOpen *ht, int key, int *cmp)
{
    int cnt = 0;
    int i = 0;

    if (ht == NULL || ht->table == NULL) return (DsResult){DS_NULL_PTR, "散列表指针为空"};

    for (i = 0; i < ht->m; ++i) {
        int idx = probe(ht, key, i);

        if (ht->table[idx].state == HO_EMPTY) {
            if (cmp != NULL) *cmp = cnt;
            return (DsResult){DS_NOT_FOUND, "未找到目标关键字"};
        }
        if (ht->table[idx].state == HO_OCCUPIED) {
            cnt++;
            if (ht->table[idx].key == key) {
                if (cmp != NULL) *cmp = cnt;
                return (DsResult){DS_OK, "查找成功"};
            }
        }
    }
    if (cmp != NULL) *cmp = cnt;
    return (DsResult){DS_NOT_FOUND, "未找到目标关键字"};
}

DsResult ho_delete(HashOpen *ht, int key)
{
    int i = 0;

    if (ht == NULL || ht->table == NULL) return (DsResult){DS_NULL_PTR, "散列表指针为空"};

    for (i = 0; i < ht->m; ++i) {
        int idx = probe(ht, key, i);

        if (ht->table[idx].state == HO_EMPTY) {
            return (DsResult){DS_NOT_FOUND, "未找到目标关键字"};
        }
        if (ht->table[idx].state == HO_OCCUPIED && ht->table[idx].key == key) {
            ht->table[idx].state = HO_TOMBSTONE;
            ht->n--;
            ht->tomb++;
            return (DsResult){DS_OK, "删除成功"};
        }
    }
    return (DsResult){DS_NOT_FOUND, "未找到目标关键字"};
}

double ho_load_factor(const HashOpen *ht)
{
    if (ht == NULL || ht->m <= 0) return 0.0;
    return (double)ht->n / (double)ht->m;
}

size_t ho_size(const HashOpen *ht)
{
    if (ht == NULL) return 0;
    return ht->n;
}

DsResult ho_destroy(HashOpen *ht)
{
    if (ht == NULL) return (DsResult){DS_NULL_PTR, "散列表指针为空"};
    free(ht->table);
    ht->table = NULL;
    ht->n = 0;
    ht->tomb = 0;
    ht->m = 0;
    ht->p = 0;
    return (DsResult){DS_OK, "散列表销毁成功"};
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "位置/下标越界";
        case DS_OVERFLOW:    return "散列表已满";
        case DS_EMPTY:       return "散列表为空";
        case DS_NOT_FOUND:   return "未找到目标关键字";
        default:             return "未知状态码";
    }
}
