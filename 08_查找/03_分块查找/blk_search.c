/**
 * @file blk_search.c
 * @brief 分块查找实现（索引折半 + 块内顺序）
 */

#include "blk_search.h"


DsResult blk_build_index(const int a[], size_t n, size_t block_size,
                         BlockIndex *idx, size_t idx_cap, size_t *nblocks)
{
    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    if (a == NULL || idx == NULL || nblocks == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (n == 0) {
        return (DsResult){DS_EMPTY, "数据数组为空"};
    }
    if (block_size == 0) {
        return (DsResult){DS_OUT_OF_RANGE, "块大小须大于0"};
    }

    for (i = 0; i < n; i += block_size) {
        if (k >= idx_cap) {
            return (DsResult){DS_OVERFLOW, "索引表容量不足"};
        }
        idx[k].start = i;
        idx[k].len = (n - i < block_size) ? (n - i) : block_size;
        idx[k].max_key = a[i];
        for (j = i + 1; j < i + idx[k].len; ++j) {
            if (a[j] > idx[k].max_key) {
                idx[k].max_key = a[j];
            }
        }
        ++k;
    }
    *nblocks = k;
    return (DsResult){DS_OK, "索引构建成功"};
}

DsResult blk_search(const int a[], const BlockIndex idx[], size_t nb,
                    int key, size_t *pos, size_t *cmp)
{
    size_t lo = 0;
    size_t hi = 0;
    size_t first = 0;
    size_t m = 0;
    size_t i = 0;
    size_t cc = 0;

    if (a == NULL || idx == NULL || pos == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (nb == 0) {
        return (DsResult){DS_EMPTY, "索引表为空"};
    }

    /* 阶段1: 索引折半, 找第一个 max_key >= key 的块（下界定位） */
    hi = nb - 1;
    first = nb;
    while (lo <= hi) {
        m = lo + (hi - lo) / 2;
        ++cc;
        if (idx[m].max_key < key) {
            lo = m + 1;
        } else {
            first = m;
            if (m == 0) {
                break;
            }
            hi = m - 1;
        }
    }
    if (first == nb || idx[first].max_key < key) {
        if (cmp != NULL) {
            *cmp = cc;
        }
        return (DsResult){DS_NOT_FOUND, "未找到目标元素"};
    }

    /* 阶段2: 块内顺序查找 */
    for (i = 0; i < idx[first].len; ++i) {
        ++cc;
        if (a[idx[first].start + i] == key) {
            *pos = idx[first].start + i;
            if (cmp != NULL) {
                *cmp = cc;
            }
            return (DsResult){DS_OK, "查找成功"};
        }
    }

    if (cmp != NULL) {
        *cmp = cc;
    }
    return (DsResult){DS_NOT_FOUND, "未找到目标元素"};
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "位置/下标越界";
        case DS_OVERFLOW:    return "索引表容量不足";
        case DS_EMPTY:       return "查找表为空";
        case DS_NOT_FOUND:   return "未找到目标元素";
        default:             return "未知状态码";
    }
}
