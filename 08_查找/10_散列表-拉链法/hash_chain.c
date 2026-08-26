/**
 * @file hash_chain.c
 * @brief 拉链法散列表实现
 */

#include <stdlib.h>
#include <string.h>

#include "hash_chain.h"


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

static int hash_key(const HashChain *ht, int key)
{
    int h = key % ht->p;
    if (h < 0) h += ht->p;
    return h;
}

DsResult hc_init(HashChain *ht, int m)
{
    if (ht == NULL) return (DsResult){DS_NULL_PTR, "散列表指针为空"};
    if (m <= 0) m = 13;

    ht->m = m;
    ht->p = prev_prime(m);
    if (ht->p < 2) ht->p = 2;
    ht->n = 0;
    ht->table = (HCNode **)malloc((size_t)m * sizeof(HCNode *));
    if (ht->table == NULL) return (DsResult){DS_OVERFLOW, "桶数组内存分配失败"};
    memset(ht->table, 0, (size_t)m * sizeof(HCNode *));
    return (DsResult){DS_OK, "散列表初始化成功"};
}

DsResult hc_insert(HashChain *ht, int key)
{
    int idx = 0;
    HCNode *cur = NULL;
    HCNode *nd = NULL;

    if (ht == NULL || ht->table == NULL) return (DsResult){DS_NULL_PTR, "散列表指针为空"};

    idx = hash_key(ht, key);

    cur = ht->table[idx];
    while (cur != NULL) {
        if (cur->key == key) return (DsResult){DS_ERROR, "关键字已存在"};
        cur = cur->next;
    }

    nd = (HCNode *)malloc(sizeof *nd);
    if (nd == NULL) return (DsResult){DS_OVERFLOW, "结点内存分配失败"};
    nd->key = key;
    nd->next = ht->table[idx];
    ht->table[idx] = nd;
    ht->n++;
    return (DsResult){DS_OK, "插入成功"};
}

DsResult hc_search(const HashChain *ht, int key, int *cmp)
{
    int idx = 0;
    int cnt = 0;
    HCNode *cur = NULL;

    if (ht == NULL || ht->table == NULL) return (DsResult){DS_NULL_PTR, "散列表指针为空"};

    idx = hash_key(ht, key);
    cur = ht->table[idx];
    while (cur != NULL) {
        cnt++;
        if (cur->key == key) {
            if (cmp != NULL) *cmp = cnt;
            return (DsResult){DS_OK, "查找成功"};
        }
        cur = cur->next;
    }
    if (cmp != NULL) *cmp = cnt;
    return (DsResult){DS_ERROR, "未找到目标关键字"};
}

DsResult hc_delete(HashChain *ht, int key)
{
    int idx = 0;
    HCNode *cur = NULL;
    HCNode *prev = NULL;

    if (ht == NULL || ht->table == NULL) return (DsResult){DS_NULL_PTR, "散列表指针为空"};

    idx = hash_key(ht, key);
    cur = ht->table[idx];
    while (cur != NULL) {
        if (cur->key == key) {
            if (prev == NULL) {
                ht->table[idx] = cur->next;
            } else {
                prev->next = cur->next;
            }
            free(cur);
            ht->n--;
            return (DsResult){DS_OK, "删除成功"};
        }
        prev = cur;
        cur = cur->next;
    }
    return (DsResult){DS_ERROR, "未找到目标关键字"};
}

double hc_load_factor(const HashChain *ht)
{
    if (ht == NULL || ht->m <= 0) return 0.0;
    return (double)ht->n / (double)ht->m;
}

size_t hc_size(const HashChain *ht)
{
    if (ht == NULL) return 0;
    return ht->n;
}

DsResult hc_destroy(HashChain *ht)
{
    int i = 0;
    if (ht == NULL) return (DsResult){DS_NULL_PTR, "散列表指针为空"};
    if (ht->table == NULL) return (DsResult){DS_OK, "已是空表"};

    for (i = 0; i < ht->m; ++i) {
        HCNode *cur = ht->table[i];
        while (cur != NULL) {
            HCNode *tmp = cur;
            cur = cur->next;
            free(tmp);
        }
    }
    free(ht->table);
    ht->table = NULL;
    ht->n = 0;
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
        case DS_OVERFLOW:    return "内存分配失败";
        case DS_EMPTY:       return "散列表为空";
        default:             return "未知状态码";
    }
}
