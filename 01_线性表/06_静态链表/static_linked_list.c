/**
 * @file static_linked_list.c
 * @brief 静态链表（游标实现）接口实现
 *
 * 存储结构: space[0] 备用链表头 → 空闲分量链；
 *           space[MAX-1] 数据链头结点 → 数据分量链，链尾 cur=0
 * 设计要点:
 *   1. 两条链共用一个数组 —— 备用链登记空闲分量（模拟系统堆），
 *      数据链承载逻辑结构（模拟用户链表）；教材约定下标 0 作
 *      备用链头，cur==0 等价于指针世界的 NULL
 *   2. 分配/释放即备用链的摘除/归还，与 malloc/free 语义对齐
 * 复杂度: 按位查找 O(n)；插入/删除本身 O(1)（不含定位）
 */

#include <stdio.h>

#include "static_linked_list.h"


/* 数据链头结点固定占用的分量下标 */
#define SLL_HEAD_INDEX (STATICLIST_MAX_SIZE - 1)

/*
 * 模拟 malloc：从备用链头摘下一个分量，返回其下标
 * 备用链头 space[0].cur == 0 表示无空闲分量，等价于 malloc 失败
 */
static DsStatus sll_malloc_node(StaticLinkedList *list, int *node_index)
{
    int index = list->space[0].cur;

    if (index == 0) {
        return DS_OVERFLOW;
    }

    /* 摘除首分量：备用链头跳过它指向下一个空闲分量 */
    list->space[0].cur = list->space[index].cur;
    *node_index = index;

    return DS_OK;
}

/*
 * 模拟 free：把分量头插归还备用链
 * 注意：用头插 O(1) 归还，且能让刚释放的分量优先被再次
 * 利用（局部性好），不必维护按序的空闲链
 */
static void sll_free_node(StaticLinkedList *list, int node_index)
{
    list->space[node_index].cur = list->space[0].cur;
    list->space[0].cur = node_index;
}

DsResult list_init(StaticLinkedList *list)
{
    int i = 0;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "静态链表指针为空"};
    }

    /*
     * 备用链: space[0] → space[1] → ... → space[MAX-2] → 0(空)
     * space[MAX-1] 是数据链头结点，永不进入备用链
     */
    for (i = 0; i < STATICLIST_MAX_SIZE - 1; i++) {
        list->space[i].cur = i + 1;
    }
    list->space[STATICLIST_MAX_SIZE - 2].cur = 0; /* 备用链收尾 */
    list->space[SLL_HEAD_INDEX].cur = 0;          /* 数据链置空 */

    list->length = 0;

    return (DsResult){DS_OK, "静态链表初始化成功"};
}

void list_destroy(StaticLinkedList *list)
{
    /*
     * 栈对象复位语义：全部分量都在栈上定长数组内，没有动态资源
     * 需要释放，"销毁"即复位 —— 重置备用链与数据链，对象回到
     * 刚 init 完的初始状态可直接复用。NULL 入参安全
     */
    if (list == NULL) {
        return;
    }
    (void)list_init(list);   /* list 非 NULL 时必然返回 DS_OK */
}

size_t list_length(const StaticLinkedList *list)
{
    /* 前置条件: list 非 NULL；防御性返回 0 而非崩溃 */
    return (list == NULL) ? 0 : list->length;
}

DsResult list_clear(StaticLinkedList *list)
{
    /*
     * 栈对象无逐结点释放问题：清空即整体重置两条链（数据链
     * 丢弃、空闲分量重新串成备用链），等价于重新 list_init
     */
    return list_init(list);
}

DsResult list_insert_at(StaticLinkedList *list, size_t pos, int value)
{
    int    prev_index = SLL_HEAD_INDEX; /* 第 pos-1 个结点分量下标，头结点为第 0 个 */
    int    new_index = 0;
    size_t i = 0;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "静态链表指针为空"};
    }
    if (pos < 1 || pos > list->length + 1) {
        return (DsResult){DS_OUT_OF_RANGE, "插入位置越界，合法范围 [1, length+1]"};
    }

    /* 定位第 pos-1 个分量：从头结点出发沿 cur 走 pos-1 步 */
    for (i = 0; i < pos - 1; i++) {
        prev_index = list->space[prev_index].cur;
    }

    if (sll_malloc_node(list, &new_index) != DS_OK) {
        return (DsResult){DS_OVERFLOW, "备用链已耗尽，无空闲分量可用"};
    }

    /* 与单链表插入同构：新分量先挂后继，再让前驱挂新分量 */
    list->space[new_index].data = value;
    list->space[new_index].cur = list->space[prev_index].cur;
    list->space[prev_index].cur = new_index;
    list->length++;

    return (DsResult){DS_OK, "插入成功"};
}

DsResult list_remove_at(StaticLinkedList *list, size_t pos, int *value)
{
    int    prev_index = SLL_HEAD_INDEX;
    int    target_index = 0;
    size_t i = 0;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "静态链表指针为空"};
    }
    if (list->length == 0) {
        return (DsResult){DS_EMPTY, "静态链表为空，无法删除"};
    }
    if (pos < 1 || pos > list->length) {
        return (DsResult){DS_OUT_OF_RANGE, "删除位置越界，合法范围 [1, length]"};
    }

    /* 定位第 pos-1 个分量，其后继即待删的第 pos 个分量 */
    for (i = 0; i < pos - 1; i++) {
        prev_index = list->space[prev_index].cur;
    }
    target_index = list->space[prev_index].cur;

    if (value != NULL) {
        *value = list->space[target_index].data;
    }

    /* 先绕链再归还：前驱游标直接跨过被删分量 */
    list->space[prev_index].cur = list->space[target_index].cur;
    sll_free_node(list, target_index);
    list->length--;

    return (DsResult){DS_OK, "删除成功"};
}

DsResult list_get(const StaticLinkedList *list, size_t pos, int *value)
{
    int    curr_index = 0;
    size_t i = 0;

    if (list == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (list->length == 0) {
        return (DsResult){DS_EMPTY, "静态链表为空，无法取元素"};
    }
    if (pos < 1 || pos > list->length) {
        return (DsResult){DS_OUT_OF_RANGE, "位置越界，合法范围 [1, length]"};
    }

    /* 头结点游标即首分量，再走 pos-1 步到第 pos 个分量 */
    curr_index = list->space[SLL_HEAD_INDEX].cur;
    for (i = 1; i < pos; i++) {
        curr_index = list->space[curr_index].cur;
    }

    *value = list->space[curr_index].data;
    return (DsResult){DS_OK, "取元素成功"};
}

DsResult list_print(const StaticLinkedList *list)
{
    int curr_index = 0;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "静态链表指针为空"};
    }

    printf("StaticLinkedList(length=%lu): head",
           (unsigned long)list->length);
    curr_index = list->space[SLL_HEAD_INDEX].cur;
    while (curr_index != 0) {
        /* 同时打印分量下标，展示游标链的真实存储布局 */
        printf(" --%d--> [%d]", curr_index, list->space[curr_index].data);
        curr_index = list->space[curr_index].cur;
    }
    printf(" --0--> (NULL)\n");

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
