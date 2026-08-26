/**
 * @file shared_stack.c
 * @brief 两栈共享空间接口实现
 *
 * 实现说明:
 *   1. 栈1 的元素落在闭区间 [0, top1]，栈2 落在 [top2, MAX-1]，
 *      共用空闲区是开区间 (top1, top2)；满判定 top1 + 1 == top2
 *      即空闲区长度为 0，此时压任一栈都无格可用
 *   2. 每次 push 都先判满再移动下标 —— 这是保证两栈元素区间
 *      永不交叉覆盖的关键（先动下标后判满就会写进对方区间）
 */

#include "shared_stack.h"


DsResult stack_init(SharedStack *stack)
{
    if (stack == NULL) {
        return (DsResult){DS_NULL_PTR, "共享栈指针为空"};
    }

    /*
     * top1 置 -1、top2 置 MAX：两栈各自"退到端点之外"，
     * 元素区间均为空，中间整段留作共用空闲区
     */
    stack->top1 = -1;
    stack->top2 = SHAREDSTACK_MAX_SIZE;

    return (DsResult){DS_OK, "共享栈初始化成功"};
}

void stack_destroy(SharedStack *stack)
{
    if (stack == NULL) {
        return;
    }

    /* 与 init 相同的复位动作：两栈各自退回端点之外 */
    stack->top1 = -1;
    stack->top2 = SHAREDSTACK_MAX_SIZE;
}

DsResult stack_push(SharedStack *stack, int stack_id, int value)
{
    if (stack == NULL) {
        return (DsResult){DS_NULL_PTR, "共享栈指针为空"};
    }
    if (stack_id != 1 && stack_id != 2) {
        return (DsResult){DS_OUT_OF_RANGE, "栈编号非法，合法值为 1 或 2"};
    }
    if (stack->top1 + 1 == stack->top2) {
        /*
         * 整段数组已用尽：注意满的是"共享空间"而非某个栈，
         * 无论压哪个栈都无格可写，只能整体报溢出
         */
        return (DsResult){DS_OVERFLOW, "共享空间已满，无法压入"};
    }

    if (stack_id == 1) {
        stack->data[++stack->top1] = value; /* 栈1 向右端扩张 */
    } else {
        stack->data[--stack->top2] = value; /* 栈2 向左端扩张 */
    }

    return (DsResult){DS_OK, "压栈成功"};
}

DsResult stack_pop(SharedStack *stack, int stack_id, int *value)
{
    if (stack == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (stack_id != 1 && stack_id != 2) {
        return (DsResult){DS_OUT_OF_RANGE, "栈编号非法，合法值为 1 或 2"};
    }

    if (stack_id == 1) {
        if (stack->top1 == -1) {
            return (DsResult){DS_EMPTY, "栈1为空，无法弹出"};
        }
        /* 注意：先取值再退下标，退回的格子交还共用空闲区 */
        *value = stack->data[stack->top1];
        stack->top1--;
    } else {
        if (stack->top2 == SHAREDSTACK_MAX_SIZE) {
            return (DsResult){DS_EMPTY, "栈2为空，无法弹出"};
        }
        *value = stack->data[stack->top2];
        stack->top2++;
    }

    return (DsResult){DS_OK, "出栈成功"};
}

DsResult stack_peek(const SharedStack *stack, int stack_id, int *value)
{
    if (stack == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (stack_id != 1 && stack_id != 2) {
        return (DsResult){DS_OUT_OF_RANGE, "栈编号非法，合法值为 1 或 2"};
    }

    if (stack_id == 1) {
        if (stack->top1 == -1) {
            return (DsResult){DS_EMPTY, "栈1为空，无法取栈顶"};
        }
        *value = stack->data[stack->top1];
    } else {
        if (stack->top2 == SHAREDSTACK_MAX_SIZE) {
            return (DsResult){DS_EMPTY, "栈2为空，无法取栈顶"};
        }
        *value = stack->data[stack->top2];
    }

    return (DsResult){DS_OK, "取栈顶成功"};
}

int stack_is_empty(const SharedStack *stack, int stack_id)
{
    if (stack_id != 1 && stack_id != 2) {
        return -1; /* 三态返回：-1 专用于报告非法编号 */
    }
    if (stack == NULL) {
        return 1;
    }

    /*
     * 栈1空判 top1 退回 -1，栈2空判 top2 退回 MAX ——
     * 各自退到端点之外即元素区间为空
     */
    return stack_id == 1 ? (stack->top1 == -1)
                         : (stack->top2 == SHAREDSTACK_MAX_SIZE);
}

int stack_is_full(const SharedStack *stack)
{
    return stack != NULL && stack->top1 + 1 == stack->top2;
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
