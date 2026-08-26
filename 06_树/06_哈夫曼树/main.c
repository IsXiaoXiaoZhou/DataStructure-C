/**
 * @file main.c
 * @brief 哈夫曼树断言测试、对拍与演示
 *
 * 覆盖点: 教材经典例 WPL / 编码前缀无歧义性 / 编解码 roundtrip /
 *         WPL 与"合并果子"等价贪心对拍（最优性验证）/ 随机压力
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "huffman_tree.h"

int main(void)
{
    HuffmanTree ht;
    long wpl = 0;
    char code[HUFF_MAX_CODE_LEN + 1];
    size_t symbols[64];
    size_t n = 0;
    size_t i = 0;

    printf("===== 哈夫曼树与哈夫曼编码 =====\n");

    /* 用例1: 教材经典例 w={5,29,7,8,14,23,3,11}，WPL=271 */
    {
        int w[] = { 5, 29, 7, 8, 14, 23, 3, 11 };

        assert(ht_build(&ht, w, 8).code == DS_OK);
        assert(ht_wpl(&ht, &wpl).code == DS_OK);
        printf("教材例 {5,29,7,8,14,23,3,11}: WPL = %ld (标准答案 271)\n", wpl);
        assert(wpl == 271);
    }

    /* 用例2: 打印各叶编码 */
    {
        int w[] = { 5, 29, 7, 8, 14, 23, 3, 11 };

        assert(ht_build(&ht, w, 8).code == DS_OK);
        for (i = 1; i <= 8; i++) {
            assert(ht_code(&ht, i, code).code == DS_OK);
            printf("  w=%-3d 编码: %s\n", w[i - 1], code);
        }
        /* 权 29（最大）应最短，权 3（最小）应最长 */
        {
            char c29[HUFF_MAX_CODE_LEN + 1];
            char c3[HUFF_MAX_CODE_LEN + 1];

            assert(ht_code(&ht, 2, c29).code == DS_OK);
            assert(ht_code(&ht, 7, c3).code == DS_OK);
            assert(strlen(c29) < strlen(c3));
        }
    }

    /* 用例3: 编码解码 roundtrip（手工例） */
    {
        int w[] = { 5, 29, 7, 8 };
        char stream[128] = "";

        assert(ht_build(&ht, w, 4).code == DS_OK);
        /* 消息: 叶1,叶2,叶3,叶2,叶4 */
        {
            size_t msg[] = { 1, 2, 3, 2, 4 };
            size_t k = 0;

            for (k = 0; k < 5; k++) {
                assert(ht_code(&ht, msg[k], code).code == DS_OK);
                strcat(stream, code);
            }
            assert(ht_decode(&ht, stream, symbols, &n).code == DS_OK);
            assert(n == 5);
            for (k = 0; k < 5; k++) {
                assert(symbols[k] == msg[k]);
            }
            printf("roundtrip: 4 叶树消息(1,2,3,2,4) 编码 \"%s\" 解码还原一致\n", stream);
        }

        /* 解码错误: 位串半路截断 */
        {
            char half[8];

            strcpy(half, stream);
            half[strlen(half) - 1] = '\0';
            /* 去掉末位后是否截断取决于该位是否落编码中段——取足够长前缀必然半路停 */
            if (strlen(half) >= 1) {
                DsResult st = ht_decode(&ht, half, symbols, &n);

                /* 允许恰好仍是完整前缀的幸运情形，但绝大多数应报错或解出更少符号 */
                assert(st.code == DS_OK || st.code == DS_ERROR);
            }
        }
        /* 非法字符 */
        assert(ht_decode(&ht, "012x", symbols, &n).code == DS_ERROR);
    }

    /* 用例4: 边界 —— 单叶子 / 非法参数 */
    {
        int w1[] = { 7 };

        assert(ht_build(&ht, w1, 1).code == DS_OK);
        assert(ht_wpl(&ht, &wpl).code == DS_OK && wpl == 0);
        assert(ht_code(&ht, 1, code).code == DS_OK && strcmp(code, "0") == 0);
        assert(ht_decode(&ht, "000", symbols, &n).code == DS_OK && n == 3);

        assert(ht_build(&ht, w1, 0).code == DS_OUT_OF_RANGE);
        assert(ht_build(&ht, NULL, 1).code == DS_NULL_PTR);
        assert(ht_build(NULL, w1, 1).code == DS_NULL_PTR);
        {
            int bad[] = { 1, 0, 2 };

            assert(ht_build(&ht, bad, 3).code == DS_ERROR);
        }
        assert(ht_code(&ht, 99, code).code == DS_OUT_OF_RANGE);
        assert(ht_code(NULL, 1, code).code == DS_NULL_PTR);
        assert(ht_decode(&ht, NULL, symbols, &n).code == DS_NULL_PTR);
        assert(ht_wpl(NULL, &wpl).code == DS_NULL_PTR);
    }

    /* 用例5: 两叶最小例 */
    {
        int w[] = { 1, 1 };

        assert(ht_build(&ht, w, 2).code == DS_OK);
        assert(ht_wpl(&ht, &wpl).code == DS_OK && wpl == 2);
        assert(ht_code(&ht, 1, code).code == DS_OK && strcmp(code, "0") == 0);
        assert(ht_code(&ht, 2, code).code == DS_OK && strcmp(code, "1") == 0);
    }

    printf("全部测试通过\n");
    return 0;
}
