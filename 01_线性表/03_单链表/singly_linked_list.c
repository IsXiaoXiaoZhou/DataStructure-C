/**
 * @file singly_linked_list.c
 * @brief 单链表（带头结点）接口实现
 *
 * 存储结构: head → [a1] → [a2] → ... → [an] → NULL
 * 设计要点:
 *   1. 头结点以值形式嵌入表结构（非指针），销毁表壳时它随结构体
 *      一起释放，遍历终止条件统一为 curr == NULL
 *   2. 一切按位操作都从"定位第 pos-1 个结点"出发 —— 带头结点的
 *      统一性体现在 pos==1 时第 0 个结点就是头结点，无需特判
 * 复杂度: 按位查找 O(n)，插入/删除本身 O(1)（不含定位）
 */

#include <stdio.h>

#include <stdlib.h>

#include "singly_linked_list.h"


/* 数据结点：只含后继指针，前驱需从头扫描 */
typedef struct ListNode {
    int             data;
    struct ListNode *next;
} ListNode;

struct SinglyLinkedList {
    ListNode head;   /* 头结点：不存数据，仅作定位锚点 */
    size_t   length; /* 缓存长度，使 list_length 为 O(1) */
};

/* 结点分配集中于此：失败统一映射为 DS_OVERFLOW */
static ListNode *node_new(int value)
{
    ListNode *node = (ListNode *)malloc(sizeof(ListNode));

    if (node != NULL) {
        node->data = value;
        node->next = NULL;
    }
    return node;
}

/* 定位第 pos 个结点（pos 从 1 开始）；pos==0 返回头结点地址 */
static ListNode *node_at(SinglyLinkedList *list, size_t pos)
{
    ListNode *curr = &list->head;
    size_t    i = 0;

    for (i = 0; i < pos; i++) {
        curr = curr->next;
    }
    return curr;
}

DsResult list_init(SinglyLinkedList **list)
{
    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表二级指针为空"};
    }

    *list = malloc(sizeof **list);
    if (*list == NULL) {
        return (DsResult){DS_OVERFLOW, "链表结构体内存分配失败"};
    }

    /* 头结点 next 置 NULL 即空表；head.data 不使用 */
    (*list)->head.next = NULL;
    (*list)->length = 0;

    return (DsResult){DS_OK, "链表创建成功"};
}

void list_destroy(SinglyLinkedList **list)
{
    ListNode *curr = NULL;
    ListNode *next = NULL;

    if (list == NULL || *list == NULL) {
        return;
    }

    /* 注意：必须先保存 next 再 free，先 free 会读已释放内存 */
    curr = (*list)->head.next;
    while (curr != NULL) {
        next = curr->next;
        free(curr);
        curr = next;
    }

    /* 数据结点释放完再释放表壳（头结点嵌在表壳内随之释放） */
    free(*list);
    *list = NULL;
}

size_t list_length(const SinglyLinkedList *list)
{
    /* 前置条件: list 非 NULL；防御性返回 0 而非崩溃 */
    return (list == NULL) ? 0 : list->length;
}

int list_is_empty(const SinglyLinkedList *list)
{
    return (list == NULL) || (list->head.next == NULL);
}

DsResult list_push_front(SinglyLinkedList *list, int value)
{
    return list_insert_at(list, 1, value);
}

DsResult list_push_back(SinglyLinkedList *list, int value)
{
    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }

    /* 无尾指针缓存，复用 insert_at 走到尾结点（第 length 个）之后插入 */
    return list_insert_at(list, list->length + 1, value);
}

DsResult list_insert_at(SinglyLinkedList *list, size_t pos, int value)
{
    ListNode *prev = NULL;
    ListNode *node = NULL;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }
    if (pos < 1 || pos > list->length + 1) {
        return (DsResult){DS_OUT_OF_RANGE, "插入位置越界，合法范围 [1, length+1]"};
    }

    /*
     * 从头结点出发找第 pos-1 个结点（头结点视为第 0 个），
     * 在其后插入 —— 带头结点的设计使 pos==1 无需特判
     */
    prev = node_at(list, pos - 1);

    node = node_new(value);
    if (node == NULL) {
        return (DsResult){DS_OVERFLOW, "结点内存分配失败"};
    }

    /* 注意：先接后继再接前驱，两步顺序可交换，但必须都完成 */
    node->next = prev->next;
    prev->next = node;
    list->length++;

    return (DsResult){DS_OK, "插入成功"};
}

