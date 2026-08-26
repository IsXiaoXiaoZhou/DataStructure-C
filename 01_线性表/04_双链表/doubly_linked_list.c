/**
 * @file doubly_linked_list.c
 * @brief 双链表（带头结点）接口实现
 *
 * 存储结构: head → [a1] ⇄ [a2] ⇄ ... ⇄ [an] → NULL
 * 设计要点:
 *   1. tail 不变量 —— 空表时 tail == &head；非空时 tail 恒指最后
 *      一个数据结点且 tail->next == NULL。每个改变尾部的操作
 *      （尾插/删尾/反转/删空）都必须同步维护它
 *   2. 头结点以值形式嵌入表结构，首元的 prev 恒指 &head，
 *      使头部的双向接链与中间位置完全同构
 * 复杂度: 按位查找 O(n)；已知结点的插入/删除 O(1)；尾插 O(1)
 */

#include <stdio.h>

#include <stdlib.h>

#include "doubly_linked_list.h"


/* 双链结点：前驱、后继双指针 */
typedef struct DListNode {
    int              data;
    struct DListNode *prev;
    struct DListNode *next;
} DListNode;

struct DoublyLinkedList {
    DListNode head;    /* 头结点：不存数据，锚定双向链 */
    DListNode *tail;   /* 不变量: 空表指 &head，非空指最后一个数据结点 */
    size_t     length; /* 缓存长度，使 list_length 为 O(1) */
};

/* 结点分配集中于此：失败统一映射为 DS_OVERFLOW */
static DListNode *node_new(int value)
{
    DListNode *node = malloc(sizeof *node);

    if (node != NULL) {
        node->data = value;
        node->prev = NULL;
        node->next = NULL;
    }
    return node;
}

/* 定位第 pos 个结点（pos 从 1 开始）；pos==0 返回头结点地址 */
static DListNode *node_at(DoublyLinkedList *list, size_t pos)
{
    DListNode *curr = &list->head;
    size_t     i = 0;

    for (i = 0; i < pos; i++) {
        curr = curr->next;
    }
    return curr;
}

DsResult list_init(DoublyLinkedList **list)
{
    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表二级指针为空"};
    }

    *list = malloc(sizeof **list);
    if (*list == NULL) {
        return (DsResult){DS_OVERFLOW, "链表结构体内存分配失败"};
    }

    /* 空表约定：tail 指回头结点自身，删空后恢复即回到此状态 */
    (*list)->head.prev = NULL;
    (*list)->head.next = NULL;
    (*list)->tail = &(*list)->head;
    (*list)->length = 0;

    return (DsResult){DS_OK, "双链表创建成功"};
}

void list_destroy(DoublyLinkedList **list)
{
    DListNode *curr = NULL;
    DListNode *next = NULL;

    if (list == NULL || *list == NULL) {
        return;
    }

    /* 注意：先保存 next 再 free；头结点嵌在表壳内随 free(*list) 释放 */
    curr = (*list)->head.next;
    while (curr != NULL) {
        next = curr->next;
        free(curr);
        curr = next;
    }

    free(*list);
    *list = NULL;
}

size_t list_length(const DoublyLinkedList *list)
{
    /* 前置条件: list 非 NULL；防御性返回 0 而非崩溃 */
    return (list == NULL) ? 0 : list->length;
}

int list_is_empty(const DoublyLinkedList *list)
{
    return (list == NULL) || (list->head.next == NULL);
}

DsResult list_push_front(DoublyLinkedList *list, int value)
{
    DListNode *node = NULL;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }

    node = node_new(value);
    if (node == NULL) {
        return (DsResult){DS_OVERFLOW, "结点内存分配失败"};
    }

    /* 头插不动 tail（除非空表），四个指针两两互接 */
    node->prev = &list->head;
    node->next = list->head.next;
    if (list->head.next != NULL) {
        list->head.next->prev = node;   /* 原首元回接新结点 */
    } else {
        list->tail = node;              /* 空表插入：新结点即尾 */
    }
    list->head.next = node;
    list->length++;

    return (DsResult){DS_OK, "头插成功"};
}

