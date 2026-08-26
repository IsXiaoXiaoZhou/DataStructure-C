/**
 * @file sparse_matrix.h
 * @brief 稀疏矩阵（三元组顺序表）接口定义
 *
 * 存储结构: 非零元个数 tu 远小于 mu*nu 时只存非零元，
 *   每个非零元记 (行号, 列号, 值) 三元组，按行序为主序排列
 *   （行号增、行内列号增），使转置等运算可顺序扫描:
 *     triples[0..tu-1] = [(r1,c1,v1), (r2,c2,v2), ...]
 * 设计要点:
 *   1. Triple 是纯数据包，无不变量需要保护，公开定义
 *   2. SparseMatrix 主体不透明 —— "按行序有序"是转置算法的
 *      全部前提，定义放 .c 中杜绝外部乱序塞入破坏约定
 *   3. 普通转置: 对原矩阵每一列全表扫描一遍，O(nu*tu);
 *      快速转置: 预计算每列非零元个数 num 与各列在结果中的
 *      起始位 cpot，一遍扫描直接定位，O(nu+tu) ——
 *      两版并存便于对比教学
 * 复杂度: create O(mu*nu); get O(tu); 转置复杂度见各函数注释
 */

#ifndef SPARSE_MATRIX_H
#define SPARSE_MATRIX_H

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

/* 三元组: 一个非零元的坐标与值（行列号从 1 开始，教材惯例） */
typedef struct {
    int row;     /* 行号，[1, mu] */
    int col;     /* 列号，[1, nu] */
    int value;   /* 元素值（非零） */
} Triple;

typedef struct SparseMatrix SparseMatrix;

DsResult matrix_create(SparseMatrix **m, size_t rows, size_t cols,
                       const int src[rows][cols]);
DsResult matrix_transpose(const SparseMatrix *src, SparseMatrix **dst);
DsResult matrix_fast_transpose(const SparseMatrix *src, SparseMatrix **dst);
DsResult matrix_add(const SparseMatrix *a, const SparseMatrix *b,
                    SparseMatrix **result);
DsResult matrix_get(const SparseMatrix *m, size_t i, size_t j, int *value);
int matrix_equals(const SparseMatrix *a, const SparseMatrix *b);
void matrix_print(const SparseMatrix *m);
void matrix_print_triples(const SparseMatrix *m);
void matrix_destroy(SparseMatrix **m);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* SPARSE_MATRIX_H */
