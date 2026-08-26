/**
 * @file triangular_matrix.h
 * @brief 三角矩阵（上/下三角压缩存储）接口定义
 *
 * 存储结构: 只存三角区内 n(n+1)/2 个元素 + 压缩数组末位 1 个常数 c，
 *   共 n(n+1)/2 + 1 个单元:
 *     下三角模式: data = [a11, a21, a22, ..., ann, c]
 *     上三角模式: data = [a11..a1n, a22..a2n, ..., ann, c]
 *   压缩映射（行列号从 1 开始，教材惯例）:
 *     下三角 i >= j: k = i(i-1)/2 + j - 1（行序，同对称矩阵）
 *     上三角 i <= j: k = (i-1)(2n-i)/2 + j - 1
 *       —— 前 i-1 行累计 (i-1)(2n-i+2)/2 个，第 i 行内偏移 j-i
 *     带外:        k = n(n+1)/2（末位，即常数 c 的家）
 * 设计要点:
 *   1. 与对称矩阵的本质区别: 上/下三角不共享存储，带外是同一个常数 c
 *   2. 常数 c 独占末位单元 —— "带外全为 c" 这一矩阵语义在存储层
 *      完整保留，get/set 带外与带内走同一套映射
 *   3. 不透明指针 —— mode 与映射公式联动，定义放 .c 中防止误用
 * 复杂度: get/set 均为 O(1)
 */

#ifndef TRIANGULAR_MATRIX_H
#define TRIANGULAR_MATRIX_H

#include <stddef.h>

typedef enum {
    DS_OK = 0,          /* 操作成功 */
    DS_ERROR,           /* 一般性错误（如查找未命中、非法参数组合） */
    DS_NULL_PTR,        /* 空指针参数 */
    DS_OUT_OF_RANGE,    /* 位置/下标越界 */
    DS_OVERFLOW,        /* 空间已满或内存分配失败（含计算结果溢出） */
    DS_EMPTY,           // 对空结构取元素
    DS_NOT_FOUND        // 查找未命中（业务正常结果，非异常）
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 三角模式: 决定有效区在下三角还是上三角 */
typedef enum {
    TRIANGULAR_LOWER = 0,   /* 下三角矩阵: 有效区 i >= j，带外在上方 */
    TRIANGULAR_UPPER = 1    /* 上三角矩阵: 有效区 i <= j，带外在下方 */
} TriangularMode;

typedef struct TriangularMatrix TriangularMatrix;

DsResult matrix_create(TriangularMatrix **m, size_t n, TriangularMode mode,
                       int c, const int src[n][n]);
DsResult matrix_get(const TriangularMatrix *m, size_t i, size_t j, int *value);
DsResult matrix_set(TriangularMatrix *m, size_t i, size_t j, int value);
void matrix_print(const TriangularMatrix *m);
void matrix_destroy(TriangularMatrix **m);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* TRIANGULAR_MATRIX_H */
