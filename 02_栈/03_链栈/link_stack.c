/**
 * @file link_stack.c
 * @brief 链栈（单链表头插实现）接口实现
 *
 * 实现说明:
 *   1. 栈头仅存 top 指针与 size 计数；结点 {value, next}，
 *      next 恒指向更靠近栈底的结点，链尾（栈底）next 为 NULL
 *   2. 进栈 = 头插，出栈 = 删头：单链表头部操作天然 O(1)，
 *      这正是链栈不设头结点也不设尾指针的原因
 */

#include <stdlib.h>

#include "link_stack.h"


/* 栈结点：value 存元素，next 指向更靠近栈底的结点 */
typedef struct LinkStackNode {
    int value;
    struct LinkStackNode *next;
} LinkStackNode;

/* 栈头：top 指向栈顶结点（空栈为 NULL），size 维护元素计数 */
struct LinkStack {
    LinkStackNode *top;
    size_t size;
};

/**
 * @brief 分配并初始化一个值为 value 的栈结点
 * @return 成功返回新结点指针，内存不足返回 NULL
 */
static LinkStackNode *node_new(int value)
{
    LinkStackNode *node = malloc(sizeof *node);

    if (node != NULL) {
        node->value = value;
        node->next = NULL;
    }

    return node;
}

DsResult stack_init(LinkStack **stack)
{
    LinkStack *new_stack = NULL;

    if (stack == NULL) {
        return (DsResult){DS_NULL_PTR, "栈二级指针为空"};
    }

    new_stack = malloc(sizeof *new_stack);
    if (new_stack == NULL) {
        return (DsResult){DS_OVERFLOW, "栈头内存分配失败"};
    }

    new_stack->top = NULL;
    new_stack->size = 0;
    *stack = new_stack;

    return (DsResult){DS_OK, "链栈创建成功"};
}

void stack_destroy(LinkStack **stack)
{
    LinkStackNode *node = NULL;
    LinkStackNode *next = NULL;

    if (stack == NULL || *stack == NULL) {
        return;
    }

    /*
     * 注意：必须先用 next 暂存后继再 free 当前结点，
     * 结点一旦释放，读 node->next 即访问已释放内存（悬垂指针）
     */
    node = (*stack)->top;
    while (node != NULL) {
        next = node->next;
        free(node);
        node = next;
    }

    free(*stack);  /* 结点全部归还后，最后释放栈头本身 */
    *stack = NULL; /* 置空指针，防止调用方误用已释放的栈头 */
}

DsResult stack_push(LinkStack *stack, int value)
{
    LinkStackNode *node = NULL;

    if (stack == NULL) {
        return (DsResult){DS_NULL_PTR, "栈指针为空"};
    }

    node = node_new(value);
    if (node == NULL) {
        return (DsResult){DS_OVERFLOW, "结点内存分配失败"};
    }

    /* 头插两步：新结点先接管原栈顶，top 再改指新结点 */
    node->next = stack->top;
    stack->top = node;
    stack->size++;

    return (DsResult){DS_OK, "压栈成功"};
}

DsResult stack_pop(LinkStack *stack, int *value)
{
    LinkStackNode *node = NULL;

    if (stack == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (stack->top == NULL) {
        return (DsResult){DS_EMPTY, "栈为空，无法弹出"};
    }

    /* 先摘下栈顶结点并取值、修正 top，最后才释放其内存 */
    node = stack->top;
    *value = node->value;
    stack->top = node->next;
    stack->size--;
    free(node);

    return (DsResult){DS_OK, "出栈成功"};
}

DsResult stack_peek(const LinkStack *stack, int *value)
{
    if (stack == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (stack->top == NULL) {
        return (DsResult){DS_EMPTY, "栈为空，无法取栈顶"};
    }

    *value = stack->top->value;

    return (DsResult){DS_OK, "取栈顶成功"};
}

int stack_is_empty(const LinkStack *stack)
{
    return stack == NULL || stack->top == NULL;
}

size_t stack_size(const LinkStack *stack)
{
    return stack == NULL ? 0 : stack->size;
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
