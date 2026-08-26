#ifndef STATIC_SEQ_LIST_H
#define STATIC_SEQ_LIST_H

/**
 * @file static_seq_list.h
 * @brief 静态顺序表（定长数组实现）接口定义
 *
 * 存储结构: data[0..length-1] 连续存放 a1..an
 * 设计要点:
 *   1. 定长数组 + 长度域，属于无不变量需要保护的定长实现，
 *      按规范例外条款直接在头文件暴露结构体 —— 教学上让
 *      "逻辑相邻 ⇔ 物理相邻" 的顺序表核心特征一目了然
 *   2. 编译期确定容量，无动态内存，故无需 destroy
 * 复杂度: 按位存取 O(1)；插入/删除需成块移动元素 O(n)
 */

#include <stddef.h>

/* 状态码 */
typedef enum {
    DS_OK = 0,          /* 操作成功 */
    DS_ERROR,           /* 一般性错误 */
    DS_NULL_PTR,        /* 空指针参数 */
    DS_OUT_OF_RANGE,    /* 位置/下标越界 */
    DS_OVERFLOW,        /* 空间已满或内存分配失败 */
    DS_EMPTY,           /* 对空结构取元素 */
    DS_NOT_FOUND,       /* 查找未命中（业务正常结果，非异常） */
    DS_DUPLICATE        /* 插入重复键 */
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 顺序表最大容量（编译期定长） */
#define SEQLIST_MAX_SIZE 100

/* 静态顺序表：定长数据区 + 当前长度 */
typedef struct {
    int    data[SEQLIST_MAX_SIZE]; /* 数据区：data[0] 存放第 1 个元素 */
    size_t length;                 /* 当前元素个数 */
} SeqList;

/**
 * @brief 初始化顺序表为空表
 * @param list 顺序表指针，不可为 NULL
 * @return DS_OK 成功; DS_NULL_PTR list 为 NULL
 * @note 仅需清零长度域，O(1)
 */
DsResult seqlist_init(SeqList *list);

/**
 * @brief 返回当前元素个数
 * @param list 顺序表指针，不可为 NULL（为 NULL 时返回 0）
 * @return 元素个数
 * @note 长度域直接读取，O(1)
 */
size_t seqlist_length(const SeqList *list);

/**
 * @brief 判断顺序表是否为空
 * @param list 顺序表指针，不可为 NULL（为 NULL 时视为空返回 1）
 * @return 1 空表; 0 非空
 * @note O(1)
 */
int seqlist_is_empty(const SeqList *list);

/**
 * @brief 在第 pos 个位置插入元素（pos 从 1 开始计）
 * @param list  顺序表指针，不可为 NULL
 * @param pos   插入位置，合法范围 [1, length+1]
 * @param value 待插入元素
 * @return DS_OK 成功; DS_NULL_PTR list 为 NULL;
 *         DS_OUT_OF_RANGE pos 非法; DS_OVERFLOW 表已满
 * @note 需将 data[pos-1..length-1] 整体后移一格，从末尾向前移动
 *       防止覆盖，时间复杂度 O(n)
 */
DsResult seqlist_insert(SeqList *list, size_t pos, int value);

/**
 * @brief 删除第 pos 个位置的元素并带回其值
 * @param list  顺序表指针，不可为 NULL
 * @param pos   删除位置，合法范围 [1, length]
 * @param value 出参，带回被删元素；可为 NULL（不关心值时）
 * @return DS_OK 成功; DS_NULL_PTR list 为 NULL;
 *         DS_EMPTY 表为空; DS_OUT_OF_RANGE pos 非法
 * @note 需将 data[pos..length-1] 整体前移一格补洞，O(n)
 */
DsResult seqlist_delete(SeqList *list, size_t pos, int *value);

/**
 * @brief 取第 pos 个元素的值
 * @param list  顺序表指针，不可为 NULL
 * @param pos   元素位置，合法范围 [1, length]
 * @param value 出参，带回元素值，不可为 NULL
 * @return DS_OK 成功; DS_NULL_PTR 参数为 NULL;
 *         DS_EMPTY 表为空; DS_OUT_OF_RANGE pos 非法
 * @note 顺序表按位存取 O(1)，这正是顺序表相对链表的核心优势
 */
DsResult seqlist_get(const SeqList *list, size_t pos, int *value);

/**
 * @brief 修改第 pos 个元素的值
 * @param list  顺序表指针，不可为 NULL
 * @param pos   元素位置，合法范围 [1, length]
 * @param value 新值
 * @return DS_OK 成功; DS_NULL_PTR list 为 NULL;
 *         DS_EMPTY 表为空; DS_OUT_OF_RANGE pos 非法
 * @note 按位定位 O(1)
 */
DsResult seqlist_set(SeqList *list, size_t pos, int value);

/**
 * @brief 查找首个等于 value 的元素位置
 * @param list  顺序表指针，不可为 NULL
 * @param value 待查找值
 * @param pos   出参，带回位置（从 1 开始；未找到时置 0），不可为 NULL
 * @return DS_OK 找到; DS_ERROR 未找到; DS_NULL_PTR 参数为 NULL
 * @note 存储无序只能顺序查找，O(n)
 */
DsResult seqlist_find(const SeqList *list, int value, size_t *pos);

/**
 * @brief 打印顺序表全部元素
 * @param list 顺序表指针，不可为 NULL
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

#endif /* STATIC_SEQ_LIST_H */
