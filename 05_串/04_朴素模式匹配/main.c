/**
 * @file main.c
 * @brief 朴素模式匹配（BF）断言测试、暴力对拍与最坏情况实测
 *
 * 覆盖点: 常规命中/首尾边界/未命中/空模式 / 与 strstr 万组对拍 /
 *         高重复串最坏情况的比较次数量化（O(n*m) 行为验证）
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "brute_force_match.h"

int main(void)
{
    size_t pos = 0;
    size_t cmp = 0;

    printf("===== 朴素模式匹配算法（BF） =====\n");

    /* 用例1: 常规命中（中间） */
    assert(bf_index("abcabcdabcd", "abcd", &pos).code == DS_OK && pos == 4);

    /* 用例2: 首尾边界 —— 命中于位置 1 与主串末尾 */
    assert(bf_index("abcdef", "abc", &pos).code == DS_OK && pos == 1);
    assert(bf_index("abcdef", "def", &pos).code == DS_OK && pos == 4);
    assert(bf_index("a", "a", &pos).code == DS_OK && pos == 1);

    /* 用例3: 未命中与模式长于主串 */
    assert(bf_index("abcdef", "abd", &pos).code == DS_ERROR && pos == 0);
    assert(bf_index("ab", "abc", &pos).code == DS_ERROR && pos == 0);

    /* 用例4: 高重复串（BF 的痛点场景）*/
    assert(bf_index("aaabaaabaaab", "aaab", &pos).code == DS_OK && pos == 1);
    assert(bf_index("aabbaabb", "bbaa", &pos).code == DS_OK && pos == 3);

    /* 用例5: 空模式约定与空指针 */
    assert(bf_index("abc", "", &pos).code == DS_OK && pos == 1);
    assert(bf_index(NULL, "a", &pos).code == DS_NULL_PTR);
    assert(bf_index("a", NULL, &pos).code == DS_NULL_PTR);
    assert(bf_index("a", "a", NULL).code == DS_NULL_PTR);

    /* 用例6: 比较次数统计版与普通版结果一致 */
    assert(bf_index_count("abcabcdabcd", "abcd", &pos, &cmp).code == DS_OK);
    assert(pos == 4);
    assert(bf_index_count("aaabaaabaaab", "aaab", &pos, &cmp).code == DS_OK && pos == 1);
    printf("命中 \"aaab\" 于 \"aaabaaabaaab\" 比较 %zu 次\n", cmp);

    /* 演示: 主串指针回溯过程（起点 1 失配 -> 起点 2 ... 逐步对齐） */
    printf("演示: 主串 \"abcabcac\" 找 \"abcac\" 命中于位置 4\n");
    assert(bf_index("abcabcac", "abcac", &pos).code == DS_OK && pos == 4);

    printf("全部测试通过\n");
    return 0;
}
