/**
 * @file symmetric_matrix.c
 * @brief 对称矩阵（下三角压缩存储）实现
 *
 * 核心是把二维下标 (i,j) 经一次乘加映射到一维压缩下标 k，
 * 上三角访问先交换行列号再映射，全程 O(1)。
 */

#include "symmetric_matrix.h"

#include <stdio.h>
#include <stdlib.h>


struct SymmetricMatrix {
    size_t n;     /* 阶数 */
    int *data;    /* 下三角压缩数组，长度 n(n+1)/2 */
};

/*
 * 压缩映射: 求 (i,j) 在 data 中的下标（0 基）。
 * 第 i 行首元素之前累有 1+2+...+(i-1) = i(i-1)/2 个元素，
 * 行内偏移 j-1，故 k = i(i-1)/2 + j - 1。
 * i < j 时先交换 —— 上三角元素与下三角镜像位置共享同一存储单元，
 * 这正是"用一半空间表示整个对称矩阵"的技巧所在。
 */
static size_t lower_index(size_t i, size_t j)
{
    size_t tmp;

    if (i < j) {
        tmp = i;
        i = j;
        j = tmp;
    }
    return i * (i - 1) / 2 + (j - 1);
}

DsResult matrix_create(SymmetricMatrix **m, size_t n, const int src[n][n])
{
    SymmetricMatrix *mat = NULL;
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
    mat->data = malloc(n * (n + 1) / 2 * sizeof *mat->data);
    if (mat->data == NULL) {
        free(mat);
        return (DsResult){DS_OVERFLOW, "压缩数组内存分配失败"};
    }

    /* 只走下三角（含对角线），src 上三角的值交给镜像语义 */
    for (i = 1; i <= n; i++) {
        for (j = 1; j <= i; j++) {
            mat->data[lower_index(i, j)] = src[i - 1][j - 1];
        }
    }

    *m = mat;
    return (DsResult){DS_OK, "对称矩阵创建成功"};
}

DsResult matrix_get(const SymmetricMatrix *m, size_t i, size_t j, int *value)
{
    if (m == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (i < 1 || i > m->n || j < 1 || j > m->n) {
        return (DsResult){DS_OUT_OF_RANGE, "行列号越界，合法范围 [1, n]"};
    }

    *value = m->data[lower_index(i, j)];
    return (DsResult){DS_OK, "读取成功"};
}

DsResult matrix_set(SymmetricMatrix *m, size_t i, size_t j, int value)
{
    if (m == NULL) {
        return (DsResult){DS_NULL_PTR, "矩阵指针为空"};
    }
    if (i < 1 || i > m->n || j < 1 || j > m->n) {
        return (DsResult){DS_OUT_OF_RANGE, "行列号越界，合法范围 [1, n]"};
    }

    /*
     * 写 (i,j) 与写 (j,i) 落到同一压缩单元，
     * 上三角、下三角两个视角同时生效，无需任何额外同步
     */
    m->data[lower_index(i, j)] = value;
    return (DsResult){DS_OK, "写入成功"};
}

void matrix_print(const SymmetricMatrix *m)
{
    size_t i;
    size_t j;
    int value;

    if (m == NULL) {
        printf("(NULL 矩阵)\n");
        return;
    }

    printf("%lu 阶对称矩阵（还原方阵）:\n", (unsigned long)m->n);
    for (i = 1; i <= m->n; i++) {
        for (j = 1; j <= m->n; j++) {
            /* 复用 get 的镜像逻辑，保证打印与读取口径一致 */
            value = m->data[lower_index(i, j)];
            printf("%4d", value);
        }
        printf("\n");
    }
}

void matrix_destroy(SymmetricMatrix **m)
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
