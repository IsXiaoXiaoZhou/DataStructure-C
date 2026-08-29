/**
 * @file seq_stack.h
 * @brief 顺序栈（定长数组实现）接口定义
 *
 * 存储结构: data[0..top] 连续存放自栈底到栈顶的元素
 *   top ---- 指向栈顶元素（空栈约定 top == -1）
 *
 * 设计要点:
 *   1. top 指向栈顶元素而非下一个空位 —— 与"top 指向空位、
 *      初始为 0"的教材变体相比，本约定下判空条件是 top == -1，
 *      入栈先自增再存值，出栈先取值再自减，两种约定不可混用
 *   2. 定长数组 + 栈顶下标，属于无不变量需要保护的定长实现，
 *      按规范例外条款直接在头文件暴露结构体 —— 教学上让
 *      "栈顶下标随进栈出栈而移动"的存储形象一目了然
 *   3. 编译期确定容量，无动态内存，destroy 仅做复位清空
 * 复杂度: 进栈/出栈/取栈顶/判空/判满/求长均为 O(1)
 */

#ifndef SEQ_STACK_H
#define SEQ_STACK_H

#include <stddef.h>

/* 状态码（统一七值版，未用到的成员也保留） */
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

/* 顺序栈最大容量（编译期定长） */
#define SEQSTACK_MAX_SIZE 100

/* 顺序栈：定长数据区 + 栈顶下标 */
typedef struct {
    int data[SEQSTACK_MAX_SIZE]; /* 数据区：data[top] 即栈顶元素 */
    int top;                     /* 栈顶元素下标，空栈为 -1 */
} SeqStack;

DsResult stack_init(SeqStack *stack);
void stack_destroy(SeqStack *stack);
DsResult stack_push(SeqStack *stack, int value);
DsResult stack_pop(SeqStack *stack, int *value);
DsResult stack_peek(const SeqStack *stack, int *value);
int stack_is_empty(const SeqStack *stack);
int stack_is_full(const SeqStack *stack);
size_t stack_size(const SeqStack *stack);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* SEQ_STACK_H */
