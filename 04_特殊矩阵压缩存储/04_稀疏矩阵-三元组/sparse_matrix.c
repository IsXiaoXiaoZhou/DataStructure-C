/**
 * @file sparse_matrix.c
 * @brief 稀疏矩阵（三元组顺序表）实现
 *
 * 核心约定: 三元组按行序为主序（行号增、行内列号增），
 * 普通转置与快速转置都建立在这一点上。
 */

#include "sparse_matrix.h"

#include <stdio.h>
#include <stdlib.h>


struct SparseMatrix {
    size_t rows;      /* 行数 mu */
    size_t cols;      /* 列数 nu */
    size_t count;     /* 非零元个数 tu */
    Triple *triples;  /* 按行序为主序排列的三元组数组 */
};

/*
 * 在有序三元组表中定位 (i,j)。
 * 找到返回 1 并由 *pos 带出下标；行号一旦越过 i 即提前终止
 * （后面的元素行号只可能更大，再扫必然扑空）
 */
static int find_triple(const SparseMatrix *m, size_t i, size_t j, size_t *pos)
{
    size_t p;

    for (p = 0; p < m->count; p++) {
        if ((size_t)m->triples[p].row == i &&
            (size_t)m->triples[p].col == j) {
            *pos = p;
            return 1;
        }
        if ((size_t)m->triples[p].row > i) {
            break;
        }
    }
    return 0;
}

/*
 * 转置结果的骨架分配: rows/cols 互换、tu 不变。
 * 两个转置函数共用，避免结构复制的重复代码
 */
static DsStatus transpose_skeleton(const SparseMatrix *src, SparseMatrix **dst)
{
    SparseMatrix *out = NULL;

    *dst = NULL;
    out = malloc(sizeof *out);
    if (out == NULL) {
        return DS_OVERFLOW;
    }
    out->rows = src->cols;
    out->cols = src->rows;
    out->count = src->count;
    /* count 可能为 0（全零矩阵），至少分配 1 个避免 malloc(0) 的实现定义行为 */
    out->triples = malloc((out->count > 0 ? out->count : 1)
                          * sizeof *out->triples);
    if (out->triples == NULL) {
        free(out);
        return DS_OVERFLOW;
    }
    *dst = out;
    return DS_OK;
}

DsResult matrix_create(SparseMatrix **m, size_t rows, size_t cols,
                       const int src[rows][cols])
{
    SparseMatrix *mat = NULL;
    size_t i;
    size_t j;
    size_t count = 0;
    size_t p = 0;

    if (m == NULL || src == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (rows < 1 || cols < 1) {
        return (DsResult){DS_OUT_OF_RANGE, "行列数须 >= 1"};
    }

    /* 第一遍只数非零元，第二遍再填充 —— 两遍都是行序扫描 */
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            if (src[i][j] != 0) {
                count++;
            }
        }
    }

    mat = malloc(sizeof *mat);
    if (mat == NULL) {
        return (DsResult){DS_OVERFLOW, "矩阵结构体内存分配失败"};
    }
    mat->rows = rows;
    mat->cols = cols;
    mat->count = count;
    /* 全零矩阵也至少分配 1 个单元，规避 malloc(0) 的实现定义行为 */
    mat->triples = malloc((count > 0 ? count : 1) * sizeof *mat->triples);
    if (mat->triples == NULL) {
        free(mat);
        return (DsResult){DS_OVERFLOW, "三元组数组内存分配失败"};
    }

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            if (src[i][j] != 0) {
                mat->triples[p].row = (int)(i + 1);
                mat->triples[p].col = (int)(j + 1);
                mat->triples[p].value = src[i][j];
                p++;
            }
        }
    }

    *m = mat;
    return (DsResult){DS_OK, "稀疏矩阵创建成功"};
}

