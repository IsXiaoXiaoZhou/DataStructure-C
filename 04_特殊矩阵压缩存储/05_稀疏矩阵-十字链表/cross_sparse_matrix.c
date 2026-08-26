/**
 * @file cross_sparse_matrix.c
 * @brief 稀疏矩阵（十字链表）实现
 *
 * 正确性核心: 行链按列升序、列链按行升序两条不变量，
 * set 的插入/删除都要同时维护两条链。
 */

#include "cross_sparse_matrix.h"

#include <stdio.h>
#include <stdlib.h>


/* 十字链表结点: 每个非零元同时挂在一条行链与一条列链上 */
struct CrossNode {
    int row;                  /* 行号，从 1 开始 */
    int col;                  /* 列号，从 1 开始 */
    int value;                /* 非零元值 */
    struct CrossNode *right;  /* 同行下一个非零元（列号更大） */
    struct CrossNode *down;   /* 同列下一个非零元（行号更大） */
};

struct CrossSparseMatrix {
    size_t rows;              /* 行数 */
    size_t cols;              /* 列数 */
    struct CrossNode **rhead;  /* rhead[i]: 第 i+1 行链首，按列升序 */
    struct CrossNode **chead;  /* chead[j]: 第 j+1 列链首，按行升序 */
};

DsResult matrix_init(CrossSparseMatrix **m, size_t rows, size_t cols)
{
    CrossSparseMatrix *mat = NULL;

    if (m == NULL) {
        return (DsResult){DS_NULL_PTR, "矩阵二级指针为空"};
    }
    if (rows < 1 || cols < 1) {
        return (DsResult){DS_OUT_OF_RANGE, "行列数须 >= 1"};
    }

    mat = malloc(sizeof *mat);
    if (mat == NULL) {
        return (DsResult){DS_OVERFLOW, "矩阵结构体内存分配失败"};
    }
    /* calloc 顺带清零: 空矩阵的每条行链/列链首指针都是 NULL */
    mat->rhead = calloc(rows, sizeof *mat->rhead);
    mat->chead = calloc(cols, sizeof *mat->chead);
    if (mat->rhead == NULL || mat->chead == NULL) {
        free(mat->rhead);
        free(mat->chead);
        free(mat);
        return (DsResult){DS_OVERFLOW, "行列头指针数组内存分配失败"};
    }
    mat->rows = rows;
    mat->cols = cols;

    *m = mat;
    return (DsResult){DS_OK, "十字链表矩阵创建成功"};
}

DsResult matrix_set(CrossSparseMatrix *m, size_t i, size_t j, int value)
{
    struct CrossNode *prev_r = NULL;
    struct CrossNode *cur_r = NULL;
    struct CrossNode *prev_c = NULL;
    struct CrossNode *cur_c = NULL;
    struct CrossNode *node = NULL;

    if (m == NULL) {
        return (DsResult){DS_NULL_PTR, "矩阵指针为空"};
    }
    if (i < 1 || i > m->rows || j < 1 || j > m->cols) {
        return (DsResult){DS_OUT_OF_RANGE, "行列号越界"};
    }

    /*
     * 行链定位: cur_r 停在首个列号 >= j 的结点，
     * prev_r 是它在这条行链上的前驱（可能为 NULL 即链首）
     */
    cur_r = m->rhead[i - 1];
    while (cur_r != NULL && (size_t)cur_r->col < j) {
        prev_r = cur_r;
        cur_r = cur_r->right;
    }

    if (cur_r != NULL && (size_t)cur_r->col == j) {
        if (value != 0) {
            /* 情形1: 结点已存在且写非零 —— 原地更新即可 */
            cur_r->value = value;
            return (DsResult){DS_OK, "更新成功"};
        }

        /*
         * 情形2: 结点已存在且写零 —— 从两条链上摘除后释放。
         * 行链前驱已在上面找到，还需沿列链再找一次列方向前驱
         */
        prev_c = NULL;
        cur_c = m->chead[j - 1];
        while (cur_c != NULL && cur_c != cur_r) {
            prev_c = cur_c;
            cur_c = cur_c->down;
        }
        /*
         * 正常情况不可能发生（结点在行链上就必在列链上），
         * 防御未来改动: 列链走到底仍未命中说明双链不变量已破坏，
         * 此时不动任何指针直接报错，避免摘链操作踩空
         */
        if (cur_c == NULL) {
            return (DsResult){DS_ERROR, "内部双链不变量破坏"};
        }

        /* 注意: 两条链各摘一次，缺一条则另一条残留悬空指针 */
        if (prev_r == NULL) {
            m->rhead[i - 1] = cur_r->right;
        } else {
            prev_r->right = cur_r->right;
        }
        if (prev_c == NULL) {
            m->chead[j - 1] = cur_r->down;
        } else {
            prev_c->down = cur_r->down;
        }
        free(cur_r);
        return (DsResult){DS_OK, "删除成功"};
    }

    /* 情形4: 结点不存在且写零 —— 无事可做 */
    if (value == 0) {
        return (DsResult){DS_OK, "写0为空操作"};
    }

    /* 情形3: 结点不存在且写非零 —— 新建结点插入两条链 */
    node = malloc(sizeof *node);
    if (node == NULL) {
        return (DsResult){DS_OVERFLOW, "结点内存分配失败"};
    }
    node->row = (int)i;
    node->col = (int)j;
    node->value = value;

    /* 先接行链: 新结点的 right 接住 cur_r，再让前驱（或链首）指向它 */
    node->right = cur_r;
    if (prev_r == NULL) {
        m->rhead[i - 1] = node;
    } else {
        prev_r->right = node;
    }

    /* 再接列链: 同样先定位列方向前驱 */
    prev_c = NULL;
    cur_c = m->chead[j - 1];
    while (cur_c != NULL && (size_t)cur_c->row < i) {
        prev_c = cur_c;
        cur_c = cur_c->down;
    }
    node->down = cur_c;
    if (prev_c == NULL) {
        m->chead[j - 1] = node;
    } else {
        prev_c->down = node;
    }

    return (DsResult){DS_OK, "插入成功"};
}

