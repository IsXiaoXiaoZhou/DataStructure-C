/**
 * @file dynamic_seq_list.c
 * @brief 动态顺序表（可扩容数组实现）接口实现
 *
 * 存储结构: heap 上连续区 data[0..capacity-1]，实际使用 [0..length-1]
 * 设计要点:
 *   1. 不透明指针 —— capacity 与 data 指针的一致性由本文件独占维护，
 *      外部无法绕过 API 改字段导致扩容后指针失效
 *   2. realloc 倍增扩容：realloc 在原块后无连续空间时会另觅新块并
 *      搬迁数据，故必须用其返回值更新 data；倍增因子 2 保证 n 次
 *      追加总搬移代价 O(n)，即尾部追加均摊 O(1)
 * 复杂度: 按位存取 O(1)；插入/删除 O(n)；追加均摊 O(1)
 */

#include <stdio.h>

#include <stdlib.h>

#include "dynamic_seq_list.h"


/* 构造参数传 0 时的默认初始容量 */
#define SEQLIST_DEFAULT_CAPACITY 8u

struct SeqList {
    int   *data;     /* 堆上数据区：data[0] 存放第 1 个元素 */
    size_t length;   /* 当前元素个数 */
    size_t capacity; /* 已分配容量，不变量: capacity >= length */
};

/*
 * 倍增扩容：new = capacity * 2
 * 注意：realloc 失败时原块仍然有效，直接返回错误不破坏现有数据
 */
static DsResult seqlist_grow(SeqList *list)
{
    size_t new_capacity = list->capacity * 2;
    int   *new_data = realloc(list->data,
                              new_capacity * sizeof *new_data);

    if (new_data == NULL) {
        return (DsResult){DS_OVERFLOW, "内存扩容失败，realloc返回NULL"};
    }

    list->data = new_data;
    list->capacity = new_capacity;
    return (DsResult){DS_OK, "扩容成功"};
}

DsResult seqlist_init(SeqList **list, size_t init_capacity)
{
    SeqList *obj = NULL;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "顺序表二级指针为空"};
    }

    /* 传 0 视为使用默认容量，避免 0 容量导致首次插入前就要扩容 */
    if (init_capacity == 0) {
        init_capacity = SEQLIST_DEFAULT_CAPACITY;
    }

    obj = malloc(sizeof *obj);
    if (obj == NULL) {
        return (DsResult){DS_OVERFLOW, "顺序表结构体内存分配失败"};
    }

    obj->data = malloc(init_capacity * sizeof *obj->data);
    if (obj->data == NULL) {
        free(obj);         /* 结构体本身已分配，需回滚防止泄漏 */
        return (DsResult){DS_OVERFLOW, "数据区内存分配失败"};
    }
    obj->length = 0;
    obj->capacity = init_capacity;
    *list = obj;

    return (DsResult){DS_OK, "顺序表创建成功"};
}

void seqlist_destroy(SeqList **list)
{
    if (list == NULL || *list == NULL) {
        return;
    }

    /* 先释放数据区再释放表壳；置 NULL 使重复销毁安全 */
    free((*list)->data);
    free(*list);
    *list = NULL;
}

size_t seqlist_length(const SeqList *list)
{
    /* 前置条件: list 非 NULL；防御性返回 0 而非崩溃 */
    return (list == NULL) ? 0 : list->length;
}

size_t seqlist_capacity(const SeqList *list)
{
    return (list == NULL) ? 0 : list->capacity;
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

    /* 满则先倍增扩容，再搬移 —— 与静态版唯一新增分支 */
    if (list->length == list->capacity) {
        DsResult status = seqlist_grow(list);
        if (status.code != DS_OK) {
            return status;
        }
    }

    /* 从末尾向前逐个后移，防止正向搬运覆盖未搬数据 */
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

    /* 删除点之后元素整体前移补洞 */
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

    printf("SeqList(length=%lu, capacity=%lu): [",
           (unsigned long)list->length, (unsigned long)list->capacity);
    for (i = 0; i < list->length; i++) {
        printf("%d%s", list->data[i], (i + 1 < list->length) ? ", " : "");
    }
    printf("]\n");

    return (DsResult){DS_OK, "打印完成"};
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
