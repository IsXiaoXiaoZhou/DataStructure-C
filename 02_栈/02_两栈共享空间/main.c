/**
 * @file main.c
 * @brief 两栈共享空间模块断言测试与演示
 *
 * 测试策略: 先验证两栈独立性与非法编号；再用 60+40 恰好挤满
 * 共享区验证判满条件；最后弹出栈2验证腾出的空间可供栈1使用
 * （这正是共享方案相对两个独立定长栈的价值所在）。
 */

#include <assert.h>
#include <stdio.h>

#include "shared_stack.h"

int main(void)
{
    SharedStack stack;
    int value = 0;
    int i = 0;

    printf("=== 两栈共享空间模块（SharedStack） ===\n\n");

    /* 用例1: 初始化后两栈均为空且共享区未满 */
    assert(stack_init(&stack).code == DS_OK);
    assert(stack_is_empty(&stack, 1) == 1);
    assert(stack_is_empty(&stack, 2) == 1);
    assert(stack_is_full(&stack) == 0);

    /* 用例2: 空栈 pop/peek 应返回 DS_EMPTY（两栈分别验证） */
    assert(stack_pop(&stack, 1, &value).code == DS_EMPTY);
    assert(stack_peek(&stack, 1, &value).code == DS_EMPTY);
    assert(stack_pop(&stack, 2, &value).code == DS_EMPTY);
    assert(stack_peek(&stack, 2, &value).code == DS_EMPTY);

    /* 用例3: 栈1压入后栈2仍空，栈2压入后栈1数据不受影响 */
    assert(stack_push(&stack, 1, 11).code == DS_OK);
    assert(stack_push(&stack, 1, 12).code == DS_OK);
    assert(stack_is_empty(&stack, 2) == 1);
    assert(stack_push(&stack, 2, 21).code == DS_OK);
    assert(stack_peek(&stack, 1, &value).code == DS_OK && value == 12);
    assert(stack_peek(&stack, 2, &value).code == DS_OK && value == 21);

    /* 用例4: 两栈各自后进先出，弹出互不干扰 */
    assert(stack_pop(&stack, 1, &value).code == DS_OK && value == 12);
    assert(stack_pop(&stack, 2, &value).code == DS_OK && value == 21);
    assert(stack_pop(&stack, 1, &value).code == DS_OK && value == 11);
    assert(stack_is_empty(&stack, 1) == 1);
    assert(stack_is_empty(&stack, 2) == 1);

    /* 用例5: 非法 stack_id 应返回 DS_OUT_OF_RANGE / -1 */
    assert(stack_push(&stack, 0, 1).code == DS_OUT_OF_RANGE);
    assert(stack_push(&stack, 3, 1).code == DS_OUT_OF_RANGE);
    assert(stack_pop(&stack, 0, &value).code == DS_OUT_OF_RANGE);
    assert(stack_peek(&stack, 3, &value).code == DS_OUT_OF_RANGE);
    assert(stack_is_empty(&stack, 0) == -1);
    assert(stack_is_empty(&stack, 3) == -1);

    /* 用例6: 空指针参数应返回 DS_NULL_PTR */
    assert(stack_init(NULL).code == DS_NULL_PTR);
    assert(stack_push(NULL, 1, 1).code == DS_NULL_PTR);
    assert(stack_pop(NULL, 1, &value).code == DS_NULL_PTR);
    assert(stack_pop(&stack, 1, NULL).code == DS_NULL_PTR);
    assert(stack_peek(NULL, 1, &value).code == DS_NULL_PTR);

    /* 用例7: 栈1占 60 格 + 栈2占 40 格恰好挤满，任一栈再压都溢出 */
    for (i = 0; i < 60; i++) {
        assert(stack_push(&stack, 1, i + 1).code == DS_OK);
    }
    for (i = 0; i < 40; i++) {
        assert(stack_push(&stack, 2, 100 + i).code == DS_OK);
    }
    assert(stack_is_full(&stack) == 1);
    assert(stack_push(&stack, 1, 999).code == DS_OVERFLOW);
    assert(stack_push(&stack, 2, 999).code == DS_OVERFLOW);

    /* 用例8: 栈2弹出腾出的空格可被栈1占用（空间互相调剂） */
    assert(stack_pop(&stack, 2, &value).code == DS_OK && value == 139);
    assert(stack_is_full(&stack) == 0);
    assert(stack_push(&stack, 1, 999).code == DS_OK);
    assert(stack_peek(&stack, 1, &value).code == DS_OK && value == 999);

    /* 用例9: destroy 复位后两栈均回到空栈（传 NULL 亦安全） */
    stack_destroy(&stack);
    assert(stack_is_empty(&stack, 1) == 1);
    assert(stack_is_empty(&stack, 2) == 1);
    assert(stack_is_full(&stack) == 0);
    stack_destroy(NULL);

    /* --- 演示段落：两栈交替进出，观察各自的后进先出 --- */
    printf("--- 演示：栈1进 1 2 3，栈2进 7 8，然后各自全部弹出 ---\n");
    assert(stack_init(&stack).code == DS_OK);
    for (i = 1; i <= 3; i++) {
        assert(stack_push(&stack, 1, i).code == DS_OK);
    }
    for (i = 7; i <= 8; i++) {
        assert(stack_push(&stack, 2, i).code == DS_OK);
    }
    while (stack_pop(&stack, 1, &value).code == DS_OK) {
        printf("栈1出栈 %d\n", value);
    }
    while (stack_pop(&stack, 2, &value).code == DS_OK) {
        printf("栈2出栈 %d\n", value);
    }

    printf("\n全部测试通过\n");

    return 0;
}
