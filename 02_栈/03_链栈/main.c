/**
 * @file main.c
 * @brief 链栈模块断言测试与演示
 *
 * 测试策略: 正常路径验证后进先出；边界路径覆盖空栈取元素与
 * 空指针参数；压入 200 个元素（超出顺序栈容量上限）验证链栈
 * 无满栈概念；最后验证 destroy 的安全性（允许 NULL）与可复用性。
 */

#include <assert.h>
#include <stdio.h>

#include "link_stack.h"

int main(void)
{
    LinkStack *stack = NULL;
    int value = 0;
    int i = 0;

    printf("=== 链栈模块（单链表头插实现） ===\n\n");

    /* 用例1: init 成功创建空栈 */
    assert(stack_init(&stack).code == DS_OK);
    assert(stack != NULL);
    assert(stack_is_empty(stack) == 1);
    assert(stack_size(stack) == 0);

    /* 用例2: 空栈 pop/peek 应返回 DS_EMPTY */
    assert(stack_pop(stack, &value).code == DS_EMPTY);
    assert(stack_peek(stack, &value).code == DS_EMPTY);

    /* 用例3: 压入 3 个元素后 size/peek 应反映最后入栈者 */
    assert(stack_push(stack, 10).code == DS_OK);
    assert(stack_push(stack, 20).code == DS_OK);
    assert(stack_push(stack, 30).code == DS_OK);
    assert(stack_size(stack) == 3);
    assert(stack_is_empty(stack) == 0);
    assert(stack_peek(stack, &value).code == DS_OK);
    assert(value == 30);

    /* 用例4: 弹出顺序应与压入相反（后进先出） */
    assert(stack_pop(stack, &value).code == DS_OK && value == 30);
    assert(stack_pop(stack, &value).code == DS_OK && value == 20);
    assert(stack_pop(stack, &value).code == DS_OK && value == 10);
    assert(stack_is_empty(stack) == 1);
    assert(stack_size(stack) == 0);

    /* 用例5: 空指针参数应返回 DS_NULL_PTR */
    assert(stack_init(NULL).code == DS_NULL_PTR);
    assert(stack_push(NULL, 1).code == DS_NULL_PTR);
    assert(stack_pop(NULL, &value).code == DS_NULL_PTR);
    assert(stack_pop(stack, NULL).code == DS_NULL_PTR);
    assert(stack_peek(NULL, &value).code == DS_NULL_PTR);
    assert(stack_peek(stack, NULL).code == DS_NULL_PTR);

    /* 用例6: 链栈无满栈概念——压入 200 个（超顺序栈容量）全部成功 */
    for (i = 1; i <= 200; i++) {
        assert(stack_push(stack, i).code == DS_OK);
    }
    assert(stack_size(stack) == 200);
    assert(stack_peek(stack, &value).code == DS_OK && value == 200);
    assert(stack_pop(stack, &value).code == DS_OK && value == 200);
    assert(stack_size(stack) == 199);

    /* 用例7: destroy 释放全部结点并置 NULL，之后可重新 init 复用 */
    stack_destroy(&stack);
    assert(stack == NULL);
    stack_destroy(NULL); /* 传 NULL 应为安全空操作 */

    /* 用例8: 销毁后重新 init 的栈功能完好 */
    assert(stack_init(&stack).code == DS_OK);
    assert(stack_is_empty(stack) == 1);
    assert(stack_push(stack, 7).code == DS_OK);
    assert(stack_pop(stack, &value).code == DS_OK && value == 7);

    /* --- 演示段落：进栈 1~5 后逐个弹出，观察后进先出 --- */
    printf("--- 演示：进栈 1 2 3 4 5 后逐个出栈 ---\n");
    for (i = 1; i <= 5; i++) {
        assert(stack_push(stack, i).code == DS_OK);
        printf("进栈 %d（当前栈长 %lu）\n", i, (unsigned long)stack_size(stack));
    }
    while (stack_pop(stack, &value).code == DS_OK) {
        printf("出栈 %d\n", value);
    }
    stack_destroy(&stack);
    assert(stack == NULL);

    printf("\n全部测试通过\n");

    return 0;
}