DsResult list_push_back(DoublyLinkedList *list, int value)
{
    DListNode *node = NULL;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }

    node = node_new(value);
    if (node == NULL) {
        return (DsResult){DS_OVERFLOW, "结点内存分配失败"};
    }

    /*
     * tail 不变量在此兑现价值：尾插无需遍历，O(1)。
     * 空表时 tail == &head，逻辑与一般情况完全一致
     */
    node->prev = list->tail;
    node->next = NULL;
    list->tail->next = node;
    list->tail = node;
    list->length++;

    return (DsResult){DS_OK, "尾插成功"};
}

DsResult list_insert_at(DoublyLinkedList *list, size_t pos, int value)
{
    DListNode *prev = NULL;
    DListNode *node = NULL;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }
    if (pos < 1 || pos > list->length + 1) {
        return (DsResult){DS_OUT_OF_RANGE, "插入位置越界，合法范围 [1, length+1]"};
    }

    /* 尾后插入直接以 tail 为前驱，免走链；否则定位第 pos-1 个结点 */
    prev = (pos == list->length + 1) ? list->tail : node_at(list, pos - 1);

    node = node_new(value);
    if (node == NULL) {
        return (DsResult){DS_OVERFLOW, "结点内存分配失败"};
    }

    /* 双向接链：先两边挂新结点，再让前驱指向新结点 */
    node->prev = prev;
    node->next = prev->next;
    if (prev->next != NULL) {
        prev->next->prev = node;
    } else {
        list->tail = node;   /* 插在尾后（prev 原是尾结点），尾指针前移 */
    }
    prev->next = node;
    list->length++;

    return (DsResult){DS_OK, "插入成功"};
}

DsResult list_pop_front(DoublyLinkedList *list, int *value)
{
    return list_remove_at(list, 1, value);
}

DsResult list_remove_at(DoublyLinkedList *list, size_t pos, int *value)
{
    DListNode *prev = NULL;
    DListNode *target = NULL;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }
    if (list->length == 0) {
        return (DsResult){DS_EMPTY, "链表为空，无法删除"};
    }
    if (pos < 1 || pos > list->length) {
        return (DsResult){DS_OUT_OF_RANGE, "删除位置越界，合法范围 [1, length]"};
    }

    /* 定位第 pos-1 个结点，摘除其后继即第 pos 个结点 */
    prev = node_at(list, pos - 1);
    target = prev->next;

    if (value != NULL) {
        *value = target->data;
    }

    /* 先绕链再释放：前驱直接接上被删结点的后继 */
    prev->next = target->next;
    if (target->next != NULL) {
        target->next->prev = prev;
    } else {
        list->tail = prev;   /* 删的是尾结点，尾指针回退（可能回到 &head） */
    }
    free(target);
    list->length--;

    return (DsResult){DS_OK, "删除成功"};
}

