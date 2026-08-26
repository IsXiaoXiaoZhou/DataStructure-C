/**
 * @file main.c
 * @brief 表达式求值模块测试与演示
 *
 * 测试策略: 每个用例先验证 infix_to_postfix 成功，再验证
 * eval_postfix 的计算结果；错误场景验证状态码与错误传播，
 * 并覆盖输出超长、数值超界两类 DS_OVERFLOW 场景。
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "expression_eval.h"

int main(void)
{
    char postfix[EXPR_MAX_LEN];
    int result;

    printf("=== 表达式求值模块（中缀转后缀 + 后缀求值） ===\n\n");

    /* 用例1: 无括号时乘法优先级高于加法 —— "3+5*2" 应得 13 */
    assert(infix_to_postfix("3+5*2", postfix).code == DS_OK);
    assert(strcmp(postfix, "3 5 2 * +") == 0);
    assert(eval_postfix(postfix, &result).code == DS_OK);
    assert(result == 13);

    /* 用例2: 括号强制改变运算顺序 —— " (10-4)/3" 应得 2 */
    assert(infix_to_postfix(" (10-4)/3", postfix).code == DS_OK);
    assert(eval_postfix(postfix, &result).code == DS_OK);
    assert(result == 2);

    /* 用例3: 括号提升加法优先级 —— "2*(3+4)" 应得 14 */
    assert(infix_to_postfix("2*(3+4)", postfix).code == DS_OK);
    assert(strcmp(postfix, "2 3 4 + *") == 0);
    assert(eval_postfix(postfix, &result).code == DS_OK);
    assert(result == 14);

    /* 用例4: 同优先级左结合 —— "8-3-2" 应得 3 而非 7 */
    assert(infix_to_postfix("8-3-2", postfix).code == DS_OK);
    assert(eval_postfix(postfix, &result).code == DS_OK);
    assert(result == 3);

    /* 用例5: 除数为零应返回 DS_ERROR —— "10/(5-5)" */
    assert(infix_to_postfix("10/(5-5)", postfix).code == DS_OK);
    assert(eval_postfix(postfix, &result).code == DS_ERROR);

    /* 用例6: 缺右括号转换阶段即报 DS_ERROR */
    assert(infix_to_postfix("2*(3+4", postfix).code == DS_ERROR);

    /* 用例7: 多余右括号同样报 DS_ERROR */
    assert(infix_to_postfix("2+3)", postfix).code == DS_ERROR);

    /* 用例8: 非法字符报 DS_ERROR */
    assert(infix_to_postfix("3+a", postfix).code == DS_ERROR);

    /* 用例9: 空指针参数报 DS_NULL_PTR */
    assert(infix_to_postfix(NULL, postfix).code == DS_NULL_PTR);
    assert(eval_postfix("3 4 +", NULL).code == DS_NULL_PTR);

    /* 用例10: 多层嵌套括号 —— "((15/(7-(1+1)))*3)-(2+(1+1))" 应得 5 */
    assert(infix_to_postfix("((15/(7-(1+1)))*3)-(2+(1+1))", postfix).code == DS_OK);
    assert(eval_postfix(postfix, &result).code == DS_OK);
    assert(result == 5);

    /* 用例11: 超长表达式（300 字符）—— 后缀输出写满 EXPR_MAX_LEN
       缓冲区，应返回 DS_OVERFLOW 而非越界写崩溃 */
    {
        char huge[301];
        memset(huge, '1', sizeof huge - 1);
        huge[sizeof huge - 1] = '\0';
        assert(infix_to_postfix(huge, postfix).code == DS_OVERFLOW);
    }

    /* 用例12: 超大数字 —— "99999999999"（11 位）超出 INT_MAX，
       eval_postfix 的累乘溢出检测应返回 DS_OVERFLOW */
    assert(infix_to_postfix("99999999999", postfix).code == DS_OK);
    assert(eval_postfix(postfix, &result).code == DS_OVERFLOW);

    /* --- 演示段落：展示转换中间结果与最终值 --- */
    printf("--- 演示：中缀式 -> 后缀式 -> 计算结果 ---\n");

    {
        const char *samples[] = {
            "3+5*2",
            " (10-4)/3",
            "2*(3+4)",
            "((15/(7-(1+1)))*3)-(2+(1+1))"
        };
        size_t i;
        for (i = 0; i < sizeof(samples) / sizeof(samples[0]); i++) {
            assert(infix_to_postfix(samples[i], postfix).code == DS_OK);
            assert(eval_postfix(postfix, &result).code == DS_OK);
            printf("%-32s => %-24s = %d\n", samples[i], postfix, result);
        }
    }

    printf("\n全部测试通过\n");
    return 0;
}
