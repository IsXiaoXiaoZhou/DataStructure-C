/**
 * @file bracket_match.h
 * @brief 括号匹配检验（栈的典型应用）接口定义
 *
 * 算法思想（左括号入栈、右括号弹栈配对）:
 *   1. 扫描到左括号 ( [ { —— 字符与下标一并入栈，等待配对
 *   2. 扫描到右括号 ) ] } —— 弹栈配对：
 *      栈空说明它前面没有左括号（缺左括号）；
 *      栈顶左括号与之不同类说明类型不匹配（如 "(...]"）
 *   3. 扫描结束栈仍非空 —— 有左括号始终没等到右括号（缺右括号）
 *
 * 设计要点:
 *   1. 模块内自包含 —— 内部用 static 数组栈实现，不依赖任何
 *      外部栈模块；栈元素同时记录左括号字符与其下标，
 *      报错时才能精确定位出错位置
 *   2. 检查结论（含"完全匹配"）一律经 BracketError 带回，
 *      返回码只表达"检查是否正常完成"，两者职责分离
 * 复杂度: 时间 O(n)，辅助空间 O(n)（n 为表达式长度）
 */

#ifndef BRACKET_MATCH_H
#define BRACKET_MATCH_H

#include <stddef.h>

/* 状态码（统一六值版，未用到的成员也保留） */
typedef enum {
    DS_OK = 0,          /* 操作成功 */
    DS_ERROR,           /* 一般性错误（如查找未命中、非法参数组合） */
    DS_NULL_PTR,        /* 空指针参数 */
    DS_OUT_OF_RANGE,    /* 位置/下标越界 */
    DS_OVERFLOW,        /* 空间已满或内存分配失败（含计算结果溢出） */
    DS_EMPTY,           // 对空结构取元素
    DS_NOT_FOUND        // 查找未命中（业务正常结果，非异常）
} DsStatus;

/* 返回结果：状态码 + 可读描述 */
typedef struct {
    DsStatus    code;       /* 枚举码（程序判断用） */
    const char *message;    /* 可读消息（日志/调试用） */
} DsResult;

/* 内部数组栈容量：括号嵌套深度上界，超过视为表达式异常 */
#define BRACKET_MAX_DEPTH 256

/* 括号检查结论类型 */
typedef enum {
    BRACKET_OK = 0,        /* 三类括号完全匹配 */
    BRACKET_MISSING_LEFT,  /* 缺左括号：右括号到来时栈已空（多右括号） */
    BRACKET_MISSING_RIGHT, /* 缺右括号：扫描结束栈中仍有左括号（多左括号） */
    BRACKET_MISMATCH       /* 左右括号类型不匹配，如 ( 与 ] 相遇 */
} BracketStatus;

/* 括号检查结论详情 */
typedef struct {
    BracketStatus type; /* 结论类型：四态之一 */
    size_t pos;         /* 出错字符下标（从 0 开始），匹配成功时为 0 */
} BracketError;

DsResult bracket_match(const char *expr, BracketError *err);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* BRACKET_MATCH_H */
