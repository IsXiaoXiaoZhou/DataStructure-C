/**
 * @file cross_sparse_matrix.h
 * @brief 稀疏矩阵（十字链表）接口定义
 *
 * 存储结构: 每个非零元是一个结点，同时挂入两条有序链:
 *   - 同一行的非零元按列升序用 right 串成行链
 *   - 同一列的非零元按行升序用 down 串成列链
 *   rhead[0..rows-1] 各指向一行链首，chead[0..cols-1] 各指向一列链首:
 *
 *     rhead[0] → (1,1) ──right──→ (1,4) → NULL
 *     rhead[1] → (2,1) ──right──→ (2,3) → NULL
 *        chead[0]        chead[2]
 *           ↓ down          ↓ down
 *         (2,1)           (3,3)
 *
 * 设计要点:
 *   1. 结点同时属于一行链与一列链，"十字"由此得名 —— 适合非零元
 *      个数/位置剧烈变化的矩阵: 插入删除只改指针，不搬动数据
 *   2. 行链列链均有序 —— set 的插入/删除定位前驱是全部复杂度所在
 *   3. 不透明指针 —— 行列双有序是链结构正确性的前提，
 *      CrossNode 完整定义放 .c 中，外部只能经 API 操作
 * 复杂度: set/get 均为 O(该行非零元数 + 该列非零元数)
 */

#ifndef CROSS_SPARSE_MATRIX_H
#define CROSS_SPARSE_MATRIX_H

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

typedef struct CrossSparseMatrix CrossSparseMatrix;

DsResult matrix_init(CrossSparseMatrix **m, size_t rows, size_t cols);
DsResult matrix_set(CrossSparseMatrix *m, size_t i, size_t j, int value);
DsResult matrix_get(const CrossSparseMatrix *m, size_t i, size_t j, int *value);
void matrix_print(const CrossSparseMatrix *m);
void matrix_print_links(const CrossSparseMatrix *m);
void matrix_destroy(CrossSparseMatrix **m);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* CROSS_SPARSE_MATRIX_H */
