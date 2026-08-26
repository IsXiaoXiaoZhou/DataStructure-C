/**
 * @file static_seq_list.c
 * @brief 静态顺序表（定长数组实现）接口实现
 *
 * 存储结构: data[0..length-1] 连续存放 a1..an
 * 设计要点:
 *   1. 结构体定义暴露在头文件中（无不变量需要保护的教学例外）
 *   2. 插入/删除的元素成块移动是本模块核心：移动方向选错
 *      会覆盖未搬移的数据 —— 插入从尾往前挪，删除从头往后挪
 * 复杂度: 按位存取 O(1)；插入/删除 O(n)
 */

#include <stdio.h>

#include "static_seq_list.h"


DsResult seqlist_init(SeqList *list)
{
    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "顺序表指针为空"};
    }
    list->length = 0;
    return (DsResult){DS_OK, "初始化成功"};
}

size_t seqlist_length(const SeqList *list)
{
    return (list == NULL) ? 0 : list->length;
}

int seqlist_is_empty(const SeqList *list)
{
    return (list == NULL) || (list->length == 0);
}

DsResult seqlist_insert(SeqList *list, size_t pos, int value)
{
    size_t i = 0;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "顺序表指针为空"};
    }
    if (pos < 1 || pos > list->length + 1) {
        return (DsResult){DS_OUT_OF_RANGE, "插入位置越界，合法范围 [1, length+1]"};
    }
    if (list->length >= SEQLIST_MAX_SIZE) {
        return (DsResult){DS_OVERFLOW, "顺序表已满，无法插入"};
    }

    /* 注意：必须从最后一个元素开始逐个后移 */
    for (i = list->length; i > pos - 1; i--) {
        list->data[i] = list->data[i - 1];
    }
    list->data[pos - 1] = value;
    list->length++;
    return (DsResult){DS_OK, "插入成功"};
}

DsResult seqlist_delete(SeqList *list, size_t pos, int *value)
{
    size_t i = 0;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "顺序表指针为空"};
    }
    if (list->length == 0) {
        return (DsResult){DS_EMPTY, "顺序表为空，无法删除"};
    }
    if (pos < 1 || pos > list->length) {
        return (DsResult){DS_OUT_OF_RANGE, "删除位置越界，合法范围 [1, length]"};
    }

    if (value != NULL) {
        *value = list->data[pos - 1];
    }

    for (i = pos - 1; i < list->length - 1; i++) {
        list->data[i] = list->data[i + 1];
    }
    list->length--;
    return (DsResult){DS_OK, "删除成功"};
}

DsResult seqlist_get(const SeqList *list, size_t pos, int *value)
{
    if (list == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (list->length == 0) {
        return (DsResult){DS_EMPTY, "顺序表为空，无法取元素"};
    }
    if (pos < 1 || pos > list->length) {
        return (DsResult){DS_OUT_OF_RANGE, "位置越界，合法范围 [1, length]"};
    }

    *value = list->data[pos - 1];
    return (DsResult){DS_OK, "取元素成功"};
}

DsResult seqlist_set(SeqList *list, size_t pos, int value)
{
    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "顺序表指针为空"};
    }
    if (list->length == 0) {
        return (DsResult){DS_EMPTY, "顺序表为空，无法修改"};
    }
    if (pos < 1 || pos > list->length) {
        return (DsResult){DS_OUT_OF_RANGE, "位置越界，合法范围 [1, length]"};
    }

    list->data[pos - 1] = value;
    return (DsResult){DS_OK, "修改成功"};
}

DsResult seqlist_find(const SeqList *list, int value, size_t *pos)
{
    size_t i = 0;

    if (list == NULL || pos == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    for (i = 0; i < list->length; i++) {
        if (list->data[i] == value) {
            *pos = i + 1;
            return (DsResult){DS_OK, "查找成功"};
        }
    }

    *pos = 0;
    return (DsResult){DS_NOT_FOUND, "未找到目标元素"};
}

DsResult seqlist_print(const SeqList *list)
{
    size_t i = 0;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "顺序表指针为空"};
    }

    printf("SeqList(length=%lu/%d): [", (unsigned long)list->length,
           SEQLIST_MAX_SIZE);
    for (i = 0; i < list->length; i++) {
        printf("%d%s", list->data[i], (i + 1 < list->length) ? ", " : "");
    }
    printf("]\n");
    return (DsResult){DS_OK, "打印完成"};
}
