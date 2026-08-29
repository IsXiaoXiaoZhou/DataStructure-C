#ifndef DOUBLY_LINKED_LIST_H
#define DOUBLY_LINKED_LIST_H

/**
 * @file doubly_linked_list.h
 * @brief 双链表（带头结点）接口定义
 *
 * 存储结构: head → [a1] ⇄ [a2] ⇄ ... ⇄ [an] → NULL
 *           (头结点嵌入表结构；tail 指针恒指向最后一个数据结点)
 * 设计要点:
 *   1. 带头结点 + 尾指针 —— tail 使尾插 O(1)，代价是每个
 *      删除/插入点都必须维护 tail 不变量
 *   2. 不透明指针 —— "tail 恒指尾结点"的不变量由 .c 独占维护
 * 复杂度: 按位查找 O(n)；已知结点的插入/删除 O(1)；尾插 O(1)
 */

#include <stddef.h>

/* 状态码（六个线性表模块统一七值版；本模块用不到的成员亦保留，冗余换统一） */
typedef enum {
    DS_OK = 0,          /* 操作成功 */
    DS_ERROR,           /* 一般性错误（如查找未命中、非法参数组合） */
    DS_NULL_PTR,        /* 空指针参数 */
    DS_OUT_OF_RANGE,    /* 位置/下标越界 */
    DS_OVERFLOW,        /* 空间已满或内存分配失败（含计算结果溢出） */
    DS_EMPTY,           // 对空结构取元素
    DS_NOT_FOUND        // 查找未命中（业务正常结果，非异常）
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 不透明指针：tail 不变量等实现细节隐藏在 .c 中 */
typedef struct DoublyLinkedList DoublyLinkedList;

DsResult list_init(DoublyLinkedList **list);
void list_destroy(DoublyLinkedList **list);
size_t list_length(const DoublyLinkedList *list);
int list_is_empty(const DoublyLinkedList *list);
DsResult list_push_front(DoublyLinkedList *list, int value);
DsResult list_push_back(DoublyLinkedList *list, int value);
DsResult list_insert_at(DoublyLinkedList *list, size_t pos, int value);
DsResult list_pop_front(DoublyLinkedList *list, int *value);
DsResult list_remove_at(DoublyLinkedList *list, size_t pos, int *value);
DsResult list_get(const DoublyLinkedList *list, size_t pos, int *value);
DsResult list_find(const DoublyLinkedList *list, int value, size_t *pos);
DsResult list_prior_elem(const DoublyLinkedList *list, size_t pos, int *value);
DsResult list_next_elem(const DoublyLinkedList *list, size_t pos, int *value);
DsResult list_reverse(DoublyLinkedList *list);
DsResult list_print(const DoublyLinkedList *list);
DsResult list_print_reverse(const DoublyLinkedList *list);
DsResult list_clear(DoublyLinkedList *list);
DsResult list_traverse(const DoublyLinkedList *list, void (*visit)(int));

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* DOUBLY_LINKED_LIST_H */
