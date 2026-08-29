/**
 * @file hash_perf.c
 * @brief 散列查找性能分析实现
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_perf.h"


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

/* ========== 拉链法 ASL 精确计算 ========== */

DsResult hp_chain_asl(const int keys[], size_t n, int m,
                      double *asl_s, double *asl_f)
{
    typedef struct CNode {
        int key;
        struct CNode *next;
    } CNode;

    CNode **table = NULL;
    int p = 0;
    size_t i = 0;
    double sum_success = 0.0;
    double sum_fail = 0.0;

    if (keys == NULL) return (DsResult){DS_NULL_PTR, "参数指针为空"};
    if (n == 0 || m <= 0) return (DsResult){DS_EMPTY, "关键字数或表长为0"};

    p = prev_prime(m);
    if (p < 2) p = 2;

    table = (CNode **)malloc((size_t)m * sizeof(CNode *));
    if (table == NULL) return (DsResult){DS_OVERFLOW, "内存分配失败"};
    memset(table, 0, (size_t)m * sizeof(CNode *));

    /* 插入所有关键字（头插法） */
    for (i = 0; i < n; ++i) {
        int h = keys[i] % p;
        CNode *nd = NULL;
        if (h < 0) h += p;
        nd = (CNode *)malloc(sizeof *nd);
        if (nd == NULL) {
            int j = 0;
            for (j = 0; j < m; ++j) {
                CNode *c = table[j];
                while (c) { CNode *t = c; c = c->next; free(t); }
            }
            free(table);
            return (DsResult){DS_OVERFLOW, "结点内存分配失败"};
        }
        nd->key = keys[i];
        nd->next = table[h];
        table[h] = nd;
    }

    /* 成功 ASL */
    {
        int j = 0;
        for (j = 0; j < m; ++j) {
            int pos = 1;
            CNode *c = table[j];
            while (c != NULL) {
                sum_success += (double)pos;
                pos++;
                c = c->next;
            }
        }
    }
    if (n > 0) sum_success /= (double)n;

    /* 不成功 ASL */
    {
        int j = 0;
        for (j = 0; j < m; ++j) {
            int len = 0;
            CNode *c = table[j];
            while (c) { len++; c = c->next; }
            sum_fail += (double)len;
        }
    }
    sum_fail /= (double)m;

    /* 清理 */
    {
        int j = 0;
        for (j = 0; j < m; ++j) {
            CNode *c = table[j];
            while (c) { CNode *t = c; c = c->next; free(t); }
        }
    }
    free(table);

    if (asl_s != NULL) *asl_s = sum_success;
    if (asl_f != NULL) *asl_f = sum_fail;
    return (DsResult){DS_OK, "拉链法ASL计算完成"};
}

/* ========== 开放定址法 ASL 精确计算 ========== */

DsResult hp_open_asl(const int keys[], size_t n, int m,
                     HPStrategy strategy, double *asl_s, double *asl_f)
{
    typedef enum { EMPTY = 0, OCCUPIED, TOMBSTONE } State;
    typedef struct { int key; State st; } Slot;

    Slot *table = NULL;
    int p = 0;
    size_t i = 0;
    double sum_s = 0.0;
    double sum_f = 0.0;

    if (keys == NULL) return (DsResult){DS_NULL_PTR, "参数指针为空"};
    if (n == 0 || m <= 0) return (DsResult){DS_EMPTY, "关键字数或表长为0"};
    if ((int)n > m) return (DsResult){DS_OVERFLOW, "关键字数超过表长"};

    p = prev_prime(m);
    if (p < 2) p = 2;

    table = (Slot *)malloc((size_t)m * sizeof(Slot));
    if (table == NULL) return (DsResult){DS_OVERFLOW, "内存分配失败"};
    {
        int j = 0;
        for (j = 0; j < m; ++j) {
            table[j].key = 0;
            table[j].st = EMPTY;
        }
    }

    /* 探测函数 */
    #define PROBE_H(key, step) \
        (strategy == HP_LINEAR ? (((key) % p + (step)) % m) \
                               : (((key) % p + (step) * (step)) % m))

    /* 插入所有关键字 */
    for (i = 0; i < n; ++i) {
        int k = keys[i];
        int step = 0;
        int first_tomb = -1;
        for (step = 0; step < m; ++step) {
            int idx = PROBE_H(k, step);
            if (table[idx].st == EMPTY) {
                int pos = first_tomb >= 0 ? first_tomb : idx;
                table[pos].key = k;
                table[pos].st = OCCUPIED;
                break;
            }
            if (table[idx].st == TOMBSTONE && first_tomb < 0) {
                first_tomb = idx;
            }
        }
    }

    /* 成功 ASL */
    {
        int total = 0;
        for (i = 0; i < n; ++i) {
            int k = keys[i];
            int step = 0;
            int cmp = 0;
            for (step = 0; step < m; ++step) {
                int idx = PROBE_H(k, step);
                if (table[idx].st == EMPTY) break;
                if (table[idx].st == OCCUPIED) {
                    cmp++;
                    if (table[idx].key == k) break;
                }
            }
            sum_s += (double)cmp;
            total++;
        }
        if (total > 0) sum_s /= (double)total;
    }

    /* 不成功 ASL */
    {
        int j = 0;
        for (j = 0; j < m; ++j) {
            int step = 0;
            int cmp = 0;
            for (step = 0; step < m; ++step) {
                int idx = (j + (strategy == HP_LINEAR ? step : step * step)) % m;
                if (table[idx].st == EMPTY) break;
                if (table[idx].st == OCCUPIED) {
                    cmp++;
                }
            }
            sum_f += (double)cmp;
        }
        sum_f /= (double)m;
    }

    #undef PROBE_H
    free(table);

    if (asl_s != NULL) *asl_s = sum_s;
    if (asl_f != NULL) *asl_f = sum_f;
    return (DsResult){DS_OK, "开放定址法ASL计算完成"};
}