DsResult matrix_get(const CrossSparseMatrix *m, size_t i, size_t j, int *value)
{
    struct CrossNode *cur = NULL;

    if (m == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (i < 1 || i > m->rows || j < 1 || j > m->cols) {
        return (DsResult){DS_OUT_OF_RANGE, "行列号越界"};
    }

    /* 行链按列升序，列号越过 j 即可提前终止 */
    cur = m->rhead[i - 1];
    while (cur != NULL && (size_t)cur->col < j) {
        cur = cur->right;
    }
    if (cur != NULL && (size_t)cur->col == j) {
        *value = cur->value;
    } else {
        *value = 0;     /* 链上找不到即为零元 */
    }
    return (DsResult){DS_OK, "读取成功"};
}

void matrix_print(const CrossSparseMatrix *m)
{
    size_t i;
    size_t j;
    struct CrossNode *cur = NULL;

    if (m == NULL) {
        printf("(NULL 矩阵)\n");
        return;
    }

    printf("%lu×%lu 稀疏矩阵（十字链表还原形式）:\n",
           (unsigned long)m->rows, (unsigned long)m->cols);
    for (i = 1; i <= m->rows; i++) {
        /* 每行从链首出发，cur 指向下一个待打印的非零元 */
        cur = m->rhead[i - 1];
        for (j = 1; j <= m->cols; j++) {
            if (cur != NULL && (size_t)cur->col == j) {
                printf("%4d", cur->value);
                cur = cur->right;
            } else {
                printf("%4d", 0);
            }
        }
        printf("\n");
    }
}

void matrix_print_links(const CrossSparseMatrix *m)
{
    size_t i;
    size_t j;
    struct CrossNode *cur = NULL;

    if (m == NULL) {
        printf("(NULL 矩阵)\n");
        return;
    }

    printf("行链明细（right 方向）:\n");
    for (i = 0; i < m->rows; i++) {
        printf("  第 %lu 行:", (unsigned long)(i + 1));
        for (cur = m->rhead[i]; cur != NULL; cur = cur->right) {
            printf(" (%d,%d,%d)", cur->row, cur->col, cur->value);
        }
        printf("\n");
    }

    printf("列链明细（down 方向）:\n");
    for (j = 0; j < m->cols; j++) {
        printf("  第 %lu 列:", (unsigned long)(j + 1));
        for (cur = m->chead[j]; cur != NULL; cur = cur->down) {
            printf(" (%d,%d,%d)", cur->row, cur->col, cur->value);
        }
        printf("\n");
    }
}

void matrix_destroy(CrossSparseMatrix **m)
{
    size_t i;
    struct CrossNode *cur = NULL;
    struct CrossNode *next = NULL;

    if (m == NULL || *m == NULL) {
        return;
    }

    /*
     * 每个结点恰好挂在一条行链上（列链只是第二个视角），
     * 故沿行链遍历释放一遍即可不漏不重，无需再走列链
     */
    for (i = 0; i < (*m)->rows; i++) {
        cur = (*m)->rhead[i];
        while (cur != NULL) {
            next = cur->right;
            free(cur);
            cur = next;
        }
    }
    free((*m)->rhead);
    free((*m)->chead);
    free(*m);
    *m = NULL;      /* 置空杜绝悬空指针，重复销毁亦安全 */
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:         return "操作成功";
        case DS_ERROR:      return "一般性错误";
        case DS_NULL_PTR:   return "空指针参数";
        case DS_OUT_OF_RANGE: return "位置/下标越界";
        case DS_OVERFLOW:   return "空间已满或内存分配失败";
        case DS_EMPTY:      return "对空结构取元素";
        case DS_NOT_FOUND:  return "查找未命中";
        default:            return "未知状态码";
    }
}
