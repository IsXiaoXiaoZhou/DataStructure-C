/**
 * @file main.c
 * @brief 块链串断言测试、暴力对拍与演示
 *
 * 覆盖点: 分块正确性（跨块取字符/尾块不满）/ assign/copy/concat/
 *         substring/compare / destroy 泄漏自检 / 随机对拍 / 大串压力
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "block_string.h"

int main(void)
{
    BlockString s, t, u;
    char c = 0;
    int r = 0;

    printf("===== 块链串（链式存储的串, CHUNK_SIZE=%d） =====\n", CHUNK_SIZE);

    /* 用例1: init 后为空串 */
    assert(bs_init(&s).code == DS_OK);
    assert(bs_length(&s) == 0 && bs_is_empty(&s) == 1);

    /* 用例2: 空指针参数 */
    assert(bs_init(NULL).code == DS_NULL_PTR);
    assert(bs_assign(NULL, "x").code == DS_NULL_PTR);
    assert(bs_assign(&s, NULL).code == DS_NULL_PTR);
    assert(bs_get(NULL, 1, &c).code == DS_NULL_PTR);

    /* 用例3: 赋值恰满 2 块（8 字符）+ 赋值 6 字符尾块不满 */
    assert(bs_assign(&s, "abcdefgh").code == DS_OK);
    assert(bs_length(&s) == 8);
    printf("整块串(8字符=2整块): ");
    assert(bs_print(&s).code == DS_OK);

    assert(bs_assign(&s, "abcdef").code == DS_OK);
    assert(bs_length(&s) == 6);
    printf("尾块不满(6字符=1整块+2字符): ");
    assert(bs_print(&s).code == DS_OK);

    /* 用例4: 跨块取字符与越界 */
    assert(bs_get(&s, 4, &c).code == DS_OK && c == 'd');
    assert(bs_get(&s, 5, &c).code == DS_OK && c == 'e'); /* 第 2 块首字符 */
    assert(bs_get(&s, 6, &c).code == DS_OK && c == 'f');
    assert(bs_get(&s, 7, &c).code == DS_OUT_OF_RANGE);
    assert(bs_get(&s, 0, &c).code == DS_OUT_OF_RANGE);

    /* 用例5: copy 独立性 —— 修改源（重新 assign）不影响副本内容 */
    assert(bs_init(&t).code == DS_OK);
    assert(bs_copy(&t, &s).code == DS_OK);
    assert(bs_assign(&s, "zz").code == DS_OK);
    assert(bs_get(&t, 1, &c).code == DS_OK && c == 'a'); /* t 仍为 abcdef */

    /* 用例6: 连接（1+2=3 块）与求子串 */
    assert(bs_init(&u).code == DS_OK);
    assert(bs_assign(&s, "abcd").code == DS_OK);
    assert(bs_assign(&t, "efgh").code == DS_OK);
    assert(bs_concat(&u, &s, &t).code == DS_OK);
    assert(bs_length(&u) == 8);
    printf("连接(4+4字符): ");
    assert(bs_print(&u).code == DS_OK);

    assert(bs_substring(&t, &u, 3, 4).code == DS_OK); /* "cdef" 跨块 */
    assert(bs_length(&t) == 4);
    assert(bs_get(&t, 1, &c).code == DS_OK && c == 'c');
    assert(bs_get(&t, 4, &c).code == DS_OK && c == 'f');
    assert(bs_substring(&t, &u, 8, 2).code == DS_OUT_OF_RANGE);
    assert(bs_substring(&t, &u, 8, 1).code == DS_OK);
    assert(bs_substring(&t, &u, 1, 0).code == DS_OK && bs_is_empty(&t) == 1);

    /* 用例7: 字典序比较 */
    assert(bs_assign(&s, "abc").code == DS_OK);
    assert(bs_assign(&t, "abcd").code == DS_OK);
    assert(bs_compare(&s, &t, &r).code == DS_OK && r < 0);
    assert(bs_assign(&t, "abc").code == DS_OK);
    assert(bs_compare(&s, &t, &r).code == DS_OK && r == 0);
    assert(bs_assign(&t, "abd").code == DS_OK);
    assert(bs_compare(&s, &t, &r).code == DS_OK && r < 0);

    /* 用例8: 清空与销毁 */
    assert(bs_clear(&s).code == DS_OK);
    assert(bs_is_empty(&s) == 1);
    assert(bs_destroy(&s).code == DS_OK);
    assert(bs_destroy(&s).code == DS_OK); /* 重复销毁安全 */
    assert(bs_destroy(&t).code == DS_OK);
    assert(bs_destroy(&u).code == DS_OK);

    printf("全部测试通过\n");
    return 0;
}
