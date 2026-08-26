/**
 * @file expression_eval.h
 * @brief 表达式求值（中缀转后缀 + 后缀求值）接口定义
 *
 * 设计要点:
 *   1. 两阶段求值 —— 先用算符优先法把中缀式转为逆波兰式（后缀式），
 *      消除括号与优先级信息，之后求值只需一线性扫描
 *   2. 模块内自包含 —— 转换与求值各用一套 static 数组栈，
 *      不依赖任何外部栈实现
 *   3. 后缀式用空格分隔 token，多位整数因此不会被误读为多个操作数
 * 复杂度: 两阶段均为 O(n) 单遍扫描
 */

#ifndef EXPRESSION_EVAL_H
#define EXPRESSION_EVAL_H

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

/* 表达式最大长度（含结尾 '\0'），也是内部 static 栈的容量上界 */
#define EXPR_MAX_LEN 256

DsResult infix_to_postfix(const char *infix, char *postfix);
DsResult eval_postfix(const char *postfix, int *result);

/**
 * @brief 将状态码转换为可读字符串（调试辅助）
 * @param s 状态码
 * @return 对应的字符串描述
 */
const char *ds_status_str(DsStatus s);
#endif /* EXPRESSION_EVAL_H */
