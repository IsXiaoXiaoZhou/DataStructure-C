/**
 * @file symmetric_matrix.h
 * @brief 对称矩阵（下三角压缩存储）接口定义
 *
 * 存储结构: n 阶对称矩阵满足 a(i,j) == a(j,i)，只存下三角（含主对角线）
 *   共 n(n+1)/2 个元素，按行序压入一维数组:
 *     data = [a11, a21, a22, a31, a32, a33, ...]
 *   压缩映射（行列号从 1 开始，教材惯例，与数学记法 a_ij 一致）:
 *     i >= j: k = i(i-1)/2 + j - 1   —— 第 i 行之前累有 1+2+...+(i-1) 个
 *     i <  j: 交换 i、j 后套用上式   —— 上三角镜像到下三角同一单元
 * 设计要点:
 *   1. 上/下三角共享存储 —— set(i,j) 天然同时改写 (j,i)，
 *      对称性在存储层面就不可能被破坏
 *   2. 不透明指针 —— 压缩映射是模块核心机制，
 *      结构体定义放 .c 中强制只能经 API 访问
 * 复杂度: get/set 均为 O(1)（纯下标算术，无遍历）
 */

#ifndef SYMMETRIC_MATRIX_H
#define SYMMETRIC_MATRIX_H

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

typedef struct SymmetricMatrix SymmetricMatrix;

DsResult matrix_create(SymmetricMatrix **m, size_t n, const int src[n][n]);
DsResult matrix_get(const SymmetricMatrix *m, size_t i, size_t j, int *value);
DsResult matrix_set(SymmetricMatrix *m, size_t i, size_t j, int value);
void matrix_print(const SymmetricMatrix *m);
void matrix_destroy(SymmetricMatrix **m);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* SYMMETRIC_MATRIX_H */
