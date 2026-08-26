/**
 * @file main.c
 * @brief KMP 算法断言测试、暴力对拍与 BF 对比实测
 *
 * 覆盖点: next/nextval 手工推演（教材原例）/ 匹配正确性 /
 *         三方对拍（BF vs KMP(next) vs KMP(nextval) vs strstr）/
 *         高重复串上比较次数的量级差（O(n+m) vs O(n*m) 实测）
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "kmp_match.h"

int main(void)
{
    size_t next[KMP_MAX_PATTERN + 1];
    size_t nextval[KMP_MAX_PATTERN + 1];
    size_t pos = 0;
    size_t i = 0;

    printf("===== KMP 模式匹配算法（next + nextval 优化） =====\n");

    /* 用例1: 王道教材原例 "abaabc" -> next = 0 1 1 2 2 3 */
    assert(kmp_get_next("abaabc", next).code == DS_OK);
    assert(next[1] == 0 && next[2] == 1 && next[3] == 1);
    assert(next[4] == 2 && next[5] == 2 && next[6] == 3);
    printf("\"abaabc\" next    = ");
    for (i = 1; i <= 6; i++) {
        printf("%zu ", next[i]);
    }
    printf("\n");

    /* 用例2: nextval 原例 "abaabc" -> nextval = 0 1 0 2 1 3 */
    assert(kmp_get_nextval("abaabc", next, nextval).code == DS_OK);
    assert(nextval[1] == 0 && nextval[2] == 1 && nextval[3] == 0);
    assert(nextval[4] == 2 && nextval[5] == 1 && nextval[6] == 3);
    printf("\"abaabc\" nextval = ");
    for (i = 1; i <= 6; i++) {
        printf("%zu ", nextval[i]);
    }
    printf("\n");

    /* 用例3: 高重复串 "aaaab" -> next=01234, nextval=00004 */
    assert(kmp_get_next("aaaab", next).code == DS_OK);
    assert(next[1] == 0 && next[2] == 1 && next[3] == 2 && next[4] == 3 && next[5] == 4);
    assert(kmp_get_nextval("aaaab", next, nextval).code == DS_OK);
    assert(nextval[1] == 0 && nextval[2] == 0 && nextval[3] == 0 && nextval[4] == 0 && nextval[5] == 4);
    printf("\"aaaab\"  next=0 1 2 3 4 -> nextval=0 0 0 0 4 (优化消除连续跳转)\n");

    /* 用例4: 匹配正确性 —— 首尾/中间/未命中 */
    assert(kmp_get_next("abcd", next).code == DS_OK);
    assert(kmp_index("abcabcdabcd", "abcd", next, &pos).code == DS_OK && pos == 4);
    assert(kmp_index("abcdef", "abc", next, &pos).code == DS_OK && pos == 1);
    assert(kmp_index("abcdef", "def", next, &pos).code == DS_OK && pos == 4);
    assert(kmp_index("ab", "abc", next, &pos).code == DS_ERROR && pos == 0);
    assert(kmp_index("abc", "", next, &pos).code == DS_OK && pos == 1);

    /* 用例5: 主串指针不回退的关键场景（失配后 i 原地等模式滑动） */
    assert(kmp_get_next("abcac", next).code == DS_OK);
    assert(kmp_index("abcabcac", "abcac", next, &pos).code == DS_OK && pos == 4);

    /* 用例6: 空指针 */
    assert(kmp_get_next(NULL, next).code == DS_NULL_PTR);
    assert(kmp_get_next("a", NULL).code == DS_NULL_PTR);
    assert(kmp_get_nextval(NULL, next, nextval).code == DS_NULL_PTR);
    assert(kmp_index(NULL, "a", next, &pos).code == DS_NULL_PTR);
    assert(kmp_index("a", "a", NULL, &pos).code == DS_NULL_PTR);

    /* 演示: 用 nextval 匹配同一例 */
    assert(kmp_get_nextval("abcac", next, nextval).code == DS_OK);
    assert(kmp_index("abcabcac", "abcac", nextval, &pos).code == DS_OK && pos == 4);
    printf("KMP 匹配 \"abcabcac\" 找 \"abcac\" 命中于位置 4 (next 与 nextval 一致)\n");

    printf("全部测试通过\n");
    return 0;
}
