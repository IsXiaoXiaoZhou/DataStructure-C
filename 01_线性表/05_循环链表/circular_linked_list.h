#ifndef CIRCULAR_LINKED_LIST_H
#define CIRCULAR_LINKED_LIST_H

/**
 * @file circular_linked_list.h
 * @brief 循环链表（尾指针 rear 表示）接口定义
 *
 * 存储结构: rear → [an] ，且 rear->next → [a1] → [a2] → ... → [an] ↺
 *           (尾结点的 next 回指首元结点，整表成环；空表 rear == NULL)
 * 设计要点:
 *   1. 尾指针表示 —— rear->next 即首元，头插/尾插均 O(1)；
 *      相比头指针表示，从尾部访问无需遍历全表
 *   2. 不透明指针 —— "rear 恒指尾结点、rear->next 恒指首元"
 *      的环不变量由 .c 独占维护，外部无法破坏环结构
 * 复杂度: 按位查找 O(n)；头插/尾插 O(1)；两表合并 O(1)
 */

#include <stddef.h>

/* 状态码（六个线性表模块统一六值版；本模块用不到的成员亦保留，冗余换统一） */
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

/* 不透明指针：环结构不变量隐藏在 .c 中 */
typedef struct CircularLinkedList CircularLinkedList;

DsResult list_init(CircularLinkedList **list);
void list_destroy(CircularLinkedList **list);
DsResult list_push_front(CircularLinkedList *list, int value);
DsResult list_push_back(CircularLinkedList *list, int value);
DsResult list_insert_at(CircularLinkedList *list, size_t pos, int value);
DsResult list_remove_at(CircularLinkedList *list, size_t pos, int *value);
DsResult list_get(const CircularLinkedList *list, size_t pos, int *value);
DsResult list_find(const CircularLinkedList *list, int value, size_t *pos);
size_t list_length(const CircularLinkedList *list);
int list_is_empty(const CircularLinkedList *list);
DsResult list_clear(CircularLinkedList *list);
DsResult list_traverse(const CircularLinkedList *list, void (*visit)(int));
DsResult list_print(const CircularLinkedList *list);
DsResult list_merge(CircularLinkedList *list_a, CircularLinkedList *list_b);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* CIRCULAR_LINKED_LIST_H */
