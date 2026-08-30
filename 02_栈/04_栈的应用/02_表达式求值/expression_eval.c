/**
 * @file expression_eval.c
 * @brief 表达式求值实现：算符优先法转后缀 + 栈式后缀求值
 *
 * 内部自包含两套 static 数组栈:
 *   - 转换阶段: char 型算符栈 op_stack
 *   - 求值阶段: int  型操作数栈 num_stack
 * 复杂度: 两阶段均 O(n)
 */

#include "expression_eval.h"

#include <limits.h>
#include <stddef.h>


/* 算符栈与操作数栈均按最坏情况（全长表达式）预留容量 */
static char op_stack[EXPR_MAX_LEN];
static int  op_top;                 /* 算符栈顶下标，-1 表示空 */

static int  num_stack[EXPR_MAX_LEN];
static int  num_top;                /* 操作数栈顶下标，-1 表示空 */

/* 后缀输出可写字符数：缓冲区 EXPR_MAX_LEN 含结尾 '\0'，须预留一格 */
#define POSTFIX_CAP (EXPR_MAX_LEN - 1)

/**
 * @brief 取运算符栈顶优先级，'(' 最低保证不被弹出
 */
static int prec_of(char op)
{
    switch (op) {
    case '+':
    case '-':
        return 1;
    case '*':
    case '/':
        return 2;
    case '(':
        return 0;    /* '(' 只在遇到 ')' 时弹出，比较优先级时视为最低 */
    default:
        return -1;
    }
}

