/**
 * @file bin_search.c
 * @brief 折半查找 / 插值查找 / 斐波那契查找实现
 */

#include "bin_search.h"


DsResult bin_search(const int a[], size_t n, int key, size_t *pos)
{
    size_t low = 0;
    size_t high = 0;
    size_t mid = 0;

    if (a == NULL || pos == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (n == 0) {
        return (DsResult){DS_EMPTY, "查找表为空"};
    }

    high = n - 1;
    while (low <= high) {
        mid = low + (high - low) / 2; /* 防 low+high 溢出 */
        if (key == a[mid]) {
            *pos = mid;
            return (DsResult){DS_OK, "查找成功"};
        }
        if (key < a[mid]) {
            if (mid == 0) {
                break; /* mid=0 时 mid-1 会下溢，直接判负 */
            }
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }
    return (DsResult){DS_NOT_FOUND, "未找到目标元素"};
}

/* ---------- 插值查找 ---------- */

DsResult interp_search(const int a[], size_t n, int key, size_t *pos)
{
    size_t low = 0;
    size_t high = 0;
    size_t mid = 0;
    long long al = 0;
    long long ah = 0;

    if (a == NULL || pos == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (n == 0) {
        return (DsResult){DS_EMPTY, "查找表为空"};
    }

    high = n - 1;
    while (low <= high) {
        al = a[low];
        ah = a[high];
        if (al == ah) {            /* 区间全同值，只能整体比对 */
            if (key == al) {
                *pos = low;
                return (DsResult){DS_OK, "查找成功"};
            }
            break;
        }
        if (key < al || key > ah) {/* 值域预判，防比例出界与除零 */
            break;
        }
        /* mid 按 key 在 [al,ah] 中的比例插值定位 */
        mid = (size_t)((long long)low
                       + ((long long)(high - low) * (key - al))
                         / (ah - al));
        if (a[mid] == key) {
            *pos = mid;
            return (DsResult){DS_OK, "查找成功"};
        }
        if (key < a[mid]) {
            if (mid == 0) {
                break;
            }
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }
    return (DsResult){DS_NOT_FOUND, "未找到目标元素"};
}

/* ---------- 斐波那契查找 ---------- */

/* 第 k 个斐波那契数: F[0]=0, F[1]=1, F[2]=1, F[3]=2 ... */
static size_t fib_k(size_t k)
{
    size_t f0 = 0;
    size_t f1 = 1;
    size_t i = 0;

    if (k == 0) {
        return 0;
    }
    for (i = 2; i <= k; ++i) {
        size_t t = f0 + f1;
        f0 = f1;
        f1 = t;
    }
    return f1;
}

DsResult fib_search(const int a[], size_t n, int key, size_t *pos)
{
    size_t k = 1;        /* 最小 k: F[k] >= n */
    size_t low = 0;
    size_t high = 0;
    size_t mid = 0;
    size_t fkm1 = 0;

    if (a == NULL || pos == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (n == 0) {
        return (DsResult){DS_EMPTY, "查找表为空"};
    }

    while (fib_k(k) < n) {
        ++k;
    }
    high = fib_k(k) - 1; /* 虚拟上限: 表尾扩展至 F[k]，越界视为 +∞ */
    while (k > 0 && low <= high) {
        if (k == 1) {                   /* F[k-1]=F[0]=0，mid 公式失效 */
            if (a[low] == key) {
                *pos = low;
                return (DsResult){DS_OK, "查找成功"};
            }
            break;
        }
        fkm1 = fib_k(k - 1);
        mid = low + fkm1 - 1;
        if (mid >= n || key < a[mid]) { /* 越界位=+∞，key 必小于 → 左半 */
            if (mid == 0) {             /* key < a[0]，无候选 */
                break;
            }
            high = mid - 1;
            k -= 1;
        } else if (key > a[mid]) {
            low = mid + 1;
            k -= 2;                     /* k>=2 才到此分支，安全 */
        } else {
            *pos = mid;
            return (DsResult){DS_OK, "查找成功"};
        }
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
        case DS_OVERFLOW:    return "空间已满或内存分配失败";
        case DS_EMPTY:       return "查找表为空";
        case DS_NOT_FOUND:   return "未找到目标元素";
        default:             return "未知状态码";
    }
}
