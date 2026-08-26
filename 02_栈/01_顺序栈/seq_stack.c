/**
 * @file seq_stack.c
 * @brief 顺序栈（定长数组实现）接口实现
 *
 * 实现说明:
 *   1. 空栈约定 top == -1：进栈先 ++top 再写 data[top]，
 *      出栈先读 data[top] 再 --top，读写顺序皆以"top 指向
 *      栈顶元素"这一约定为前提，不可颠倒
 *   2. 所有会失败的操作先做空指针检查、再查栈状态，
 *      错误码在破坏数据前即时返回
 */

#include "seq_stack.h"


DsResult stack_init(SeqStack *stack)
{
    if (stack == NULL) {
        return (DsResult){DS_NULL_PTR, "栈指针为空"};
    }

    stack->top = -1;

    return (DsResult){DS_OK, "栈初始化成功"};
}

void stack_destroy(SeqStack *stack)
{
    if (stack == NULL) {
        return;
    }

    /*
     * 静态数组无内存可释放：复位 top 后残留数据在逻辑上
     * 已不可达，无需逐格清零（非安全清零场景）
     */
    stack->top = -1;
}

DsResult stack_push(SeqStack *stack, int value)
{
    if (stack == NULL) {
        return (DsResult){DS_NULL_PTR, "栈指针为空"};
    }
    if (stack->top == SEQSTACK_MAX_SIZE - 1) {
        return (DsResult){DS_OVERFLOW, "栈已满，无法压入"};
    }

    /* top 先自增指向新栈顶格，再存值 —— 与判满条件配套 */
    stack->data[++stack->top] = value;

    return (DsResult){DS_OK, "压栈成功"};
}

DsResult stack_pop(SeqStack *stack, int *value)
{
    if (stack == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (stack->top == -1) {
        return (DsResult){DS_EMPTY, "栈为空，无法弹出"};
    }

    /* 注意：必须先取值再下移 top，顺序颠倒会读到栈外元素 */
    *value = stack->data[stack->top];
    stack->top--;

    return (DsResult){DS_OK, "出栈成功"};
}

DsResult stack_peek(const SeqStack *stack, int *value)
{
    if (stack == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (stack->top == -1) {
        return (DsResult){DS_EMPTY, "栈为空，无法取栈顶"};
    }

    *value = stack->data[stack->top];

    return (DsResult){DS_OK, "取栈顶成功"};
}

int stack_is_empty(const SeqStack *stack)
{
    return stack == NULL || stack->top == -1;
}

int stack_is_full(const SeqStack *stack)
{
    return stack != NULL && stack->top == SEQSTACK_MAX_SIZE - 1;
}

size_t stack_size(const SeqStack *stack)
{
    /* 空栈 top == -1，加 1 后折算为 0，无需特判 */
    return stack == NULL ? 0 : (size_t)(stack->top + 1);
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
