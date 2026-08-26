/**
 * @file tridiagonal_matrix.h
 * @brief 三对角矩阵（带状压缩存储）接口定义
 *
 * 存储结构: 仅 |i-j| <= 1 的元素（三条对角线）非零，按行序压入
 *   长度 3n-2 的一维数组:
 *     data = [a11, a12, a21, a22, a23, a32, ..., a(n,n-1), ann]
 *   压缩映射（行列号从 1 开始）: k = 2i + j - 3
 *     推导: 前 i-1 行非零元个数 = 2 + 3(i-2) = 3i-4
 *           （首行 2 个，第 2..n-1 行各 3 个；末行不足 3 个
 *             不影响"前 i-1 行"的计数，i <= n）
 *           第 i 行内首元是 a(i,i-1)，行内偏移 = j - (i-1)
 *           故 k = (3i-4) + (j-i+1) = 2i + j - 3
 *     验证: (1,1)→0, (1,2)→1, (2,1)→2, (2,2)→3, (2,3)→4, (3,2)→5 ✓
 * 设计要点:
 *   1. 3n-2 个单元恰装下 2+3(n-2)+2 个带内元素（首末行各 2 个）
 *   2. 与三角矩阵的常数 c 不同: 对角矩阵带外是真正的 0，
 *      get 带外直接返回 0；set 带外非零值无处存放，拒绝之
 *   3. 不透明指针 —— 映射公式是核心机制，定义放 .c 中
 * 复杂度: get/set 均为 O(1)
 */

#ifndef TRIDIAGONAL_MATRIX_H
#define TRIDIAGONAL_MATRIX_H

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

typedef struct TridiagonalMatrix TridiagonalMatrix;

DsResult matrix_create(TridiagonalMatrix **m, size_t n, const int src[n][n]);
DsResult matrix_get(const TridiagonalMatrix *m, size_t i, size_t j, int *value);
DsResult matrix_set(TridiagonalMatrix *m, size_t i, size_t j, int value);
void matrix_print(const TridiagonalMatrix *m);
void matrix_destroy(TridiagonalMatrix **m);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* TRIDIAGONAL_MATRIX_H */