DsResult infix_to_postfix(const char *infix, char *postfix)
{
    size_t out_pos = 0;
    size_t i;

    if (infix == NULL || postfix == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    op_top = -1;
    for (i = 0; infix[i] != '\0'; i++) {
        char ch = infix[i];

        if (ch == ' ') {
            continue;                   /* 源式中的空格仅作分隔，忽略 */
        }

        if (ch >= '0' && ch <= '9') {
            /* 多位操作数整体搬运，避免 "10" 被拆成 1 和 0 */
            while (infix[i] >= '0' && infix[i] <= '9') {
                if (out_pos >= POSTFIX_CAP) {
                    return (DsResult){DS_OVERFLOW, "后缀输出缓冲区已满"};
                }
                postfix[out_pos++] = infix[i];
                i++;
            }
            i--;                        /* 外层 for 会再自增一次，退回 */
            if (out_pos >= POSTFIX_CAP) {
                return (DsResult){DS_OVERFLOW, "后缀输出缓冲区已满"};
            }
            postfix[out_pos++] = ' ';   /* token 间以空格分隔 */
            continue;
        }

        if (ch == '(') {
            if (op_top >= EXPR_MAX_LEN - 1) {
                return (DsResult){DS_OVERFLOW, "算符栈已满"};
            }
            op_stack[++op_top] = ch;    /* 左括号无条件入栈 */
            continue;
        }

        if (ch == ')') {
            /* 弹到 '(' 为止；栈先空说明右括号多余 */
            while (op_top >= 0 && op_stack[op_top] != '(') {
                if (out_pos >= POSTFIX_CAP) {
                    return (DsResult){DS_OVERFLOW, "后缀输出缓冲区已满"};
                }
                postfix[out_pos++] = op_stack[op_top--];
                if (out_pos >= POSTFIX_CAP) {
                    return (DsResult){DS_OVERFLOW, "后缀输出缓冲区已满"};
                }
                postfix[out_pos++] = ' ';
            }
            if (op_top < 0) {
                return (DsResult){DS_ERROR, "括号不匹配，缺少左括号"};
            }
            op_top--;                   /* 丢弃 '('，它不进入后缀式 */
            continue;
        }

        if (ch == '+' || ch == '-' || ch == '*' || ch == '/') {
            /*
             * 左结合规则：栈顶优先级 >= 当前运算符时必须先弹后压，
             * 否则少弹一个，"5-3+2" 会被转成 5 3 2 + -，
             * 算出 5-(3+2)=0，而正确答案是 4
             */
            while (op_top >= 0 && prec_of(op_stack[op_top]) >= prec_of(ch)) {
                if (out_pos >= POSTFIX_CAP) {
                    return (DsResult){DS_OVERFLOW, "后缀输出缓冲区已满"};
                }
                postfix[out_pos++] = op_stack[op_top--];
                if (out_pos >= POSTFIX_CAP) {
                    return (DsResult){DS_OVERFLOW, "后缀输出缓冲区已满"};
                }
                postfix[out_pos++] = ' ';
            }
            if (op_top >= EXPR_MAX_LEN - 1) {
                return (DsResult){DS_OVERFLOW, "算符栈已满"};
            }
            op_stack[++op_top] = ch;
            continue;
        }

        return (DsResult){DS_ERROR, "表达式含非法字符"};                /* 非法字符 */
    }

    /* 收尾弹栈：剩余算符按逆序全部输出 */
    while (op_top >= 0) {
        if (op_stack[op_top] == '(') {
            return (DsResult){DS_ERROR, "括号不匹配，缺少右括号"};            /* 左括号没有配对的右括号 */
        }
        if (out_pos >= POSTFIX_CAP) {
            return (DsResult){DS_OVERFLOW, "后缀输出缓冲区已满"};
        }
        postfix[out_pos++] = op_stack[op_top--];
        if (out_pos >= POSTFIX_CAP) {
            return (DsResult){DS_OVERFLOW, "后缀输出缓冲区已满"};
        }
        postfix[out_pos++] = ' ';
    }

    if (out_pos == 0) {
        return (DsResult){DS_ERROR, "空表达式无值可求"};                /* 空表达式无值可求 */
    }
    postfix[out_pos - 1] = '\0';        /* 去掉末尾多余空格 */
    return (DsResult){DS_OK, "中缀转后缀成功"};
}

DsResult eval_postfix(const char *postfix, int *result)
{
    size_t i = 0;

    if (postfix == NULL || result == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    num_top = -1;
    while (postfix[i] != '\0') {
        if (postfix[i] == ' ') {
            i++;
            continue;
        }

        if (postfix[i] >= '0' && postfix[i] <= '9') {
            int value = 0;
            while (postfix[i] >= '0' && postfix[i] <= '9') {
                int digit = postfix[i] - '0';
                /*
                 * 溢出检测原理: 本次累乘要不越界，须满足
                 *   value*10 + digit <= INT_MAX
                 * 移项取整除（向下取整恰好把余数让给不等式右端）：
                 *   value <= (INT_MAX - digit) / 10
                 * 取反面即得越界判据 value > (INT_MAX-digit)/10。
                 * 先判后算，把溢出拦截在发生之前，避开有符号
                 * 整数溢出这一未定义行为
                 */
                if (value > (INT_MAX - digit) / 10) {
                    num_top = -1;       /* 复位栈状态，避免污染下次调用 */
                    return (DsResult){DS_OVERFLOW, "数值超出int表示范围"};
                }
                value = value * 10 + digit;
                i++;
            }
            if (num_top >= EXPR_MAX_LEN - 1) {
                num_top = -1;
                return (DsResult){DS_OVERFLOW, "操作数栈已满"};
            }
            num_stack[++num_top] = value;
            continue;
        }

        /*
         * 遇运算符：需两个操作数可用。注意先弹出的是右操作数 rhs，
         * 后弹出的是左操作数 lhs —— 顺序颠倒会把 8/4 算成 4/8
         */
        {
            int rhs;
            int lhs;
            if (num_top < 1) {
                return (DsResult){DS_ERROR, "操作数不足，表达式残缺"};
            }
            rhs = num_stack[num_top--];
            lhs = num_stack[num_top--];

            /* 弹二压一后 num_top 必小于原值，写回处不会越界 */
            switch (postfix[i]) {
            case '+':
                num_stack[++num_top] = lhs + rhs;
                break;
            case '-':
                num_stack[++num_top] = lhs - rhs;
                break;
            case '*':
                num_stack[++num_top] = lhs * rhs;
                break;
            case '/':
                if (rhs == 0) {
                    num_top = -1;       /* 复位栈状态，避免污染下次调用 */
                    return (DsResult){DS_ERROR, "除数为零"};
                }
                num_stack[++num_top] = lhs / rhs;
                break;
            default:
                num_top = -1;
                return (DsResult){DS_ERROR, "后缀式含未知运算符"};
            }
            i++;
        }
    }

    if (num_top != 0) {
        return (DsResult){DS_ERROR, "表达式残缺，结果不唯一"};
    }
    *result = num_stack[num_top];
    return (DsResult){DS_OK, "后缀求值成功"};
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
