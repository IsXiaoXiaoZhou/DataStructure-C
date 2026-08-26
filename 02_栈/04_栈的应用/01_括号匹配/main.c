/**
 * @file main.c
 * @brief 括号匹配模块断言测试与演示
 *
 * 测试策略: 正确场景覆盖混合嵌套与无括号表达式；三种错误
 * 场景分别验证错误类型与出错位置下标；另测空指针与嵌套
 * 超深（内部栈容量上界）两个异常返回码。
 */

#include <assert.h>
#include <stdio.h>

#include "bracket_match.h"

/**
 * @brief 将结论类型翻译为中文描述（演示打印复用）
 */
static const char *error_desc(BracketStatus type)
{
    switch (type) {
    case BRACKET_OK:
        return "括号完全匹配";
    case BRACKET_MISSING_LEFT:
        return "缺左括号（多右括号）";
    case BRACKET_MISSING_RIGHT:
        return "缺右括号（多左括号）";
    case BRACKET_MISMATCH:
    default:
        return "左右括号类型不匹配";
    }
}

int main(void)
{
    BracketError err;
    char deep_expr[BRACKET_MAX_DEPTH + 2];
    int i = 0;

    printf("=== 括号匹配模块（bracket_match） ===\n\n");

    /* 用例1: 三类括号混合嵌套正确应返回 BRACKET_OK */
    assert(bracket_match("(1+2)*[3-(4/5)]", &err).code == DS_OK);
    assert(err.type == BRACKET_OK);

    assert(bracket_match("{[()()]{}}", &err).code == DS_OK);
    assert(err.type == BRACKET_OK);

    /* 用例2: 空串与不含括号的表达式视为合法 */
    assert(bracket_match("", &err).code == DS_OK && err.type == BRACKET_OK);
    assert(bracket_match("abc + 123", &err).code == DS_OK && err.type == BRACKET_OK);

    /* 用例3: 缺左括号——右括号到来时栈已空，pos 指向该右括号 */
    assert(bracket_match("())", &err).code == DS_OK);
    assert(err.type == BRACKET_MISSING_LEFT);
    assert(err.pos == 2);

    assert(bracket_match(")(", &err).code == DS_OK);
    assert(err.type == BRACKET_MISSING_LEFT);
    assert(err.pos == 0);

    /* 用例4: 缺右括号——扫描结束栈非空，pos 指向最早未配对的左括号 */
    assert(bracket_match("(()", &err).code == DS_OK);
    assert(err.type == BRACKET_MISSING_RIGHT);
    assert(err.pos == 0);

    assert(bracket_match("a{b[c(d)]", &err).code == DS_OK);
    assert(err.type == BRACKET_MISSING_RIGHT);
    assert(err.pos == 1);

    /* 用例5: 类型不匹配——pos 指向引发冲突的右括号 */
    assert(bracket_match("(]", &err).code == DS_OK);
    assert(err.type == BRACKET_MISMATCH);
    assert(err.pos == 1);

    assert(bracket_match("{(})", &err).code == DS_OK);
    assert(err.type == BRACKET_MISMATCH);
    assert(err.pos == 2);

    assert(bracket_match("([)]", &err).code == DS_OK);
    assert(err.type == BRACKET_MISMATCH);
    assert(err.pos == 2);

    /* 用例6: 空指针参数应返回 DS_NULL_PTR */
    assert(bracket_match(NULL, &err).code == DS_NULL_PTR);
    assert(bracket_match("()", NULL).code == DS_NULL_PTR);

    /* 用例7: 嵌套深度超过内部栈容量应返回 DS_OVERFLOW */
    for (i = 0; i < BRACKET_MAX_DEPTH + 1; i++) {
        deep_expr[i] = '(';
    }
    deep_expr[BRACKET_MAX_DEPTH + 1] = '\0';
    assert(bracket_match(deep_expr, &err).code == DS_OVERFLOW);

    /* --- 演示段落：逐个检查并打印结论 --- */
    printf("--- 演示：表达式括号检查 ---\n");
    {
        const char *samples[] = {
            "((a+b)*(c-d))",
            "[(a+b]*(c-d))",
            "((a+b)",
            "(a+b))",
        };
        size_t j;
        for (j = 0; j < sizeof(samples) / sizeof(samples[0]); j++) {
            assert(bracket_match(samples[j], &err).code == DS_OK);
            if (err.type == BRACKET_OK) {
                printf("%-16s => %s\n", samples[j], error_desc(err.type));
            } else {
                printf("%-16s => %s，出错下标 %lu\n",
                       samples[j], error_desc(err.type),
                       (unsigned long)err.pos);
            }
        }
    }

    printf("\n全部测试通过\n");

    return 0;
}
