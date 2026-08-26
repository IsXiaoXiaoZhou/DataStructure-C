/**
 * @file shared_stack.h
 * @brief 两栈共享同一存储空间的接口定义
 *
 * 存储结构: 一段定长数组被两个栈从两端相向使用
 *   栈1: data[0..top1]   从左端向右增长（top1 初始 -1）
 *   栈2: data[top2..MAX-1] 从右端向左增长（top2 初始 MAX）
 *   两栈之间的开区间 (top1, top2) 为共用空闲区
 *
 * 设计要点:
 *   1. 为什么要共用一段空间 —— 两个独立定长栈各占 MAX 格时，
 *      任一栈满即报溢出，哪怕另一栈全空；共用方案下只有整个
 *      数组真正用尽（top1 + 1 == top2）才报满，两栈容量此消
 *      彼长、互相调剂，在两栈规模互补的场景显著省内存
 *   2. 满判定 top1 + 1 == top2 必须先于写入执行 —— 保证两栈
 *      的元素区间永不交叉覆盖，这是本结构唯一要守住的不变量
 *   3. 定长数组 + 两个栈顶下标，存储布局本身就是教学重点，
 *      按规范例外条款直接在头文件暴露结构体
 * 复杂度: 进栈/出栈/取栈顶/判空/判满均为 O(1)
 */

#ifndef SHARED_STACK_H
#define SHARED_STACK_H

#include <stddef.h>

/* 状态码（统一六值版，未用到的成员也保留） */
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

/* 共享数组总容量（编译期定长，两栈容量的和的上界） */
#define SHAREDSTACK_MAX_SIZE 100

/* 共享栈：栈1占左端（top1 初始 -1），栈2占右端（top2 初始 MAX） */
typedef struct {
    int data[SHAREDSTACK_MAX_SIZE]; /* 共用数据区 */
    int top1;                       /* 栈1栈顶元素下标，栈1空为 -1 */
    int top2;                       /* 栈2栈顶元素下标，栈2空为 MAX */
} SharedStack;

DsResult stack_init(SharedStack *stack);
void stack_destroy(SharedStack *stack);
DsResult stack_push(SharedStack *stack, int stack_id, int value);
DsResult stack_pop(SharedStack *stack, int stack_id, int *value);
DsResult stack_peek(const SharedStack *stack, int stack_id, int *value);
int stack_is_empty(const SharedStack *stack, int stack_id);
int stack_is_full(const SharedStack *stack);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* SHARED_STACK_H */
