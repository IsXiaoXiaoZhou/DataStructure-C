#ifndef OPTIMAL_MERGE_TREE_H
#define OPTIMAL_MERGE_TREE_H

/**
 * @file optimal_merge_tree.h
 * @brief k 叉最佳归并树（按哈夫曼思想）接口定义
 *
 * k 路归并的归并树为 k 叉树——每个内部结点有恰 k 个孩子，叶子为各归并段。
 * 段长越短越早参与归并（越深），长段越靠近根，使总读写 I/O 次数最小。
 * 补虚段: k 叉树须满足 (n-1)%(k-1)==0。当不满足时用长度为 0 的虚叶子补齐。
 */

#include <stddef.h>

/* 六值状态码（与全工程逐字一致） */
typedef enum {
    DS_OK = 0,          /* 操作成功 */
    DS_ERROR,           /* 一般性错误 */
    DS_NULL_PTR,        /* 空指针参数 */
    DS_OUT_OF_RANGE,    /* 位置/下标越界 */
    DS_OVERFLOW,        /* 空间已满或内存分配失败 */
    DS_EMPTY,           // 对空结构取元素
    DS_NOT_FOUND        // 查找未命中（业务正常结果，非异常）
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

DsResult opt_merge_dummy_needed(size_t m, unsigned k, size_t *count);
DsResult opt_merge_wpl(const int len[], size_t m, unsigned k, long long *wpl);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* OPTIMAL_MERGE_TREE_H */
