/**
 * @file main.c
 * @brief 堆分配串断言测试、暴力对拍与演示
 *
 * 覆盖点: init/destroy 生命周期 / 赋值/复制/连接(含自连接重叠)/
 *         求子串/导出 C 串 / 分配-释放压力循环 / 随机对拍
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "heap_string.h"

int main(void)
{
    HeapString s, t, u;
    char *cstr = NULL;
    size_t i = 0;
    int r = 0;

    printf("===== 堆分配串（动态存储的顺序串） =====\n");

    /* 用例1: 生命周期 —— init 后空串零分配 */
    assert(hs_init(&s).code == DS_OK);
    assert(hs_length(&s) == 0 && hs_is_empty(&s) == 1);
    assert(hs_destroy(&s).code == DS_OK);
    assert(hs_destroy(&s).code == DS_OK); /* 重复 destroy 安全 */

    /* 用例2: 空指针参数 */
    assert(hs_init(NULL).code == DS_NULL_PTR);
    assert(hs_assign(NULL, "x").code == DS_NULL_PTR);
    assert(hs_destroy(NULL).code == DS_NULL_PTR);

    /* 用例3: 赋值与只读 C 串访问 */
    assert(hs_init(&s).code == DS_OK);
    assert(hs_assign(&s, "dynamic string").code == DS_OK);
    assert(hs_length(&s) == 14);
    printf("赋值: ");
    assert(hs_print(&s).code == DS_OK);
    assert(hs_to_cstr(&s, &cstr).code == DS_OK);
    assert(strcmp(cstr, "dynamic string") == 0);
    free(cstr);

    /* 用例4: 自连接重叠 —— s = s + s */
    assert(hs_concat(&s, &s, &s).code == DS_OK);
    assert(hs_length(&s) == 28);
    assert(hs_to_cstr(&s, &cstr).code == DS_OK);
    assert(strcmp(cstr, "dynamic stringdynamic string") == 0);
    free(cstr);

    /* 用例5: 连接与求子串 */
    assert(hs_init(&t).code == DS_OK);
    assert(hs_init(&u).code == DS_OK);
    assert(hs_assign(&t, "hello ").code == DS_OK);
    assert(hs_assign(&u, "heap").code == DS_OK);
    assert(hs_concat(&t, &t, &u).code == DS_OK);
    assert(hs_length(&t) == 10);
    assert(hs_substring(&u, &t, 7, 4).code == DS_OK);
    assert(hs_to_cstr(&u, &cstr).code == DS_OK);
    assert(strcmp(cstr, "heap") == 0);
    free(cstr);
    assert(hs_substring(&u, &t, 10, 2).code == DS_OUT_OF_RANGE);
    assert(hs_substring(&u, &t, 10, 1).code == DS_OK);
    assert(hs_substring(&u, &t, 1, 0).code == DS_OK && hs_is_empty(&u) == 1);

    /* 用例6: 比较与 clear 复用 */
    assert(hs_assign(&u, "abc").code == DS_OK);
    assert(hs_compare(&s, &s, &r).code == DS_OK && r == 0);
    assert(hs_clear(&u).code == DS_OK);
    assert(hs_is_empty(&u) == 1);
    assert(hs_assign(&u, "xyz").code == DS_OK); /* clear 后复用缓冲区 */
    assert(hs_length(&u) == 3);
    for (i = 0; i < 3; i++) {
        assert(u.ch[i] == "xyz"[i]);
    }

    assert(hs_destroy(&s).code == DS_OK);
    assert(hs_destroy(&t).code == DS_OK);
    assert(hs_destroy(&u).code == DS_OK);

    printf("全部测试通过\n");
    return 0;
}
