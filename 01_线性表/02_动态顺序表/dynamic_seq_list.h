#ifndef DYNAMIC_SEQ_LIST_H
#define DYNAMIC_SEQ_LIST_H

/**
 * @file dynamic_seq_list.h
 * @brief 动态顺序表（可扩容数组实现）接口定义
 *
 * 存储结构: heap 上连续区 data[0..capacity-1]，实际使用 [0..length-1]
 * 设计要点:
 *   1. 不透明指针 —— 结构体含 capacity/length 不变量（扩容后指针、
 *      长度必须一致），定义放 .c 中防止外部绕过 API 直接改字段破坏一致性
 *   2. 倍增扩容策略使尾部追加均摊 O(1)
 * 复杂度: 按位存取 O(1)；插入/删除 O(n)；追加均摊 O(1)
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

/* 不透明指针：实现细节（data/length/capacity）隐藏在 .c 中 */
typedef struct SeqList SeqList;

/**
 * @brief 创建并初始化动态顺序表
 * @param list           二级指针出参，带回新表，不可为 NULL
 * @param init_capacity  初始容量；传 0 时内部取默认值 8
 * @return DS_OK 成功; DS_NULL_PTR 参数为 NULL; DS_OVERFLOW 内存分配失败
 * @note O(1)；表用完必须 seqlist_destroy 释放
 */
DsResult seqlist_init(SeqList **list, size_t init_capacity);

/**
 * @brief 销毁顺序表并释放全部资源
 * @param list 表指针；NULL 时安全空操作；销毁后指针置 NULL 防悬垂
 * @note O(1)；内部 free 后置 *list = NULL，重复销毁安全
 */
void seqlist_destroy(SeqList **list);

/**
 * @brief 返回当前元素个数
 * @param list 表指针，不可为 NULL（为 NULL 时返回 0）
 * @return 元素个数
 * @note O(1)
 */
size_t seqlist_length(const SeqList *list);

/**
 * @brief 返回当前已分配容量
 * @param list 表指针，不可为 NULL（为 NULL 时返回 0）
 * @return 容量（已分配可容纳的元素数，非元素个数）
 * @note O(1)；与 length 的区别：capacity >= length 恒成立
 */
size_t seqlist_capacity(const SeqList *list);

/**
 * @brief 判断顺序表是否为空
 * @param list 表指针，不可为 NULL（为 NULL 时视为空返回 1）
 * @return 1 空表; 0 非空
 * @note O(1)
 */
int seqlist_is_empty(const SeqList *list);

/**
 * @brief 在第 pos 个位置插入元素（pos 从 1 开始计）
 * @param list  表指针，不可为 NULL
 * @param pos   插入位置，合法范围 [1, length+1]
 * @param value 待插入元素
 * @return DS_OK 成功; DS_NULL_PTR list 为 NULL;
 *         DS_OUT_OF_RANGE pos 非法; DS_OVERFLOW 扩容失败
 * @note 满则先倍增扩容再从末尾向前搬移元素插入，O(n)
 */
DsResult seqlist_insert(SeqList *list, size_t pos, int value);

/**
 * @brief 删除第 pos 个位置的元素并带回其值
 * @param list  表指针，不可为 NULL
 * @param pos   删除位置，合法范围 [1, length]
 * @param value 出参，带回被删元素；可为 NULL（不关心值时）
 * @return DS_OK 成功; DS_NULL_PTR list 为 NULL;
 *         DS_EMPTY 表为空; DS_OUT_OF_RANGE pos 非法
 * @note 删除点之后元素整体前移一格，O(n)
 */
DsResult seqlist_delete(SeqList *list, size_t pos, int *value);

/**
 * @brief 取第 pos 个元素的值
 * @param list  表指针，不可为 NULL
 * @param pos   元素位置，合法范围 [1, length]
 * @param value 出参，带回元素值，不可为 NULL
 * @return DS_OK 成功; DS_NULL_PTR 参数为 NULL;
 *         DS_EMPTY 表为空; DS_OUT_OF_RANGE pos 非法
 * @note 按位存取 O(1)
 */
DsResult seqlist_get(const SeqList *list, size_t pos, int *value);

/**
 * @brief 修改第 pos 个元素的值
 * @param list  表指针，不可为 NULL
 * @param pos   元素位置，合法范围 [1, length]
 * @param value 新值
 * @return DS_OK 成功; DS_NULL_PTR list 为 NULL;
 *         DS_EMPTY 表为空; DS_OUT_OF_RANGE pos 非法
 * @note 按位定位 O(1)
 */
DsResult seqlist_set(SeqList *list, size_t pos, int value);

/**
 * @brief 查找首个等于 value 的元素位置
 * @param list  表指针，不可为 NULL
 * @param value 待查找值
 * @param pos   出参，带回位置（从 1 开始；未找到时置 0），不可为 NULL
 * @return DS_OK 找到; DS_ERROR 未找到; DS_NULL_PTR 参数为 NULL
 * @note 顺序查找 O(n)
 */
DsResult seqlist_find(const SeqList *list, int value, size_t *pos);

/**
 * @brief 打印顺序表全部元素与容量信息
 * @param list 表指针，不可为 NULL
 * @return DS_OK 成功; DS_NULL_PTR list 为 NULL
 * @note 调试用展示接口，O(n)
 */
DsResult seqlist_print(const SeqList *list);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* DYNAMIC_SEQ_LIST_H */
