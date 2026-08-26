/**
 * @file main.c
 * @brief 三对角矩阵模块测试与演示
 *
 * 测试策略: 带内读写、带外 get 恒 0、带外 set 的零值/非零值两种待遇、
 * n=1 退化阶、越界与空指针；src 带外故意放非零验证 create 的提取语义。
 */

#include <assert.h>
#include <stdio.h>

#include "tridiagonal_matrix.h"

int main(void)
{
    TridiagonalMatrix *m = NULL;
    TridiagonalMatrix *one = NULL;
    /* 5 阶源: 带内为真实值，带外故意放 7 —— create 应只提取带内 */
    int src[5][5] = {
        { 2,  1,  7,  7,  7},
        { 3,  4,  5,  7,  7},
        { 7,  6,  7,  8,  7},
        { 7,  7,  9,  1,  2},
        { 7,  7,  7,  3,  6}
    };
    int one_src[1][1] = { { 42} };
    int value;

    printf("=== 对角矩阵模块（三对角带状压缩 3n-2 个元素） ===\n\n");

    /* 用例1: 构造成功，带内读取正确 */
    assert(matrix_create(&m, 5, src).code == DS_OK);
    assert(matrix_get(m, 1, 1, &value).code == DS_OK && value == 2);
    assert(matrix_get(m, 1, 2, &value).code == DS_OK && value == 1);
    assert(matrix_get(m, 3, 3, &value).code == DS_OK && value == 7);
    assert(matrix_get(m, 5, 4, &value).code == DS_OK && value == 3);
    assert(matrix_get(m, 5, 5, &value).code == DS_OK && value == 6);

    /* 用例2: 带外 get 恒为 0 —— src 里带外的 7 被忽略 */
    assert(matrix_get(m, 1, 3, &value).code == DS_OK && value == 0);
    assert(matrix_get(m, 2, 4, &value).code == DS_OK && value == 0);
    assert(matrix_get(m, 4, 1, &value).code == DS_OK && value == 0);
    assert(matrix_get(m, 5, 2, &value).code == DS_OK && value == 0);

    /* 用例3: set 带内正常 */
    assert(matrix_set(m, 3, 4, 80).code == DS_OK);
    assert(matrix_get(m, 3, 4, &value).code == DS_OK && value == 80);
    assert(matrix_set(m, 4, 3, 90).code == DS_OK);
    assert(matrix_get(m, 4, 3, &value).code == DS_OK && value == 90);

    /* 用例4: set 带外写 0 合法（空操作），写非零被拒绝 */
    assert(matrix_set(m, 1, 3, 0).code == DS_OK);
    assert(matrix_set(m, 1, 3, 5).code == DS_OUT_OF_RANGE);
    assert(matrix_set(m, 5, 1, 5).code == DS_OUT_OF_RANGE);

    /* 用例5: 行列号越界与空指针 */
    assert(matrix_get(m, 0, 1, &value).code == DS_OUT_OF_RANGE);
    assert(matrix_get(m, 1, 6, &value).code == DS_OUT_OF_RANGE);
    assert(matrix_set(m, 6, 6, 1).code == DS_OUT_OF_RANGE);
    assert(matrix_get(NULL, 1, 1, &value).code == DS_NULL_PTR);
    assert(matrix_set(NULL, 1, 1, 1).code == DS_NULL_PTR);
    assert(matrix_create(NULL, 5, src).code == DS_NULL_PTR);
    assert(matrix_create(&m, 0, src).code == DS_OUT_OF_RANGE);
    matrix_destroy(NULL);

    /* 用例6: n=1 退化阶 —— 压缩数组仅 1 个单元，(1,1) 即全部 */
    assert(matrix_create(&one, 1, one_src).code == DS_OK);
    assert(matrix_get(one, 1, 1, &value).code == DS_OK && value == 42);
    assert(matrix_set(one, 1, 1, 7).code == DS_OK);
    assert(matrix_get(one, 1, 1, &value).code == DS_OK && value == 7);
    assert(matrix_get(one, 1, 2, &value).code == DS_OUT_OF_RANGE);

    /* --- 演示段落: 还原方阵打印（带外为 0，可见三条对角线） --- */
    printf("--- 演示: 5 阶三对角矩阵还原方阵 ---\n");
    matrix_print(m);
    printf("\n--- 演示: 1 阶退化矩阵 ---\n");
    matrix_print(one);

    matrix_destroy(&m);
    matrix_destroy(&one);
    printf("\n全部测试通过\n");
    return 0;
}