/* ========== α 扫描实验 ========== */

DsResult hp_alpha_scan(const int keys[], size_t n, int m,
                       double alpha_start, double alpha_end, int steps)
{
    int i = 0;
    double step_size = 0.0;

    if (keys == NULL) return (DsResult){DS_NULL_PTR, "参数指针为空"};
    if (n == 0 || m <= 0 || steps <= 0) return (DsResult){DS_EMPTY, "参数无效"};
    if (alpha_start <= 0 || alpha_end <= 0 || alpha_start >= alpha_end)
        return (DsResult){DS_ERROR, "alpha范围无效"};

    step_size = (alpha_end - alpha_start) / (double)(steps - 1);

    printf("\n");
    printf("========== α 扫描实验 (m=%d) ==========\n", m);
    printf("%-6s | %-12s %-12s %-12s | %-12s %-12s | %-12s %-12s %-12s\n",
           "α", "链-成功(实)", "链-成功(理)", "链-不成功(实)",
           "链-不成功(理)", "线性-成功(实)", "线性-成功(理)",
           "线性-不成功(实)", "线性-不成功(理)");
    printf("-------+-----------------------------------------------------"
           "---------------------------------------------------\n");

    for (i = 0; i < steps; ++i) {
        double alpha = alpha_start + step_size * (double)i;
        int cnt = (int)(alpha * (double)m);
        double chain_s = 0, chain_f = 0;
        double lin_s = 0, lin_f = 0;
        double chain_s_theory = 0, chain_f_theory = 0;
        double lin_s_theory = 0, lin_f_theory = 0;

        if (cnt <= 0 || (size_t)cnt > n) continue;

        hp_chain_asl(keys, (size_t)cnt, m, &chain_s, &chain_f);
        chain_s_theory = 1.0 + alpha / 2.0;
        chain_f_theory = alpha;

        hp_open_asl(keys, (size_t)cnt, m, HP_LINEAR, &lin_s, &lin_f);
        if (alpha < 1.0) {
            lin_s_theory = (1.0 / alpha) * log(1.0 / (1.0 - alpha));
            lin_f_theory = 1.0 / (1.0 - alpha);
        } else {
            lin_s_theory = 0;
            lin_f_theory = 0;
        }

        printf("%-6.2f | %-12.4f %-12.4f %-12.4f | %-12.4f %-12.4f | %-12.4f %-12.4f %-12.4f\n",
               alpha, chain_s, chain_s_theory, chain_f,
               chain_f_theory, lin_s, lin_s_theory, lin_f,
               lin_f_theory);
    }

    return (DsResult){DS_OK, "α扫描实验完成"};
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:          return "操作成功";
        case DS_ERROR:       return "一般性错误";
        case DS_NULL_PTR:    return "空指针参数";
        case DS_OUT_OF_RANGE: return "位置/下标越界";
        case DS_OVERFLOW:    return "内存分配失败";
        case DS_EMPTY:       return "参数为空";
        case DS_NOT_FOUND:   return "查找未命中";
        default:             return "未知状态码";
    }
}