DsResult list_pop_front(SinglyLinkedList *list, int *value)
{
    return list_remove_at(list, 1, value);
}

DsResult list_remove_at(SinglyLinkedList *list, size_t pos, int *value)
{
    ListNode *prev = NULL;
    ListNode *target = NULL;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }
    if (list->length == 0) {
        return (DsResult){DS_EMPTY, "链表为空，无法删除"};
    }
    if (pos < 1 || pos > list->length) {
        return (DsResult){DS_OUT_OF_RANGE, "删除位置越界，合法范围 [1, length]"};
    }

    /* 定位第 pos-1 个结点，删除其后继即第 pos 个结点 */
    prev = node_at(list, pos - 1);
    target = prev->next;

    if (value != NULL) {
        *value = target->data;
    }

    /* 先摘链再释放，防止 free 后读野指针 */
    prev->next = target->next;
    free(target);
    list->length--;

    return (DsResult){DS_OK, "删除成功"};
}

DsResult list_get(const SinglyLinkedList *list, size_t pos, int *value)
{
    ListNode *curr = NULL;
    size_t    i = 0;

    if (list == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (list->length == 0) {
        return (DsResult){DS_EMPTY, "链表为空，无法取元素"};
    }
    if (pos < 1 || pos > list->length) {
        return (DsResult){DS_OUT_OF_RANGE, "位置越界，合法范围 [1, length]"};
    }

    /* 头结点为第 0 个，走 pos-1 步到达第 pos 个结点 */
    curr = list->head.next;
    for (i = 1; i < pos; i++) {
        curr = curr->next;
    }

    *value = curr->data;
    return (DsResult){DS_OK, "取元素成功"};
}

DsResult list_find(const SinglyLinkedList *list, int value, size_t *pos)
{
    ListNode *curr = NULL;
    size_t    index = 0;

    if (list == NULL || pos == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    curr = list->head.next;
    while (curr != NULL) {
        index++;
        if (curr->data == value) {
            *pos = index;
            return (DsResult){DS_OK, "查找成功"};
        }
        curr = curr->next;
    }

    *pos = 0;
    return (DsResult){DS_NOT_FOUND, "未找到目标元素"};
}

DsResult list_reverse(SinglyLinkedList *list)
{
    ListNode *prev = NULL;
    ListNode *curr = NULL;
    ListNode *next = NULL;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }

    /*
     * 迭代三指针法，每轮三步（以 a→b→c 为例）:
     *   第 1 步 next = curr->next   先保存后继 b，
     *           否则下一步改向会让 c 之后的链丢失
     *   第 2 步 curr->next = prev   当前结点掉头指向前驱（a 掉头指向 NULL）
     *   第 3 步 prev/curr 整体右移  prev=a, curr=b，进入下一轮
     * 循环结束时 prev 停在原尾结点——它就是新首元
     */
    curr = list->head.next;
    while (curr != NULL) {
        next = curr->next;
        curr->next = prev;
        prev = curr;
        curr = next;
    }

    /* 头结点改接新首元；prev==NULL（原空表）时同样正确 */
    list->head.next = prev;

    return (DsResult){DS_OK, "反转成功"};
}

DsResult list_clear(SinglyLinkedList *list)
{
    ListNode *curr = NULL;
    ListNode *next = NULL;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }

    /* 与 destroy 同法逐结点释放（先存 next 再 free），但止步于表壳 */
    curr = list->head.next;
    while (curr != NULL) {
        next = curr->next;
        free(curr);
        curr = next;
    }

    /* 头结点随表壳保留，只需断链归零即回到刚 init 完的空表状态 */
    list->head.next = NULL;
    list->length = 0;

    return (DsResult){DS_OK, "链表清空成功"};
}

DsResult list_traverse(const SinglyLinkedList *list, void (*visit)(int))
{
    ListNode *curr = NULL;

    if (list == NULL || visit == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    /* 从首元沿 next 走到 NULL 为止，每个元素回调一次 */
    for (curr = list->head.next; curr != NULL; curr = curr->next) {
        visit(curr->data);
    }

    return (DsResult){DS_OK, "遍历完成"};
}

DsResult list_print(const SinglyLinkedList *list)
{
    ListNode *curr = NULL;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }

    printf("SinglyLinkedList(length=%lu): head -> ",
           (unsigned long)list->length);
    for (curr = list->head.next; curr != NULL; curr = curr->next) {
        printf("[%d] -> ", curr->data);
    }
    printf("NULL\n");

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
