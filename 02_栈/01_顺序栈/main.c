/**
 * @file main.c
 * @brief 顺序栈模块断言测试与演示
 *
 * 测试策略: 正常路径验证后进先出与栈顶约定；边界路径覆盖
 * 空栈取元素、满栈再压、空指针参数；最后 destroy 复位复验。
 */

#include <assert.h>
#include <stdio.h>

#include "seq_stack.h"

int main(void)
{
    SeqStack stack;
    int value = 0;
    int i = 0;

    printf("=== 顺序栈模块（定长数组实现） ===\n\n");

    /* 用例1: 初始化后应为空栈且未满 */
    assert(stack_init(&stack).code == DS_OK);
    assert(stack_is_empty(&stack) == 1);
    assert(stack_is_full(&stack) == 0);
    assert(stack_size(&stack) == 0);

    /* 用例2: 空栈 pop/peek 应返回 DS_EMPTY */
    assert(stack_pop(&stack, &value).code == DS_EMPTY);
    assert(stack_peek(&stack, &value).code == DS_EMPTY);

    /* 用例3: 压入 3 个元素后 size/peek 应反映最后入栈者 */
    assert(stack_push(&stack, 10).code == DS_OK);
    assert(stack_push(&stack, 20).code == DS_OK);
    assert(stack_push(&stack, 30).code == DS_OK);
    assert(stack_size(&stack) == 3);
    assert(stack_is_empty(&stack) == 0);
    assert(stack_peek(&stack, &value).code == DS_OK);
    assert(value == 30);

    /* 用例4: 弹出顺序应与压入相反（后进先出） */
    assert(stack_pop(&stack, &value).code == DS_OK && value == 30);
    assert(stack_pop(&stack, &value).code == DS_OK && value == 20);
    assert(stack_pop(&stack, &value).code == DS_OK && value == 10);
    assert(stack_is_empty(&stack) == 1);

    /* 用例5: 空指针参数应返回 DS_NULL_PTR */
    assert(stack_push(NULL, 1).code == DS_NULL_PTR);
    assert(stack_pop(NULL, &value).code == DS_NULL_PTR);
    assert(stack_pop(&stack, NULL).code == DS_NULL_PTR);
    assert(stack_peek(NULL, &value).code == DS_NULL_PTR);
    assert(stack_peek(&stack, NULL).code == DS_NULL_PTR);
    assert(stack_init(NULL).code == DS_NULL_PTR);

    /* 用例6: 压满 SEQSTACK_MAX_SIZE 个后再压应返回 DS_OVERFLOW */
    for (i = 0; i < SEQSTACK_MAX_SIZE; i++) {
        assert(stack_push(&stack, i + 1).code == DS_OK);
    }
    assert(stack_is_full(&stack) == 1);
    assert(stack_size(&stack) == (size_t)SEQSTACK_MAX_SIZE);
    assert(stack_push(&stack, 999).code == DS_OVERFLOW);

    /* 用例7: 弹出 1 个后不再满，可再次压入 */
    assert(stack_pop(&stack, &value).code == DS_OK && value == SEQSTACK_MAX_SIZE);
    assert(stack_is_full(&stack) == 0);
    assert(stack_push(&stack, 888).code == DS_OK);
    assert(stack_peek(&stack, &value).code == DS_OK && value == 888);

    /* 用例8: destroy 复位后应回到空栈（传 NULL 亦安全） */
    stack_destroy(&stack);
    assert(stack_is_empty(&stack) == 1);
    assert(stack_size(&stack) == 0);
    stack_destroy(NULL);

    /* --- 演示段落：进栈 1~5 后逐个弹出，观察后进先出 --- */
    printf("--- 演示：进栈 1 2 3 4 5 后逐个出栈 ---\n");
    assert(stack_init(&stack).code == DS_OK);
    for (i = 1; i <= 5; i++) {
        assert(stack_push(&stack, i).code == DS_OK);
        printf("进栈 %d（当前栈长 %lu）\n", i, (unsigned long)stack_size(&stack));
    }
    while (stack_pop(&stack, &value).code == DS_OK) {
        printf("出栈 %d\n", value);
    }
    stack_destroy(&stack);

    printf("\n全部测试通过\n");

    return 0;
}
