#ifndef SINGLY_LINKED_LIST_H
#define SINGLY_LINKED_LIST_H

/**
 * @file singly_linked_list.h
 * @brief 单链表（带头结点）接口定义
 *
 * 存储结构: head → [a1] → [a2] → ... → [an] → NULL
 *           (头结点不含数据，头结点的 next 指向首元结点)
 * 设计要点:
 *   1. 带头结点 —— 首元结点的插入/删除与其他位置逻辑统一，
 *      无需对空表、头部操作做特判
 *   2. 不透明指针 —— 结构体定义在 .c 中，外部无法绕过 API
 *      破坏"头结点恒存在"这一不变量
 * 复杂度: 按位查找 O(n)，插入/删除本身 O(1)（不含定位）
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

/* 不透明指针：头结点约定等不变量由 .c 独占维护 */
typedef struct SinglyLinkedList SinglyLinkedList;

/**
 * @brief 创建并初始化空单链表
 * @param list 二级指针出参，带回新链表，不可为 NULL
 * @return DS_OK 成功; DS_NULL_PTR 参数为 NULL; DS_OVERFLOW 内存分配失败
 * @note O(1)；创建即含头结点；用完必须 list_destroy 释放
 */
DsResult list_init(SinglyLinkedList **list);

/**
 * @brief 销毁链表并释放全部结点
 * @param list 二级指针；NULL 安全；销毁后 *list 置 NULL
 * @note 逐结点释放 O(n)；必须先保存 next 再 free，否则断链泄漏
 */
void list_destroy(SinglyLinkedList **list);

/**
 * @brief 返回元素个数
 * @param list 链表指针，不可为 NULL（为 NULL 时返回 0）
 * @return 元素个数
 * @note O(1)
 */
size_t list_length(const SinglyLinkedList *list);

/**
 * @brief 判断链表是否为空
 * @param list 链表指针，不可为 NULL（为 NULL 时视为空返回 1）
 * @return 1 空; 0 非空
 * @note 判据是头结点的 next 是否为 NULL，O(1)
 */
int list_is_empty(const SinglyLinkedList *list);

/**
 * @brief 头插：在首元结点之前插入
 * @param list  链表指针，不可为 NULL
 * @param value 待插入元素
 * @return DS_OK 成功; DS_NULL_PTR list 为 NULL; DS_OVERFLOW 分配失败
 * @note 带头结点使头插无需特判，指针操作 O(1)
 */
DsResult list_push_front(SinglyLinkedList *list, int value);

/**
 * @brief 尾插：在表尾追加元素
 * @param list  链表指针，不可为 NULL
 * @param value 待插入元素
 * @return DS_OK 成功; DS_NULL_PTR list 为 NULL; DS_OVERFLOW 分配失败
 * @note 无尾指针缓存需先走到尾结点，O(n)
 */
DsResult list_push_back(SinglyLinkedList *list, int value);

/**
 * @brief 在第 pos 个位置插入元素（pos 从 1 开始计）
 * @param list  链表指针，不可为 NULL
 * @param pos   插入位置，合法范围 [1, length+1]
 * @param value 待插入元素
 * @return DS_OK 成功; DS_NULL_PTR list 为 NULL;
 *         DS_OUT_OF_RANGE pos 非法; DS_OVERFLOW 结点分配失败
 * @note 需先定位到第 pos-1 个结点，时间复杂度 O(n)
 */
DsResult list_insert_at(SinglyLinkedList *list, size_t pos, int value);

/**
 * @brief 头删：删除首元结点并带回其值
 * @param list  链表指针，不可为 NULL
 * @param value 出参，带回被删元素；可为 NULL（不关心值时）
 * @return DS_OK 成功; DS_NULL_PTR list 为 NULL; DS_EMPTY 表为空
 * @note 仅改头结点 next，O(1)
 */
DsResult list_pop_front(SinglyLinkedList *list, int *value);

/**
 * @brief 删除第 pos 个位置的元素并带回其值
 * @param list  链表指针，不可为 NULL
 * @param pos   删除位置，合法范围 [1, length]
 * @param value 出参，带回被删元素；可为 NULL（不关心值时）
 * @return DS_OK 成功; DS_NULL_PTR list 为 NULL;
 *         DS_EMPTY 表为空; DS_OUT_OF_RANGE pos 非法
 * @note 需先定位到第 pos-1 个结点，O(n)
 */
DsResult list_remove_at(SinglyLinkedList *list, size_t pos, int *value);

/**
 * @brief 取第 pos 个元素的值
 * @param list  链表指针，不可为 NULL
 * @param pos   元素位置，合法范围 [1, length]
 * @param value 出参，带回元素值，不可为 NULL
 * @return DS_OK 成功; DS_NULL_PTR 参数为 NULL;
 *         DS_EMPTY 表为空; DS_OUT_OF_RANGE pos 非法
 * @note 单链表不支持随机存取，需从头走 pos-1 步，O(n)
 */
DsResult list_get(const SinglyLinkedList *list, size_t pos, int *value);

/**
 * @brief 查找首个等于 value 的元素位置
 * @param list  链表指针，不可为 NULL
 * @param value 待查找值
 * @param pos   出参，带回位置（从 1 开始；未找到时置 0），不可为 NULL
 * @return DS_OK 找到; DS_ERROR 未找到; DS_NULL_PTR 参数为 NULL
 * @note 顺序扫描 O(n)
 */
DsResult list_find(const SinglyLinkedList *list, int value, size_t *pos);

/**
 * @brief 原地反转链表（迭代三指针法）
 * @param list 链表指针，不可为 NULL
 * @return DS_OK 成功; DS_NULL_PTR list 为 NULL
 * @note 逐结点把 next 指向前驱，一趟 O(n) 完成，O(1) 额外空间
 */
DsResult list_reverse(SinglyLinkedList *list);

/**
 * @brief 清空链表：释放全部数据结点，保留表壳
 * @param list 链表指针，不可为 NULL
 * @return DS_OK 成功; DS_NULL_PTR list 为 NULL
 * @note 与 destroy 的区别：表壳（含头结点）不释放，清空后
 *       length==0、头结点 next==NULL，可继续插入复用，O(n)
 */
DsResult list_clear(SinglyLinkedList *list);

/**
 * @brief 遍历链表，对每个元素调用回调 visit
 * @param list  链表指针，不可为 NULL
 * @param visit 元素访问回调，不可为 NULL
 * @return DS_OK 成功; DS_NULL_PTR list 或 visit 为 NULL
 * @note 从首元到尾元依次回调，共 length 次，O(n)
 */
DsResult list_traverse(const SinglyLinkedList *list, void (*visit)(int));

/**
 * @brief 打印链表全部元素
 * @param list 链表指针，不可为 NULL
 * @return DS_OK 成功; DS_NULL_PTR list 为 NULL
 * @note 演示用，O(n)
 */
DsResult list_print(const SinglyLinkedList *list);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* SINGLY_LINKED_LIST_H */