DsResult list_get(const DoublyLinkedList *list, size_t pos, int *value)
{
    DListNode *curr = NULL;
    size_t     i = 0;

    if (list == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (list->length == 0) {
        return (DsResult){DS_EMPTY, "链表为空，无法取元素"};
    }
    if (pos < 1 || pos > list->length) {
        return (DsResult){DS_OUT_OF_RANGE, "位置越界，合法范围 [1, length]"};
    }

    curr = list->head.next;
    for (i = 1; i < pos; i++) {
        curr = curr->next;
    }

    *value = curr->data;
    return (DsResult){DS_OK, "取元素成功"};
}

DsResult list_find(const DoublyLinkedList *list, int value, size_t *pos)
{
    DListNode *curr = NULL;
    size_t     index = 0;

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

DsResult list_prior_elem(const DoublyLinkedList *list, size_t pos, int *value)
{
    DListNode *curr = NULL;
    size_t     i = 0;

    if (list == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (list->length == 0) {
        return (DsResult){DS_EMPTY, "链表为空，无前驱元素"};
    }
    if (pos < 1 || pos > list->length) {
        return (DsResult){DS_OUT_OF_RANGE, "位置越界，合法范围 [1, length]"};
    }
    if (pos == 1) {
        return (DsResult){DS_OUT_OF_RANGE, "首元无前驱元素"};
    }

    /* 定位第 pos 个结点：从头结点后走 pos-1 步 */
    curr = list->head.next;
    for (i = 1; i < pos; i++) {
        curr = curr->next;
    }

    /*
     * 借 prev 指针一步取得前驱 —— 这正是双链表的意义所在：
     * 单链表即使已拿到第 pos 个结点，求其前驱也只能从头重扫
     * O(n)；双链表的 prev 域让"已知结点求前驱"成为 O(1) 一跳
     */
    *value = curr->prev->data;
    return (DsResult){DS_OK, "取前驱成功"};
}

DsResult list_next_elem(const DoublyLinkedList *list, size_t pos, int *value)
{
    DListNode *curr = NULL;
    size_t     i = 0;

    if (list == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (list->length == 0) {
        return (DsResult){DS_EMPTY, "链表为空，无后继元素"};
    }
    if (pos < 1 || pos > list->length) {
        return (DsResult){DS_OUT_OF_RANGE, "位置越界，合法范围 [1, length]"};
    }
    if (pos == list->length) {
        return (DsResult){DS_OUT_OF_RANGE, "尾元无后继元素"};
    }

    /* 定位第 pos 个结点：从头结点后走 pos-1 步 */
    curr = list->head.next;
    for (i = 1; i < pos; i++) {
        curr = curr->next;
    }

    /* 借 next 指针一步取得后继，与 prior_elem 经 prev 取前驱对称 */
    *value = curr->next->data;
    return (DsResult){DS_OK, "取后继成功"};
}

DsResult list_reverse(DoublyLinkedList *list)
{
    DListNode *old_first = NULL;
    DListNode *curr = NULL;
    DListNode *next = NULL;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }
    if (list->length == 0) {
        return (DsResult){DS_OK, "空表反转无需操作"};
    }

    /*
     * 双链表反转：对每个结点交换 prev/next 指针即可整体掉头。
     *   第 1 步 next = curr->next     先保存原后继，防掉头后失联
     *   第 2 步 交换 curr 的 prev/next 后继变前驱、前驱变后继
     *   第 3 步 curr = next           沿原方向继续前进
     * 循环结束时原尾结点即新首元，原首元即新尾结点
     */
    old_first = list->head.next;
    curr = old_first;
    while (curr != NULL) {
        next = curr->next;
        curr->next = curr->prev;
        curr->prev = next;
        curr = next;
    }

    /* 修两个锚点：头结点接新首元（原尾），tail 改指新尾（原首元） */
    list->head.next = list->tail;
    list->tail->prev = &list->head;
    list->tail = old_first;
    /* 原首元掉头后 next 指向 &head，按约定新尾的 next 必须为 NULL */
    old_first->next = NULL;

    return (DsResult){DS_OK, "反转成功"};
}

DsResult list_print(const DoublyLinkedList *list)
{
    DListNode *curr = NULL;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }

    printf("DoublyLinkedList(length=%lu): head ⇄ ",
           (unsigned long)list->length);
    for (curr = list->head.next; curr != NULL; curr = curr->next) {
        printf("[%d] ⇄ ", curr->data);
    }
    printf("NULL\n");

    return (DsResult){DS_OK, "打印完成"};
}

DsResult list_clear(DoublyLinkedList *list)
{
    DListNode *curr = NULL;
    DListNode *next = NULL;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }

    /* 与 destroy 同法沿 next 逐结点释放（先存 next 再 free），但止步于表壳 */
    curr = list->head.next;
    while (curr != NULL) {
        next = curr->next;
        free(curr);
        curr = next;
    }

    /* 恢复 init 时的空表约定：head 断链、tail 复位指回头结点自身 */
    list->head.prev = NULL;
    list->head.next = NULL;
    list->tail = &list->head;
    list->length = 0;

    return (DsResult){DS_OK, "链表清空成功"};
}

DsResult list_traverse(const DoublyLinkedList *list, void (*visit)(int))
{
    DListNode *curr = NULL;

    if (list == NULL || visit == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    /* 从首元沿 next 走到 NULL 为止，每个元素回调一次 */
    for (curr = list->head.next; curr != NULL; curr = curr->next) {
        visit(curr->data);
    }

    return (DsResult){DS_OK, "遍历完成"};
}

DsResult list_print_reverse(const DoublyLinkedList *list)
{
    DListNode *curr = NULL;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }

    /* 从 tail 沿 prev 反向走，走到头结点（&head）为止 */
    printf("Reverse: NULL ⇄ ");
    for (curr = list->tail; curr != &list->head; curr = curr->prev) {
        printf("[%d] ⇄ ", curr->data);
    }
    printf("head\n");

    return (DsResult){DS_OK, "逆序打印完成"};
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
