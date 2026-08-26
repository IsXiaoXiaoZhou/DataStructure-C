/**
 * @file main.c
 * @brief 稀疏矩阵（十字链表）模块测试与演示
 *
 * 测试策略: 乱序 set 一批跨行跨列非零元后全量 get 对照期望矩阵
 * （行链寻址正确），再覆盖更新、删除（含链中间结点）、重插、
 * 越界与空指针；演示段落打印行链/列链明细验证 right/down 双链。
 */

#include <assert.h>
#include <stdio.h>

#include "cross_sparse_matrix.h"

#define TEST_ROWS 4
#define TEST_COLS 5

/* 期望矩阵与十字链表逐元素对照: 全量 get 验证行链寻址与有序性 */
static void check_all(const CrossSparseMatrix *m, const int expect[TEST_ROWS][TEST_COLS])
{
    size_t i;
    size_t j;
    int value;

    for (i = 0; i < TEST_ROWS; i++) {
        for (j = 0; j < TEST_COLS; j++) {
            assert(matrix_get(m, i + 1, j + 1, &value).code == DS_OK);
            assert(value == expect[i][j]);
        }
    }
}

int main(void)
{
    CrossSparseMatrix *m = NULL;
    int value;
    /* 期望矩阵: 与每步 set 操作同步维护，供全量对照 */
    int expect[TEST_ROWS][TEST_COLS] = { {0} };

    printf("=== 稀疏矩阵模块（十字链表 right/down 双链） ===\n\n");

    /* 用例1: 初始化后为空矩阵，get 全零 */
    assert(matrix_init(&m, TEST_ROWS, TEST_COLS).code == DS_OK);
    check_all(m, expect);

    /* 用例2: 乱序插入 7 个跨行跨列非零元，全量对照 */
    assert(matrix_set(m, 2, 3, 9).code == DS_OK);    expect[1][2] = 9;
    assert(matrix_set(m, 1, 1, 3).code == DS_OK);    expect[0][0] = 3;
    assert(matrix_set(m, 4, 5, 2).code == DS_OK);    expect[3][4] = 2;
    assert(matrix_set(m, 1, 4, 7).code == DS_OK);    expect[0][3] = 7;
    assert(matrix_set(m, 3, 3, 1).code == DS_OK);    expect[2][2] = 1;
    assert(matrix_set(m, 2, 1, 5).code == DS_OK);    expect[1][0] = 5;
    assert(matrix_set(m, 4, 2, 6).code == DS_OK);    expect[3][1] = 6;
    check_all(m, expect);

    /* 用例3: 更新已存在结点（同行链中间位置） */
    assert(matrix_set(m, 2, 3, 99).code == DS_OK);   expect[1][2] = 99;
    check_all(m, expect);

    /* 用例4: 删除行链链首结点，其余元素不受影响 */
    assert(matrix_set(m, 1, 1, 0).code == DS_OK);    expect[0][0] = 0;
    check_all(m, expect);

    /* 用例5: 删除列链中间结点 —— (3,3) 在列 3 链的 (2,3) 与 NULL 之间 */
    assert(matrix_set(m, 3, 3, 0).code == DS_OK);    expect[2][2] = 0;
    check_all(m, expect);

    /* 用例6: 删除后原位置重插（回到行链链首、列链中间） */
    assert(matrix_set(m, 1, 1, 11).code == DS_OK);   expect[0][0] = 11;
    assert(matrix_set(m, 3, 3, 4).code == DS_OK);    expect[2][2] = 4;
    check_all(m, expect);

    /* 用例7: 对不存在的零元写 0 是合法空操作 */
    assert(matrix_set(m, 4, 4, 0).code == DS_OK);
    assert(matrix_set(m, 1, 1, 0).code == DS_OK);    expect[0][0] = 0;
    assert(matrix_set(m, 1, 1, 0).code == DS_OK);    /* 已删再删仍安全 */
    check_all(m, expect);

    /* 用例8: 越界与空指针 */
    assert(matrix_set(m, 0, 1, 1).code == DS_OUT_OF_RANGE);
    assert(matrix_set(m, 1, 0, 1).code == DS_OUT_OF_RANGE);
    assert(matrix_set(m, 5, 1, 1).code == DS_OUT_OF_RANGE);
    assert(matrix_set(m, 1, 6, 1).code == DS_OUT_OF_RANGE);
    assert(matrix_get(m, 5, 5, &value).code == DS_OUT_OF_RANGE);
    assert(matrix_get(m, 1, 6, &value).code == DS_OUT_OF_RANGE);
    assert(matrix_set(NULL, 1, 1, 1).code == DS_NULL_PTR);
    assert(matrix_get(NULL, 1, 1, &value).code == DS_NULL_PTR);
    assert(matrix_init(NULL, 4, 5).code == DS_NULL_PTR);
    assert(matrix_init(&m, 0, 5).code == DS_OUT_OF_RANGE);
    matrix_destroy(NULL);

    /* --- 演示段落: 还原矩阵 + right/down 两条链明细 --- */
    printf("--- 演示: 还原矩阵（当前非零元 6 个） ---\n");
    matrix_print(m);
    printf("\n--- 演示: 十字链表明细（(行,列,值) 序列验证两条链均有序） ---\n");
    matrix_print_links(m);

    matrix_destroy(&m);
    printf("\n全部测试通过\n");
    return 0;
}
