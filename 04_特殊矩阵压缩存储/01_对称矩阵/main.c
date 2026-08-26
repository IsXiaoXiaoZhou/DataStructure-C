/**
 * @file main.c
 * @brief 对称矩阵模块测试与演示
 *
 * 测试策略: 重点验证压缩共享存储的两条推论 ——
 * 上/下三角 get 同值、set 任一侧镜像同步；再覆盖越界与空指针。
 */

#include <assert.h>
#include <stdio.h>

#include "symmetric_matrix.h"

int main(void)
{
    SymmetricMatrix *m = NULL;
    /* 4 阶对称矩阵: 下三角按教材风格给出，上三角镜像 */
    int src[4][4] = {
        { 1,  2,  3,  4},
        { 2,  5,  6,  7},
        { 3,  6,  8,  9},
        { 4,  7,  9, 10}
    };
    int value;

    printf("=== 对称矩阵模块（下三角压缩 n(n+1)/2 个元素） ===\n\n");

    /* 用例1: 构造成功，对角线与下三角读取正确 */
    assert(matrix_create(&m, 4, src).code == DS_OK);
    assert(matrix_get(m, 1, 1, &value).code == DS_OK && value == 1);
    assert(matrix_get(m, 4, 4, &value).code == DS_OK && value == 10);
    assert(matrix_get(m, 3, 2, &value).code == DS_OK && value == 6);

    /* 用例2: 上/下三角同值 —— 压缩共享单元的直接体现 */
    assert(matrix_get(m, 1, 4, &value).code == DS_OK && value == 4);
    assert(matrix_get(m, 4, 1, &value).code == DS_OK && value == 4);
    assert(matrix_get(m, 2, 3, &value).code == DS_OK && value == 6);
    assert(matrix_get(m, 3, 2, &value).code == DS_OK && value == 6);

    /* 用例3: set 上三角位置，下三角镜像应同步变化 */
    assert(matrix_set(m, 1, 3, 99).code == DS_OK);
    assert(matrix_get(m, 1, 3, &value).code == DS_OK && value == 99);
    assert(matrix_get(m, 3, 1, &value).code == DS_OK && value == 99);

    /* 用例4: set 下三角位置，上三角镜像应同步变化 */
    assert(matrix_set(m, 4, 2, -8).code == DS_OK);
    assert(matrix_get(m, 2, 4, &value).code == DS_OK && value == -8);
    assert(matrix_get(m, 4, 2, &value).code == DS_OK && value == -8);

    /* 用例5: 行列号越界返回 DS_OUT_OF_RANGE（0 与 n+1 两侧都要测） */
    assert(matrix_get(m, 0, 1, &value).code == DS_OUT_OF_RANGE);
    assert(matrix_get(m, 1, 0, &value).code == DS_OUT_OF_RANGE);
    assert(matrix_get(m, 5, 1, &value).code == DS_OUT_OF_RANGE);
    assert(matrix_get(m, 1, 5, &value).code == DS_OUT_OF_RANGE);
    assert(matrix_set(m, 5, 5, 1).code == DS_OUT_OF_RANGE);

    /* 用例6: 空指针与非法阶数 */
    assert(matrix_create(NULL, 4, src).code == DS_NULL_PTR);
    assert(matrix_create(&m, 0, src).code == DS_OUT_OF_RANGE);
    assert(matrix_get(NULL, 1, 1, &value).code == DS_NULL_PTR);
    assert(matrix_set(NULL, 1, 1, 1).code == DS_NULL_PTR);
    matrix_destroy(NULL);

    /* --- 演示段落: 还原方阵打印（上三角由镜像补全） --- */
    printf("--- 演示: set(1,3)=99、set(4,2)=-8 后的还原方阵 ---\n");
    matrix_print(m);

    matrix_destroy(&m);
    printf("\n全部测试通过\n");
    return 0;
}
