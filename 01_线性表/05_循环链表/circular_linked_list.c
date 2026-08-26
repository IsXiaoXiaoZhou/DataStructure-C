/**
 * @file circular_linked_list.c
 * @brief 循环链表（尾指针 rear 表示）接口实现
 *
 * 存储结构: rear → [an] ，且 rear->next → [a1] → [a2] → ... → [an] ↺
 * 设计要点:
 *   1. 环不变量 —— rear 恒指尾结点，rear->next 恒指首元；
 *      空表 rear == NULL。所有增删都在"rear 之后"这个统一位置
 *      附近进行，头插与尾插仅差是否更新 rear 一步
 *   2. 环无终点：遍历一律以计数 length 圈或断环后判 NULL 控制
 * 复杂度: 按位查找 O(n)；头插/尾插 O(1)；合并 O(1)
 */

#include <stdio.h>

#include <stdlib.h>

#include "circular_linked_list.h"


/* 环结点：与单链表结点同构，靠"尾结点回指首元"成环 */
typedef struct CListNode {
    int             data;
    struct CListNode *next;
} CListNode;

struct CircularLinkedList {
    CListNode *rear;   /* 不变量: 空表 NULL；非空指尾结点，rear->next 指首元 */
    size_t     length; /* 缓存长度，控制环遍历的圈数 */
};

/* 结点分配集中于此：失败统一映射为 DS_OVERFLOW */
static CListNode *node_new(int value)
{
    CListNode *node = malloc(sizeof *node);

    if (node != NULL) {
        node->data = value;
        node->next = NULL;
    }
    return node;
}

DsResult list_init(CircularLinkedList **list)
{
    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表二级指针为空"};
    }

    *list = malloc(sizeof **list);
    if (*list == NULL) {
        return (DsResult){DS_OVERFLOW, "链表结构体内存分配失败"};
    }

    (*list)->rear = NULL;
    (*list)->length = 0;

    return (DsResult){DS_OK, "循环链表创建成功"};
}

void list_destroy(CircularLinkedList **list)
{
    CListNode *curr = NULL;
    CListNode *next = NULL;

    if (list == NULL || *list == NULL) {
        return;
    }

    if ((*list)->rear != NULL) {
        /*
         * 环没有天然终点：先把尾结点断环（next 置 NULL），
         * 之后即可按普通单链表逐结点释放
         */
        curr = (*list)->rear->next;
        (*list)->rear->next = NULL;
        while (curr != NULL) {
            next = curr->next;
            free(curr);
            curr = next;
        }
    }

    free(*list);
    *list = NULL;
}

DsResult list_push_front(CircularLinkedList *list, int value)
{
    CListNode *node = NULL;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }

    node = node_new(value);
    if (node == NULL) {
        return (DsResult){DS_OVERFLOW, "结点内存分配失败"};
    }

    if (list->rear == NULL) {
        /* 空表：新结点自成环，自己既是首元也是尾元，rear 必须同步 */
        node->next = node;
        list->rear = node;
    } else {
        /* 头插不动 rear：新结点插在 rear（尾）与原首元之间 */
        node->next = list->rear->next;
        list->rear->next = node;
    }
    list->length++;

    return (DsResult){DS_OK, "头插成功"};
}

DsResult list_push_back(CircularLinkedList *list, int value)
{
    CListNode *node = NULL;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }

    node = node_new(value);
    if (node == NULL) {
        return (DsResult){DS_OVERFLOW, "结点内存分配失败"};
    }

    if (list->rear == NULL) {
        /* 空表：与头插同构，单结点成环 */
        node->next = node;
    } else {
        /*
         * 尾插 O(1) 的来历：新结点插在 rear 之后，
         * next 继承原首元指针，环的其余部分完全不动
         */
        node->next = list->rear->next;
        list->rear->next = node;
    }
    /* 与头插唯一区别：新结点成为尾元，rear 前移 */
    list->rear = node;
    list->length++;

    return (DsResult){DS_OK, "尾插成功"};
}

