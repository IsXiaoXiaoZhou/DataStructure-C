/**
 * @file main.c
 * @brief 稀疏矩阵（三元组顺序表）模块测试与演示
 *
 * 测试策略: 用教材 6×7 经典矩阵验证普通/快速两版转置结果一致、
 * 转置语义（T(j,i)==M(i,j)）、转置的转置复原、别名调用防御；
 * 再覆盖全零矩阵、矩阵相加（常规/抵消为 0/形状不符/零矩阵）、
 * 越界与空指针。
 */

#include <assert.h>
#include <stdio.h>

#include "sparse_matrix.h"

int main(void)
{
    SparseMatrix *m = NULL;
    SparseMatrix *t1 = NULL;
    SparseMatrix *t2 = NULL;
    SparseMatrix *back = NULL;
    SparseMatrix *zero_m = NULL;
    SparseMatrix *zero_t = NULL;
    SparseMatrix *zero_t2 = NULL;
    SparseMatrix *add_a = NULL;
    SparseMatrix *add_b = NULL;
    SparseMatrix *add_sum = NULL;
    SparseMatrix *add_expect = NULL;
    SparseMatrix *add_zero = NULL;
    SparseMatrix *add_zero_sum = NULL;
    SparseMatrix *add_other = NULL;
    /* 教材经典 6×7 稀疏矩阵: 8 个非零元，行列跨度大 */
    int src[6][7] = {
        { 0, 12,  9,  0,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0,  0},
        {-3,  0,  0,  0,  0, 14,  0},
        { 0,  0, 24,  0,  0,  0,  0},
        { 0, 18,  0,  0,  0,  0,  0},
        {15,  0,  0, -7,  0,  0,  0}
    };
    int zero_src[3][3] = { {0} };
    /* 加法用 A: 4 个非零元 */
    int add_a_src[3][4] = {
        { 5,  0,  0,  2},
        { 0, -1,  0,  0},
        { 0,  0,  3,  0}
    };
    /* 加法用 B: 4 个非零元，(1,1) 处与 A 恰好抵消 */
    int add_b_src[3][4] = {
        {-5,  0,  7,  0},
        { 0,  4,  0,  0},
        { 6,  0,  0,  0}
    };
    /* A+B 期望和: (1,1) 处 5+(-5)=0 抵消，非零元 5 个 */
    int add_sum_src[3][4] = {
        { 0,  0,  7,  2},
        { 0,  3,  0,  0},
        { 6,  0,  3,  0}
    };
    int add_zero_src[3][4] = { {0} };
    int add_other_src[4][3] = { {0} };   /* 4×3，与 3×4 形状不符 */
    int value;

    printf("=== 稀疏矩阵模块（三元组顺序表，按行序主序） ===\n\n");

    /* 用例1: 构造成功，非零元与零元的 get 均正确 */
    assert(matrix_create(&m, 6, 7, src).code == DS_OK);
    assert(matrix_get(m, 1, 2, &value).code == DS_OK && value == 12);
    assert(matrix_get(m, 3, 1, &value).code == DS_OK && value == -3);
    assert(matrix_get(m, 6, 4, &value).code == DS_OK && value == -7);
    assert(matrix_get(m, 1, 1, &value).code == DS_OK && value == 0);
    assert(matrix_get(m, 2, 5, &value).code == DS_OK && value == 0);

    /* 用例2: 越界与空指针 */
    assert(matrix_get(m, 0, 1, &value).code == DS_OUT_OF_RANGE);
    assert(matrix_get(m, 1, 8, &value).code == DS_OUT_OF_RANGE);
    assert(matrix_get(m, 7, 1, &value).code == DS_OUT_OF_RANGE);
    assert(matrix_get(NULL, 1, 1, &value).code == DS_NULL_PTR);
    assert(matrix_create(NULL, 6, 7, src).code == DS_NULL_PTR);
    assert(matrix_create(&m, 0, 7, src).code == DS_OUT_OF_RANGE);
    assert(matrix_transpose(NULL, &t1).code == DS_NULL_PTR);
    assert(matrix_fast_transpose(m, NULL).code == DS_NULL_PTR);
    matrix_destroy(NULL);

    /* 用例3: 普通转置与快速转置均成功且结果完全一致 */
    assert(matrix_transpose(m, &t1).code == DS_OK);
    assert(matrix_fast_transpose(m, &t2).code == DS_OK);
    assert(matrix_equals(t1, t2) == 1);

    /* 用例4: 转置语义抽查 —— T(j,i) == M(i,j)，且转置后规模互换 */
    assert(matrix_get(t1, 2, 1, &value).code == DS_OK && value == 12);
    assert(matrix_get(t1, 1, 3, &value).code == DS_OK && value == -3);
    assert(matrix_get(t1, 6, 3, &value).code == DS_OK && value == 14);
    assert(matrix_get(t1, 4, 6, &value).code == DS_OK && value == -7);
    assert(matrix_get(t1, 1, 1, &value).code == DS_OK && value == 0);
    assert(matrix_get(t1, 7, 1, &value).code == DS_OK && value == 0);   /* 转置后 7×6，第 7 行合法 */
    assert(matrix_get(t1, 1, 7, &value).code == DS_OUT_OF_RANGE);       /* 但只剩 6 列 */

    /* 用例5: 转置的转置复原原矩阵（两版算法交叉验证） */
    assert(matrix_fast_transpose(t1, &back).code == DS_OK);
    assert(matrix_equals(back, m) == 1);

    /* 用例6: 别名防御 —— dst 即 src 自身地址时返回 DS_ERROR，源矩阵完好 */
    assert(matrix_transpose(m, &m).code == DS_ERROR);
    assert(matrix_fast_transpose(m, &m).code == DS_ERROR);
    assert(matrix_get(m, 1, 2, &value).code == DS_OK && value == 12);
    assert(matrix_get(m, 6, 4, &value).code == DS_OK && value == -7);
    assert(matrix_equals(m, back) == 1);   /* 源矩阵未被动过 */

    /* 用例7: 全零矩阵 —— tu=0 边界，两版转置仍一致 */
    assert(matrix_create(&zero_m, 3, 3, zero_src).code == DS_OK);
    assert(matrix_get(zero_m, 2, 2, &value).code == DS_OK && value == 0);
    assert(matrix_transpose(zero_m, &zero_t).code == DS_OK);
    assert(matrix_equals(zero_m, zero_t) == 1);
    assert(matrix_fast_transpose(zero_m, &zero_t2).code == DS_OK);
    assert(matrix_equals(zero_t, zero_t2) == 1);

    /* 用例8: 矩阵相加 —— 常规、抵消为 0、形状不符、零矩阵 */
    assert(matrix_create(&add_a, 3, 4, add_a_src).code == DS_OK);
    assert(matrix_create(&add_b, 3, 4, add_b_src).code == DS_OK);
    assert(matrix_create(&add_expect, 3, 4, add_sum_src).code == DS_OK);

    /* 常规相加: a 独有 / b 独有 / 同位置求和三种情形一并覆盖 */
    assert(matrix_add(add_a, add_b, &add_sum).code == DS_OK);
    assert(matrix_equals(add_sum, add_expect) == 1);
    assert(matrix_get(add_sum, 1, 3, &value).code == DS_OK && value == 7);
    assert(matrix_get(add_sum, 2, 2, &value).code == DS_OK && value == 3);

    /* 相加抵消: (1,1) 处 5+(-5)=0 不产生三元组，该位置读出零元 */
    assert(matrix_get(add_sum, 1, 1, &value).code == DS_OK && value == 0);

    /* 不同形状: 4×3 与 3×4 不可相加，返回 DS_ERROR */
    assert(matrix_create(&add_other, 4, 3, add_other_src).code == DS_OK);
    assert(matrix_add(add_a, add_other, &add_zero_sum).code == DS_ERROR);

    /* 与零矩阵相加: 结果与原矩阵完全一致 */
    assert(matrix_create(&add_zero, 3, 4, add_zero_src).code == DS_OK);
    assert(matrix_add(add_a, add_zero, &add_zero_sum).code == DS_OK);
    assert(matrix_equals(add_zero_sum, add_a) == 1);

    /* matrix_add 的空指针 */
    assert(matrix_add(NULL, add_b, &add_sum).code == DS_NULL_PTR);
    assert(matrix_add(add_a, NULL, &add_sum).code == DS_NULL_PTR);
    assert(matrix_add(add_a, add_b, NULL).code == DS_NULL_PTR);

    /* 用例9: 不同矩阵判定不相等 */
    assert(matrix_equals(m, t1) == 0);
    assert(matrix_equals(m, NULL) == 0);
    assert(matrix_equals(NULL, NULL) == 1);

    /* --- 演示段落: 三元组表与还原矩阵 --- */
    printf("--- 演示: 原矩阵 M (6×7, tu=8) ---\n");
    matrix_print(m);
    printf("\n--- 演示: M 的三元组表 ---\n");
    matrix_print_triples(m);
    printf("\n--- 演示: 快速转置结果 T (7×6) ---\n");
    matrix_print(t2);
    printf("\n--- 演示: T 的三元组表 ---\n");
    matrix_print_triples(t2);
    printf("\n--- 演示: A+B 和矩阵（(1,1) 处 5+(-5)=0 抵消，tu=5） ---\n");
    matrix_print(add_sum);

    matrix_destroy(&m);
    matrix_destroy(&t1);
    matrix_destroy(&t2);
    matrix_destroy(&back);
    matrix_destroy(&zero_m);
    matrix_destroy(&zero_t);
    matrix_destroy(&zero_t2);
    matrix_destroy(&add_a);
    matrix_destroy(&add_b);
    matrix_destroy(&add_sum);
    matrix_destroy(&add_expect);
    matrix_destroy(&add_zero);
    matrix_destroy(&add_zero_sum);
    matrix_destroy(&add_other);
    printf("\n全部测试通过\n");
    return 0;
}
