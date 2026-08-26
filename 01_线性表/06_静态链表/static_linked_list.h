#ifndef STATIC_LINKED_LIST_H
#define STATIC_LINKED_LIST_H

/**
 * @file static_linked_list.h
 * @brief 静态链表（游标实现）接口定义
 *
 * 存储结构: space[0] 为备用链表头（cur 指向首个空闲分量），
 *           space[MAX-1] 为数据链表头结点（cur 指向首个数据分量），
 *           各分量靠 cur 域串成链，模拟 next 指针
 * 设计要点:
 *   1. 游标模拟指针 —— 在不支持指针的语言里也能实现链表；
 *      结点的"地址"即数组下标，0 充当 NULL（备用链空标志）
 *   2. 定长数组实现且无隐藏不变量，按规范例外条款结构体
 *      直接暴露在头文件，教学上可观察两个链的存储布局
 * 复杂度: 按位查找 O(n)；插入/删除本身 O(1)（不含定位）；
 *         sll_malloc_node/sll_free_node 均 O(1)
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

/* 分量数组最大容量 */
#define STATICLIST_MAX_SIZE 100

/* 分量：data 存元素，cur 存下一分量的下标（游标即"指针"） */
typedef struct {
    int data;
    int cur;
} StaticNode;

/* 静态链表：定长分量数组 + 长度域（存储布局教学可见） */
typedef struct {
    StaticNode space[STATICLIST_MAX_SIZE];
    size_t     length; /* 数据链当前结点数（不含头结点） */
} StaticLinkedList;

DsResult list_init(StaticLinkedList *list);
void list_destroy(StaticLinkedList *list);
size_t list_length(const StaticLinkedList *list);
DsResult list_clear(StaticLinkedList *list);
DsResult list_insert_at(StaticLinkedList *list, size_t pos, int value);
DsResult list_remove_at(StaticLinkedList *list, size_t pos, int *value);
DsResult list_get(const StaticLinkedList *list, size_t pos, int *value);
DsResult list_print(const StaticLinkedList *list);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* STATIC_LINKED_LIST_H */
