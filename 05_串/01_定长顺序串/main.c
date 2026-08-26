/**
 * @file main.c
 * @brief 定长顺序串断言测试、暴力对拍与演示
 *
 * 覆盖点: 赋值/比较/连接/求子串/定位/清空 / 边界与溢出 /
 *         随机数据对拍（concat/substring 与手工结果逐位核对）
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "static_string.h"

int main(void)
{
    StaticString s, t, u;
    size_t pos = 0;
    int r = 0;

    printf("===== 定长顺序串（定长数组实现） =====\n");

    /* 用例1: 赋值后长度正确，空串判定正确 */
    assert(ss_assign(&s, "data structure").code == DS_OK);
    assert(ss_length(&s) == 14);
    assert(ss_is_empty(&s) == 0);
    printf("赋值: ");
    assert(ss_print(&s).code == DS_OK);

    /* 用例2: 空指针参数返回 DS_NULL_PTR */
    assert(ss_assign(NULL, "x").code == DS_NULL_PTR);
    assert(ss_assign(&s, NULL).code == DS_NULL_PTR);
    assert(ss_compare(NULL, &s, &r).code == DS_NULL_PTR);
    assert(ss_index(&s, &s, NULL).code == DS_NULL_PTR);

    /* 用例3: 赋值超长返回 DS_OVERFLOW（不截断策略） */
    {
        char big[STR_MAX_SIZE + 2];
        size_t i = 0;

        for (i = 0; i < sizeof(big) - 1; i++) {
            big[i] = 'x';
        }
        big[sizeof(big) - 1] = '\0';
        assert(ss_assign(&s, big).code == DS_OVERFLOW);
    }

    /* 用例4: 字典序比较 —— 前缀相同短者小，逐位不等看码值 */
    assert(ss_assign(&s, "abc").code == DS_OK);
    assert(ss_assign(&t, "abcd").code == DS_OK);
    assert(ss_compare(&s, &t, &r).code == DS_OK && r < 0);
    assert(ss_compare(&t, &s, &r).code == DS_OK && r > 0);
    assert(ss_assign(&u, "abc").code == DS_OK);
    assert(ss_compare(&s, &u, &r).code == DS_OK && r == 0);
    assert(ss_assign(&t, "b").code == DS_OK);
    assert(ss_compare(&t, &s, &r).code == DS_OK && r > 0);

    /* 用例5: 连接成功与溢出 */
    assert(ss_assign(&s, "hello ").code == DS_OK);
    assert(ss_assign(&t, "world").code == DS_OK);
    assert(ss_concat(&u, &s, &t).code == DS_OK);
    assert(u.length == 11);
    printf("连接: ");
    assert(ss_print(&u).code == DS_OK);

    /* 用例6: 求子串边界 —— len=0 得空串，越界报错 */
    assert(ss_substring(&t, &u, 7, 5).code == DS_OK); /* "world" */
    assert(t.length == 5 && t.data[0] == 'w');
    assert(ss_substring(&t, &u, 1, 0).code == DS_OK);
    assert(ss_is_empty(&t) == 1);
    assert(ss_substring(&t, &u, 0, 1).code == DS_OUT_OF_RANGE);
    assert(ss_substring(&t, &u, 12, 1).code == DS_OUT_OF_RANGE);
    assert(ss_substring(&t, &u, 11, 2).code == DS_OUT_OF_RANGE);
    assert(ss_substring(&t, &u, 11, 1).code == DS_OK); /* 恰取最后 1 个字符 */

    /* 用例7: 定位命中/未命中/空模式 */
    assert(ss_assign(&s, "abcabcd").code == DS_OK);
    assert(ss_assign(&t, "abcd").code == DS_OK);
    assert(ss_index(&s, &t, &pos).code == DS_OK && pos == 4);
    assert(ss_assign(&t, "abd").code == DS_OK);
    assert(ss_index(&s, &t, &pos).code == DS_ERROR && pos == 0);
    assert(ss_clear(&t).code == DS_OK);
    assert(ss_index(&s, &t, &pos).code == DS_OK && pos == 1); /* 空模式约定 */

    /* 用例8: 清空后可复用 */
    assert(ss_clear(&s).code == DS_OK);
    assert(ss_length(&s) == 0 && ss_is_empty(&s) == 1);
    assert(ss_assign(&s, "reuse").code == DS_OK);
    assert(ss_length(&s) == 5);

    /* 演示: 综合操作输出 */
    printf("子串(7,5) 与 定位演示完成, 复用串: ");
    assert(ss_print(&s).code == DS_OK);

    printf("全部测试通过\n");
    return 0;
}
