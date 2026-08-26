/**
 * @file main.c
 * @brief 大根堆 + 堆排序断言测试
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "heap_sort.h"

static int cmp_int(const void *x, const void *y)
{
    int a = *(const int *)x;
    int b = *(const int *)y;
    return (a > b) - (a < b);
}

static int is_asc(const int a[], size_t n)
{
    size_t i = 0;
    for (i = 1; i < n; ++i) {
        if (a[i - 1] > a[i]) {
            return 0;
        }
    }
    return 1;
}

static int is_max_heap(const DsHeap *h)
{
    size_t i = 0;
    if (h->size == 0) return 1;
    for (i = 1; i <= h->size / 2; ++i) {
        size_t l = 2 * i;
        size_t r = l + 1;
        if (h->buf[i] < h->buf[l]) return 0;
        if (r <= h->size && h->buf[i] < h->buf[r]) return 0;
    }
    return 1;
}

int main(void)
{
    enum { N = 500, CAP = 256 };
    int a[N];
    int expected[N];
    int heapbuf[CAP + 1];
    DsHeap heap;
    int out = 0;
    size_t i = 0;
    size_t cnt = 0;
    int rounds = 0;

    printf("===== 大根堆 + 堆排序 heap_sort =====\n");

    /* ---------- 堆 ADT ---------- */
    assert(heap_create(NULL, heapbuf, CAP).code == DS_NULL_PTR);
    assert(heap_create(&heap, NULL, CAP).code == DS_NULL_PTR);
    assert(heap_create(&heap, heapbuf, CAP).code == DS_OK);
    assert(heap_peek(&heap, &out).code == DS_EMPTY);
    assert(heap_delete_top(&heap, &out).code == DS_EMPTY);
    assert(heap_insert(NULL, 1).code == DS_NULL_PTR);

    /* 已知序列插堆 */
    {
        static const int seq[] = { 3, 5, 1, 9, 2, 8, 4, 7 };
        size_t k = 0;
        int curmax = 0;
        for (k = 0; k < sizeof(seq) / sizeof(seq[0]); ++k) {
            int top = 0;
            if (seq[k] > curmax) curmax = seq[k];
            assert(heap_insert(&heap, seq[k]).code == DS_OK);
            assert(is_max_heap(&heap));
            assert(heap_peek(&heap, &top).code == DS_OK);
            assert(top == curmax);
        }
        while (heap_delete_top(&heap, &out).code == DS_OK) {
            (void)out;
        }
    }

    /* 随机插堆 + 连续删顶 */
    srand(20260825u);
    for (rounds = 0; rounds < 500; ++rounds) {
        size_t n = (size_t)(rand() % CAP) + 1;
        heap_clear(&heap);
        for (i = 0; i < n; ++i) {
            int v = rand() % 1000;
            assert(heap_insert(&heap, v).code == DS_OK);
            assert(is_max_heap(&heap));
            expected[i] = v;
        }
        qsort(expected, n, sizeof *expected, cmp_int);
        cnt = n;
        while (heap_delete_top(&heap, &out).code == DS_OK && cnt > 0) {
            assert(out == expected[cnt - 1]);
            --cnt;
        }
        assert(cnt == 0);
    }

    /* 堆满 */
    heap_clear(&heap);
    for (i = 0; i < CAP; ++i) {
        assert(heap_insert(&heap, (int)i).code == DS_OK);
    }
    assert(heap_insert(&heap, 0).code == DS_OVERFLOW);

    /* ---------- heap_sort 随机对拍 ---------- */
    assert(heap_sort(NULL, 3).code == DS_NULL_PTR);
    assert(heap_sort(a, 0).code == DS_OK);
    a[0] = 6;
    assert(heap_sort(a, 1).code == DS_OK && a[0] == 6);
    for (rounds = 0; rounds < 2000; ++rounds) {
        size_t n = (size_t)(rand() % N) + 1;
        for (i = 0; i < n; ++i) {
            a[i] = rand() % 1000;
            expected[i] = a[i];
        }
        assert(heap_sort(a, n).code == DS_OK);
        qsort(expected, n, sizeof *expected, cmp_int);
        for (i = 0; i < n; ++i) {
            assert(a[i] == expected[i]);
        }
        assert(is_asc(a, n));
    }

    printf("全部测试通过\n");
    return 0;
}
