/**
 * @file triangular_matrix.c
 * @brief 三角矩阵（上/下三角压缩存储）实现
 *
 * 与对称矩阵共用"行序压缩"思想，差异在: 上/下三角不共享，
 * 带外全部落到压缩数组末位的常数 c 上。
 */

#include "triangular_matrix.h"

#include <stdio.h>
#include <stdlib.h>


struct TriangularMatrix {
    size_t n;              /* 阶数 */
    TriangularMode mode;   /* 有效区在下三角还是上三角 */
    int *data;             /* 长度 n(n+1)/2 + 1，末位存常数 c */
};

/*
 * 求 (i,j) 的压缩下标。
 * 带内: *in_zone 置 1，返回三角区内的下标;
 *   下三角 k = i(i-1)/2 + j - 1            —— 行首前累有 i(i-1)/2 个
 *   上三角 k = (i-1)(2n-i)/2 + j - 1       —— 前 i-1 行累计
 *     (i-1)n - (i-1)(i-2)/2 = (i-1)(2n-i+2)/2 个，行内偏移 j-i
 * 带外: *in_zone 置 0，返回末位下标 n(n+1)/2（常数 c 的存储位置）
 */
static size_t storage_index(const TriangularMatrix *m, size_t i, size_t j,
                            int *in_zone)
{
    if (m->mode == TRIANGULAR_LOWER) {
        if (i >= j) {
            *in_zone = 1;
            return i * (i - 1) / 2 + (j - 1);
        }
    } else {
        if (i <= j) {
            *in_zone = 1;
            return (i - 1) * (2 * m->n - i) / 2 + (j - 1);
        }
    }
    *in_zone = 0;
    return m->n * (m->n + 1) / 2;   /* 常数 c 恒存于压缩数组末位 */
}

DsResult matrix_create(TriangularMatrix **m, size_t n, TriangularMode mode,
                       int c, const int src[n][n])
{
    TriangularMatrix *mat = NULL;
    size_t i;
    size_t j;
    size_t zone_size;
    int in_zone;

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

    zone_size = n * (n + 1) / 2;
    mat->n = n;
    mat->mode = mode;
    /* 三角区 n(n+1)/2 个 + 常数 c 占末位 1 个 */
    mat->data = malloc((zone_size + 1) * sizeof *mat->data);
    if (mat->data == NULL) {
        free(mat);
        return (DsResult){DS_OVERFLOW, "压缩数组内存分配失败"};
    }
    mat->data[zone_size] = c;

    /* 只拷贝带内元素，src 带外的值被忽略（带外由 c 统一表示） */
    for (i = 1; i <= n; i++) {
        for (j = 1; j <= n; j++) {
            size_t k = storage_index(mat, i, j, &in_zone);
            if (in_zone) {
                mat->data[k] = src[i - 1][j - 1];
            }
        }
    }

    *m = mat;
    return (DsResult){DS_OK, "三角矩阵创建成功"};
}

DsResult matrix_get(const TriangularMatrix *m, size_t i, size_t j, int *value)
{
    int in_zone;

    if (m == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (i < 1 || i > m->n || j < 1 || j > m->n) {
        return (DsResult){DS_OUT_OF_RANGE, "行列号越界，合法范围 [1, n]"};
    }

    /*
     * 带内带外共用一次映射: 带外时 storage_index 返回末位下标，
     * 读到的自然就是常数 c —— get 无需分支即可满足"带外返回 c"
     */
    *value = m->data[storage_index(m, i, j, &in_zone)];
    return (DsResult){DS_OK, "读取成功"};
}

DsResult matrix_set(TriangularMatrix *m, size_t i, size_t j, int value)
{
    int in_zone;

    if (m == NULL) {
        return (DsResult){DS_NULL_PTR, "矩阵指针为空"};
    }
    if (i < 1 || i > m->n || j < 1 || j > m->n) {
        return (DsResult){DS_OUT_OF_RANGE, "行列号越界，合法范围 [1, n]"};
    }

    /*
     * 写带外 = 修改常数 c: 带外所有单元共享末位这一个存储位置，
     * 写任意带外 (i,j) 都会改变全部带外元素的取值 —— 这不是 bug，
     * 而是三角矩阵"带外为同一常数"定义的存储层体现
     */
    m->data[storage_index(m, i, j, &in_zone)] = value;
    return (DsResult){DS_OK, "写入成功"};
}

void matrix_print(const TriangularMatrix *m)
{
    size_t i;
    size_t j;
    int value;

    if (m == NULL) {
        printf("(NULL 矩阵)\n");
        return;
    }

    printf("%lu 阶%s三角矩阵（还原方阵）:\n", (unsigned long)m->n,
           m->mode == TRIANGULAR_LOWER ? "下" : "上");
    for (i = 1; i <= m->n; i++) {
        for (j = 1; j <= m->n; j++) {
            /* 直接读压缩数组，与 get 同一套映射，口径一致 */
            int in_zone;
            value = m->data[storage_index(m, i, j, &in_zone)];
            printf("%4d", value);
        }
        printf("\n");
    }
}

void matrix_destroy(TriangularMatrix **m)
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
