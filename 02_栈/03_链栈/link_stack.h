/**
 * @file link_stack.h
 * @brief 链栈（单链表头插实现）接口定义
 *
 * 存储结构: top → [an] → [a(n-1)] → ... → [a1] → NULL
 *   top ---- 链表首结点指针，即栈顶；a1 为栈底（最后被弹出）
 *
 * 设计要点:
 *   1. 头插法天然契合后进先出 —— 进栈即在链头插入新结点，
 *      出栈即删除首结点，top 就是链表头指针；既不需要头结点
 *      （不存在"在首结点前插入"的特判需求），也不需要尾指针
 *   2. "top 恒指向栈顶结点、栈空恒为 NULL"是需要维护的不变量，
 *      故采用不透明指针 —— 结构体定义在 .c 中，外部无法绕过
 *      API 直接改链破坏栈序
 *   3. 结点按需 malloc、用完即 free，无满栈概念（故无 is_full），
 *      容量仅受系统内存限制
 * 复杂度: 进栈/出栈/取栈顶/判空/求长均为 O(1)
 */

#ifndef LINK_STACK_H
#define LINK_STACK_H

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

/* 链栈类型（不透明指针：结构体定义在 link_stack.c 中） */
typedef struct LinkStack LinkStack;

DsResult stack_init(LinkStack **stack);
void stack_destroy(LinkStack **stack);
DsResult stack_push(LinkStack *stack, int value);
DsResult stack_pop(LinkStack *stack, int *value);
DsResult stack_peek(const LinkStack *stack, int *value);
int stack_is_empty(const LinkStack *stack);
size_t stack_size(const LinkStack *stack);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* LINK_STACK_H */