DsResult list_insert_at(CircularLinkedList *list, size_t pos, int value)
{
    CListNode *prev = NULL;
    CListNode *node = NULL;
    size_t     i = 0;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }
    if (pos < 1 || pos > list->length + 1) {
        return (DsResult){DS_OUT_OF_RANGE, "插入位置越界，合法范围 [1, length+1]"};
    }

    node = node_new(value);
    if (node == NULL) {
        return (DsResult){DS_OVERFLOW, "结点内存分配失败"};
    }

    if (list->rear == NULL) {
        /* 空表：新结点自成环（既是首元也是尾元），rear 必须同步 */
        node->next = node;
        list->rear = node;
    } else {
        /*
         * 前驱定位：首元的前驱是尾结点 rear（环的性质），故从
         * rear 出发走 pos-1 步即第 pos-1 个结点；尾后插入
         * （pos==length+1）的前驱恰是 rear 本身，无需走链
         */
        prev = list->rear;
        if (pos != list->length + 1) {
            for (i = 0; i < pos - 1; i++) {
                prev = prev->next;
            }
        }
        node->next = prev->next;
        prev->next = node;
        if (pos == list->length + 1) {
            list->rear = node;   /* 与头插唯一区别：新结点成为尾元 */
        }
    }
    list->length++;

    return (DsResult){DS_OK, "插入成功"};
}

DsResult list_remove_at(CircularLinkedList *list, size_t pos, int *value)
{
    CListNode *prev = NULL;
    CListNode *target = NULL;
    size_t     i = 0;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }
    if (list->length == 0) {
        return (DsResult){DS_EMPTY, "链表为空，无法删除"};
    }
    if (pos < 1 || pos > list->length) {
        return (DsResult){DS_OUT_OF_RANGE, "删除位置越界，合法范围 [1, length]"};
    }

    /*
     * 首元的前驱是尾结点 rear（环的性质），故定位一律从 rear
     * 出发走 pos-1 步即得第 pos 个结点的前驱 —— pos==1 时
     * 前驱恰为 rear，无需任何特判
     */
    prev = list->rear;
    for (i = 0; i < pos - 1; i++) {
        prev = prev->next;
    }
    target = prev->next;

    if (value != NULL) {
        *value = target->data;
    }

    if (target == list->rear) {
        if (list->length == 1) {
            /* 删唯一的结点：环消失，表回空 */
            list->rear = NULL;
        } else {
            list->rear = prev;   /* 删尾结点：前驱升任新尾 */
        }
    }

    /* 先绕链再释放，保持环上其余结点连接完好 */
    prev->next = target->next;
    free(target);
    list->length--;

    return (DsResult){DS_OK, "删除成功"};
}

DsResult list_get(const CircularLinkedList *list, size_t pos, int *value)
{
    CListNode *curr = NULL;
    size_t     i = 0;

    if (list == NULL || value == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }
    if (list->length == 0) {
        return (DsResult){DS_EMPTY, "链表为空，无法取元素"};
    }
    if (pos < 1 || pos > list->length) {
        return (DsResult){DS_OUT_OF_RANGE, "位置越界，合法范围 [1, length]"};
    }

    curr = list->rear->next;   /* 首元 */
    for (i = 1; i < pos; i++) {
        curr = curr->next;
    }

    *value = curr->data;
    return (DsResult){DS_OK, "取元素成功"};
}

DsResult list_find(const CircularLinkedList *list, int value, size_t *pos)
{
    CListNode *curr = NULL;
    size_t     i = 0;

    if (list == NULL || pos == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    /* 环无终点：从首元按计数走一圈即停，找到立即返回 */
    if (list->rear != NULL) {
        curr = list->rear->next;
        for (i = 0; i < list->length; i++) {
            if (curr->data == value) {
                *pos = i + 1;
                return (DsResult){DS_OK, "查找成功"};
            }
            curr = curr->next;
        }
    }

    /* 未命中（含空表）：置 0 返回 DS_NOT_FOUND，与顺序表语义一致 */
    *pos = 0;
    return (DsResult){DS_NOT_FOUND, "未找到目标元素"};
}

size_t list_length(const CircularLinkedList *list)
{
    /* 前置条件: list 非 NULL；防御性返回 0 而非崩溃 */
    return (list == NULL) ? 0 : list->length;
}

int list_is_empty(const CircularLinkedList *list)
{
    return (list == NULL) || (list->rear == NULL);
}

DsResult list_clear(CircularLinkedList *list)
{
    CListNode *curr = NULL;
    CListNode *next = NULL;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }

    if (list->rear != NULL) {
        /*
         * 与 destroy 同法：环没有天然终点，先把尾结点断环
         * （next 置 NULL），再按普通单链表逐结点释放；
         * 但止步于表壳，rear 归 NULL 即回到空表
         */
        curr = list->rear->next;
        list->rear->next = NULL;
        while (curr != NULL) {
            next = curr->next;
            free(curr);
            curr = next;
        }
        list->rear = NULL;
    }
    list->length = 0;

    return (DsResult){DS_OK, "链表清空成功"};
}

