/**
 * @file tridiagonal_matrix.c
 * @brief 三对角矩阵（带状压缩存储）实现
 *
 * 带内元素经 k = 2i + j - 3 一步映射到位；
 * 带外元素根本不占存储，get 返回 0、set 拒绝非零写入。
 */

#include "tridiagonal_matrix.h"

#include <stdio.h>
#include <stdlib.h>


struct TridiagonalMatrix {
    size_t n;     /* 阶数 */
    int *data;    /* 带内压缩数组，长度 3n-2 */
};

/*
 * 判断 (i,j) 是否在三条对角线内（|i-j| <= 1）。
 * 借无符号减法: 行列差的绝对值为 0 或 1 即在带内，
 * 避免对 size_t 做有符号的 abs 运算
 */
static int in_band(size_t i, size_t j)
{
    return (i >= j ? i - j : j - i) <= 1;
}

DsResult matrix_create(TridiagonalMatrix **m, size_t n, const int src[n][n])
{
    TridiagonalMatrix *mat = NULL;
    size_t i;
    size_t j;

    if (m == NULL || src == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (n < 1) {
        return (DsResult){DS_OUT_OF_RANGE, "矩阵阶数须 >= 1"};
    }

    mat = malloc(sizeof *mat);
    if (mat == NULL) {
        return (DsResult){DS_OVERFLOW, "矩阵结构体内存分配失败"};
    }

    mat->n = n;
    mat->data = malloc((3 * n - 2) * sizeof *mat->data);
    if (mat->data == NULL) {
        free(mat);
        return (DsResult){DS_OVERFLOW, "压缩数组内存分配失败"};
    }

    /* 只提取带内元素，src 带外的值被忽略（对角矩阵带外恒为 0） */
    for (i = 1; i <= n; i++) {
        for (j = 1; j <= n; j++) {
            if (in_band(i, j)) {
                mat->data[2 * i + j - 3] = src[i - 1][j - 1];
            }
        }
    }

    *m = mat;
    return (DsResult){DS_OK, "三对角矩阵创建成功"};
}

DsResult matrix_get(const TridiagonalMatrix *m, size_t i, size_t j, int *value)
{
    if (m == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (i < 1 || i > m->n || j < 1 || j > m->n) {
        return (DsResult){DS_OUT_OF_RANGE, "行列号越界，合法范围 [1, n]"};
    }

    if (!in_band(i, j)) {
        *value = 0;     /* 带外恒为 0，不访问压缩数组 */
        return (DsResult){DS_OK, "带外元素恒为0"};
    }
    *value = m->data[2 * i + j - 3];
    return (DsResult){DS_OK, "读取成功"};
}

DsResult matrix_set(TridiagonalMatrix *m, size_t i, size_t j, int value)
{
    if (m == NULL) {
        return (DsResult){DS_NULL_PTR, "矩阵指针为空"};
    }
    if (i < 1 || i > m->n || j < 1 || j > m->n) {
        return (DsResult){DS_OUT_OF_RANGE, "行列号越界，合法范围 [1, n]"};
    }

    if (!in_band(i, j)) {
        /*
         * 带外不占存储: 写 0 与矩阵现状一致，是无副作用的合法操作；
         * 写非零则压缩数组里没有它的位置 —— 拒绝并报越界，
         * 提醒调用方该矩阵从结构上就不允许带外非零元
         */
        if (value == 0) {
            return (DsResult){DS_OK, "带外写0为空操作"};
        }
        return (DsResult){DS_OUT_OF_RANGE, "带外不允许写非零值"};
    }
    m->data[2 * i + j - 3] = value;
    return (DsResult){DS_OK, "写入成功"};
}

void matrix_print(const TridiagonalMatrix *m)
{
    size_t i;
    size_t j;
    int value;

    if (m == NULL) {
        printf("(NULL 矩阵)\n");
        return;
    }

    printf("%lu 阶三对角矩阵（还原方阵）:\n", (unsigned long)m->n);
    for (i = 1; i <= m->n; i++) {
        for (j = 1; j <= m->n; j++) {
            /* 带外走 0 分支、带内走映射，与 get 口径一致 */
            value = in_band(i, j) ? m->data[2 * i + j - 3] : 0;
            printf("%4d", value);
        }
        printf("\n");
    }
}

void matrix_destroy(TridiagonalMatrix **m)
{
    if (m == NULL || *m == NULL) {
        return;
    }
    free((*m)->data);
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