DsResult matrix_transpose(const SparseMatrix *src, SparseMatrix **dst)
{
    SparseMatrix *out = NULL;
    size_t col;
    size_t p;
    size_t q = 0;
    DsStatus status;

    if (src == NULL || dst == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    /*
     * 别名防御: transpose(m, &m) 会让 *dst（即 m 本身）被新矩阵覆盖，
     * 旧矩阵指针就此丢失导致内存泄漏 —— 入口直接拒绝
     */
    if (src == *dst) {
        return (DsResult){DS_ERROR, "不允许别名调用 transpose(m,&m)"};
    }

    if ((status = transpose_skeleton(src, &out)) != DS_OK) {
        return (DsResult){status, "转置骨架分配失败"};
    }

    /*
     * 普通转置（教材经典算法）:
     * 转置矩阵按行序 = 原矩阵按列序。于是对原矩阵的每一列 col
     * （即转置后的第 col 行）从头到尾扫一遍三元组表，遇列号为
     * col 的依次放入结果表。每列都要扫全表，共 nu * tu 次 ——
     * 算法简单直白，但列数、非零元一多就平方级爆炸，
     * 这正是快速转置要解决的问题
     */
    for (col = 1; col <= src->cols; col++) {
        for (p = 0; p < src->count; p++) {
            if ((size_t)src->triples[p].col == col) {
                out->triples[q].row = src->triples[p].col;
                out->triples[q].col = src->triples[p].row;
                out->triples[q].value = src->triples[p].value;
                q++;
            }
        }
    }

    *dst = out;
    return (DsResult){DS_OK, "普通转置成功"};
}

DsResult matrix_fast_transpose(const SparseMatrix *src, SparseMatrix **dst)
{
    SparseMatrix *out = NULL;
    size_t *num = NULL;    /* num[c]: 原矩阵第 c+1 列的非零元个数 */
    size_t *cpot = NULL;   /* cpot[c]: 该列首元在结果表中的下标 */
    size_t c;
    size_t p;
    DsStatus status;

    if (src == NULL || dst == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    /*
     * 别名防御: fast_transpose(m, &m) 会让 *dst（即 m 本身）被新矩阵
     * 覆盖，旧矩阵指针就此丢失导致内存泄漏 —— 入口直接拒绝
     */
    if (src == *dst) {
        return (DsResult){DS_ERROR, "不允许别名调用 fast_transpose(m,&m)"};
    }

    if ((status = transpose_skeleton(src, &out)) != DS_OK) {
        return (DsResult){status, "转置骨架分配失败"};
    }

    num = calloc(src->cols, sizeof *num);
    cpot = malloc(src->cols * sizeof *cpot);
    if (num == NULL || cpot == NULL) {
        free(num);
        free(cpot);
        free(out->triples);
        free(out);
        return (DsResult){DS_OVERFLOW, "辅助数组内存分配失败"};
    }

    /*
     * 快速转置（教材经典算法）:
     * 第一遍扫描原表统计每列非零元个数 num[c];
     * cpot[c] = 前 c 列非零元总数之和，即第 c+1 列首元应落
     * 在结果表中的下标（cpot[0] = 0，cpot[c] = cpot[c-1] + num[c-1]）。
     * 第二遍扫描原表，每个三元组查 cpot 直接落位，落一个该列
     * 的 cpot 自增一次为同列下一个元素让位。
     * 两遍扫描共 O(nu + tu)，代价仅两个辅助数组 ——
     * 用空间与预处理换取扫描次数从 nu 次降到 1 次
     */
    for (p = 0; p < src->count; p++) {
        num[src->triples[p].col - 1]++;
    }
    cpot[0] = 0;
    for (c = 1; c < src->cols; c++) {
        cpot[c] = cpot[c - 1] + num[c - 1];
    }
    for (p = 0; p < src->count; p++) {
        size_t q = cpot[src->triples[p].col - 1]++;
        out->triples[q].row = src->triples[p].col;
        out->triples[q].col = src->triples[p].row;
        out->triples[q].value = src->triples[p].value;
    }

    free(num);
    free(cpot);
    *dst = out;
    return (DsResult){DS_OK, "快速转置成功"};
}

DsResult matrix_add(const SparseMatrix *a, const SparseMatrix *b,
                    SparseMatrix **result)
{
    SparseMatrix *out = NULL;
    size_t pa = 0;
    size_t pb = 0;
    size_t q = 0;

    if (a == NULL || b == NULL || result == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    /* 矩阵加法的前提: 形状完全相同 */
    if (a->rows != b->rows || a->cols != b->cols) {
        return (DsResult){DS_ERROR, "矩阵行列数不等，无法相加"};
    }

    out = malloc(sizeof *out);
    if (out == NULL) {
        return (DsResult){DS_OVERFLOW, "矩阵结构体内存分配失败"};
    }
    out->rows = a->rows;
    out->cols = a->cols;
    /*
     * 上界 tu_a+tu_b: 归并结果最多这么多个非零元（相加抵消只会更少）。
     * 两加数全零时至少分配 1 个，规避 malloc(0) 的实现定义行为
     */
    out->triples = malloc((a->count + b->count > 0 ? a->count + b->count : 1)
                          * sizeof *out->triples);
    if (out->triples == NULL) {
        free(out);
        return (DsResult){DS_OVERFLOW, "三元组数组内存分配失败"};
    }

    /*
     * 行序主序双指针归并（教材经典算法）:
     * pa、pb 分别是 a、b 两张有序表的游标，比较当前两个三元组的
     * (行,列) —— 序小者独占该位置，原样抄入结果；
     * 相等者同位置相加，和非零才存入，为零则双双跳过。
     * 经典考点: 5+(-5)=0 不产生三元组 —— 结果矩阵该位置是零元，
     * 零元在稀疏矩阵存储中本就没有席位，直接跳过即可，
     * 这也是"和的非零元个数可能少于任一加数"的根源
     */
    while (pa < a->count && pb < b->count) {
        if (a->triples[pa].row < b->triples[pb].row ||
            (a->triples[pa].row == b->triples[pb].row &&
             a->triples[pa].col < b->triples[pb].col)) {
            out->triples[q++] = a->triples[pa++];      /* a 独有位置 */
        } else if (b->triples[pb].row < a->triples[pa].row ||
                   (b->triples[pb].row == a->triples[pa].row &&
                    b->triples[pb].col < a->triples[pa].col)) {
            out->triples[q++] = b->triples[pb++];      /* b 独有位置 */
        } else {
            int sum = a->triples[pa].value + b->triples[pb].value;

            if (sum != 0) {
                out->triples[q].row = a->triples[pa].row;
                out->triples[q].col = a->triples[pa].col;
                out->triples[q].value = sum;
                q++;
            }
            /* sum == 0（如 5+(-5)）: 不产生三元组，两个游标都前进 */
            pa++;
            pb++;
        }
    }
    /* 归并收尾: 剩下的一侧整体抄入，另一侧必已耗尽 */
    while (pa < a->count) {
        out->triples[q++] = a->triples[pa++];
    }
    while (pb < b->count) {
        out->triples[q++] = b->triples[pb++];
    }

    out->count = q;      /* q 恰为实际存入的三元组个数 */
    *result = out;
    return (DsResult){DS_OK, "矩阵相加成功"};
}

DsResult matrix_get(const SparseMatrix *m, size_t i, size_t j, int *value)
{
    size_t pos;

    if (m == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (i < 1 || i > m->rows || j < 1 || j > m->cols) {
        return (DsResult){DS_OUT_OF_RANGE, "行列号越界"};
    }

    if (find_triple(m, i, j, &pos)) {
        *value = m->triples[pos].value;
    } else {
        *value = 0;     /* 不在三元组表中即为零元 */
    }
    return (DsResult){DS_OK, "读取成功"};
}

int matrix_equals(const SparseMatrix *a, const SparseMatrix *b)
{
    size_t p;

    if (a == NULL || b == NULL) {
        return a == b;
    }
    if (a->rows != b->rows || a->cols != b->cols || a->count != b->count) {
        return 0;
    }
    for (p = 0; p < a->count; p++) {
        if (a->triples[p].row != b->triples[p].row ||
            a->triples[p].col != b->triples[p].col ||
            a->triples[p].value != b->triples[p].value) {
            return 0;
        }
    }
    return 1;
}

void matrix_print(const SparseMatrix *m)
{
    size_t i;
    size_t j;
    size_t p = 0;

    if (m == NULL) {
        printf("(NULL 矩阵)\n");
        return;
    }

    printf("%lu×%lu 稀疏矩阵（还原形式）:\n",
           (unsigned long)m->rows, (unsigned long)m->cols);
    /*
     * 三元组按行序有序，打印游标 p 随双重循环单调前移，
     * 每个非零元恰好被消费一次，整体 O(mu*nu + tu)
     */
    for (i = 1; i <= m->rows; i++) {
        for (j = 1; j <= m->cols; j++) {
            if (p < m->count &&
                (size_t)m->triples[p].row == i &&
                (size_t)m->triples[p].col == j) {
                printf("%4d", m->triples[p].value);
                p++;
            } else {
                printf("%4d", 0);
            }
        }
        printf("\n");
    }
}

void matrix_print_triples(const SparseMatrix *m)
{
    size_t p;

    if (m == NULL) {
        printf("(NULL 矩阵)\n");
        return;
    }

    printf("三元组表 (mu=%lu, nu=%lu, tu=%lu):\n",
           (unsigned long)m->rows, (unsigned long)m->cols,
           (unsigned long)m->count);
    printf("  行  列  值\n");
    for (p = 0; p < m->count; p++) {
        printf("%4d%4d%4d\n",
               m->triples[p].row, m->triples[p].col, m->triples[p].value);
    }
}

void matrix_destroy(SparseMatrix **m)
{
    if (m == NULL || *m == NULL) {
        return;
    }
    free((*m)->triples);
    free(*m);
    *m = NULL;      /* 置空杜绝悬空指针，重复销毁亦安全 */
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:         return "操作成功";
        case DS_ERROR:      return "一般性错误";
        case DS_NULL_PTR:   return "空指针参数";
        case DS_OUT_OF_RANGE: return "位置/下标越界";
        case DS_OVERFLOW:   return "空间已满或内存分配失败";
        case DS_EMPTY:      return "对空结构取元素";
        case DS_NOT_FOUND:  return "查找未命中";
        default:            return "未知状态码";
    }
}