DsResult list_traverse(const CircularLinkedList *list, void (*visit)(int))
{
    CListNode *curr = NULL;
    size_t     i = 0;

    if (list == NULL || visit == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    if (list->rear != NULL) {
        /* 按计数走一圈立即停，绝不能等 curr 绕回起点判断——那是死循环 */
        curr = list->rear->next;
        for (i = 0; i < list->length; i++) {
            visit(curr->data);
            curr = curr->next;
        }
    }

    return (DsResult){DS_OK, "遍历完成"};
}

DsResult list_print(const CircularLinkedList *list)
{
    CListNode *curr = NULL;
    size_t     i = 0;

    if (list == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }

    printf("CircularLinkedList(length=%lu): rear ↺ ",
           (unsigned long)list->length);
    if (list->rear != NULL) {
        /* 按计数走一圈立即停，绝不能等 curr 绕回起点判断——那是死循环 */
        curr = list->rear->next;
        for (i = 0; i < list->length; i++) {
            printf("[%d] → ", curr->data);
            curr = curr->next;
        }
    }
    printf("(回到首元)\n");

    return (DsResult){DS_OK, "打印完成"};
}

DsResult list_merge(CircularLinkedList *list_a, CircularLinkedList *list_b)
{
    CListNode *a_first = NULL;

    if (list_a == NULL || list_b == NULL) {
        return (DsResult){DS_NULL_PTR, "链表指针为空"};
    }
    /*
     * 自合并防御：merge(a,a) 会使后续"b 置空"步骤把 a 自己清空
     * （rear=NULL、length=0），而环上结点已互相改接、无人再持有
     * 它们的入口指针，全部泄漏 —— 故同表合并直接拒绝
     */
    if (list_a == list_b) {
        return (DsResult){DS_ERROR, "不能自合并同一张表"};
    }

    /* 退化情形：任一表为空时无需接环，只处理归属关系 */
    if (list_b->rear == NULL) {
        return (DsResult){DS_OK, "被并表为空，无需操作"};
    }
    if (list_a->rear == NULL) {
        list_a->rear = list_b->rear;        /* a 空：直接接管 b 的环 */
        list_a->length = list_b->length;
        list_b->rear = NULL;
        list_b->length = 0;
        return (DsResult){DS_OK, "空表接管成功"};
    }

    /*
     * 两环相接，指针调整四步（a: a1..an，b: b1..bm）:
     *   第 1 步 保存 a 的首元 a1（改指针前必留快照）
     *   第 2 步 a 尾 an 的 next 改指 b 首元 b1 —— 两链首尾相接
     *   第 3 步 b 尾 bm 的 next 改指 a1 —— 重新闭合成一个大环
     *   第 4 步 rear 移到 bm（新尾）；b 置空（长度并入 a）
     */
    a_first = list_a->rear->next;                 /* 第 1 步 */
    list_a->rear->next = list_b->rear->next;      /* 第 2 步 */
    list_b->rear->next = a_first;                 /* 第 3 步 */
    list_a->rear = list_b->rear;                  /* 第 4 步 */
    list_a->length += list_b->length;
    list_b->rear = NULL;
    list_b->length = 0;

    return (DsResult){DS_OK, "两环合并成功"};
}

const char *ds_status_str(DsStatus s)
{
    switch (s) {
        case DS_OK:         return "操作成功";
        case DS_ERROR:      return "一般性错误";
        case DS_NULL_PTR:   return "空指针参数";
        case DS_OUT_OF_RANGE: return "位置/下标越界";
        case DS_OVERFLOW:   return "空间已满或内存分配失败";
        case DS_EMPTY:      return "对空结构取元素";
        case DS_NOT_FOUND:  return "查找未命中";
        default:            return "未知状态码";
    }
}
