#ifndef UNION_FIND_H
#define UNION_FIND_H

/**
 * @file union_find.h
 * @brief 并查集（不相交集合）接口定义
 *
 * 存储结构: 双亲表示法数组 —— parent[x] 存双亲，根的 parent 指向自身，
 *           另配 size[] 支持按大小合并
 * 设计要点:
 *   1. find 带路径压缩（迭代两趟实现，防深链递归爆栈）:
 *      第一趟找根，第二趟沿途结点全部直挂根 —— 树被压成"扇形"
 *   2. union 按大小合并: 小树挂大树，保证树高 O(log n)
 *   3. 两种优化合用后单操作均摊 O(α(n))，α 为反阿克曼函数
 *      （工程上视作常数）
 * 应用: 等价类/连通性判定/Kruskal 最小生成树判环（见 07_图 模块）
 */

#include <stddef.h>

/* 状态码（按本模块所需裁剪，语义见各枚举项注释） */
typedef enum {
    DS_OK = 0,       /* 操作成功 */
    DS_ERROR,        /* 一般性错误（本集合与彼集合本就相同，未发生合并） */
    DS_NULL_PTR,     /* 空指针参数 */
    DS_OUT_OF_RANGE  /* 元素下标越界 */
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 并查集 */
typedef struct {
    size_t *parent; /* 双亲数组，根的 parent 指向自身 */
    size_t *size;   /* 仅根有效: 以该根为根的树结点数 */
    size_t n;       /* 元素个数（下标 0..n-1） */
    size_t sets;    /* 当前集合个数 */
} UnionFind;

DsResult uf_init(UnionFind *uf, size_t n);
DsResult uf_destroy(UnionFind *uf);
DsResult uf_find(UnionFind *uf, size_t x, size_t *root);
DsResult uf_union(UnionFind *uf, size_t x, size_t y, int *merged);
DsResult uf_connected(UnionFind *uf, size_t x, size_t y);
size_t uf_set_count(const UnionFind *uf);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* UNION_FIND_H */
