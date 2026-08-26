/**
 * @file main.c
 * @brief 三角矩阵模块测试与演示
 *
 * 测试策略: 下三角、上三角两种模式各测一组，重点验证
 * 带外 get 返回常数 c、带外 set 修改 c 后全部带外同步变化。
 */

#include <assert.h>
#include <stdio.h>

#include "triangular_matrix.h"

int main(void)
{
    TriangularMatrix *m = NULL;
    TriangularMatrix *um = NULL;
    /* 下三角源: 带外的 0 会被忽略，由常数 c 接管 */
    int lower_src[4][4] = {
        { 1,  0,  0,  0},
        { 2,  5,  0,  0},
        { 3,  6,  8,  0},
        { 4,  7,  9, 10}
    };
    /* 上三角源: 带外的 0 同样被忽略 */
    int upper_src[4][4] = {
        { 1,  2,  3,  4},
        { 0,  5,  6,  7},
        { 0,  0,  8,  9},
        { 0,  0,  0, 10}
    };
    int value;

    printf("=== 三角矩阵模块（n(n+1)/2 + 1 个压缩单元） ===\n\n");

    /* ---------- 下三角模式 ---------- */

    /* 用例1: 构造成功，带内读取正确 */
    assert(matrix_create(&m, 4, TRIANGULAR_LOWER, 9, lower_src).code == DS_OK);
    assert(matrix_get(m, 1, 1, &value).code == DS_OK && value == 1);
    assert(matrix_get(m, 4, 4, &value).code == DS_OK && value == 10);
    assert(matrix_get(m, 3, 2, &value).code == DS_OK && value == 6);

    /* 用例2: 带外（i < j）get 返回常数 c，而非 src 里的 0 */
    assert(matrix_get(m, 1, 3, &value).code == DS_OK && value == 9);
    assert(matrix_get(m, 2, 4, &value).code == DS_OK && value == 9);
    assert(matrix_get(m, 3, 4, &value).code == DS_OK && value == 9);

    /* 用例3: set 带内正常 */
    assert(matrix_set(m, 2, 1, 20).code == DS_OK);
    assert(matrix_get(m, 2, 1, &value).code == DS_OK && value == 20);

    /* 用例4: set 带外即改常数 c，全部带外单元同步变化，带内不受影响 */
    assert(matrix_set(m, 1, 4, 7).code == DS_OK);
    assert(matrix_get(m, 1, 4, &value).code == DS_OK && value == 7);
    assert(matrix_get(m, 2, 3, &value).code == DS_OK && value == 7);
    assert(matrix_get(m, 3, 1, &value).code == DS_OK && value == 3);

    /* 用例5: 下三角模式的越界与空指针 */
    assert(matrix_get(m, 0, 2, &value).code == DS_OUT_OF_RANGE);
    assert(matrix_get(m, 2, 5, &value).code == DS_OUT_OF_RANGE);
    assert(matrix_set(m, 5, 5, 1).code == DS_OUT_OF_RANGE);
    assert(matrix_get(NULL, 1, 1, &value).code == DS_NULL_PTR);
    assert(matrix_set(NULL, 1, 1, 1).code == DS_NULL_PTR);

    /* ---------- 上三角模式 ---------- */

    /* 用例6: 上三角带内读取正确（验证另一套映射公式） */
    assert(matrix_create(&um, 4, TRIANGULAR_UPPER, -1, upper_src).code == DS_OK);
    assert(matrix_get(um, 1, 4, &value).code == DS_OK && value == 4);
    assert(matrix_get(um, 2, 3, &value).code == DS_OK && value == 6);
    assert(matrix_get(um, 4, 4, &value).code == DS_OK && value == 10);

    /* 用例7: 上三角带外返回常数 c=-1 */
    assert(matrix_get(um, 3, 1, &value).code == DS_OK && value == -1);
    assert(matrix_get(um, 4, 2, &value).code == DS_OK && value == -1);

    /* 用例8: 上三角 set 带外改 c，带内不受影响 */
    assert(matrix_set(um, 4, 1, 0).code == DS_OK);
    assert(matrix_get(um, 3, 2, &value).code == DS_OK && value == 0);
    assert(matrix_get(um, 1, 2, &value).code == DS_OK && value == 2);

    /* 用例9: 构造参数的空指针与非法阶数 */
    assert(matrix_create(NULL, 4, TRIANGULAR_LOWER, 9, lower_src).code == DS_NULL_PTR);
    assert(matrix_create(&um, 0, TRIANGULAR_UPPER, -1, upper_src).code == DS_OUT_OF_RANGE);
    matrix_destroy(NULL);

    /* --- 演示段落: 两种模式的还原方阵 --- */
    printf("--- 演示: 下三角模式（c 已被用例4 改为 7） ---\n");
    matrix_print(m);
    printf("\n--- 演示: 上三角模式（c 已被用例8 改为 0） ---\n");
    matrix_print(um);

    matrix_destroy(&m);
    matrix_destroy(&um);
    printf("\n全部测试通过\n");
    return 0;
}
