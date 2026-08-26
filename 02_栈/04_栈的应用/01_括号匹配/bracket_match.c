/**
 * @file bracket_match.c
 * @brief 括号匹配检验接口实现
 *
 * 实现说明:
 *   1. 模块内自包含一套 static 数组栈（不依赖外部栈模块）：
 *      栈元素同时记录左括号字符与其在表达式中的下标，
 *      出错时才能报告准确位置
 *   2. 配对判定只看"弹出的左括号与当前右括号是否同类"，
 *      与括号里夹了什么字符无关，故非括号字符一律跳过
 */

#include "bracket_match.h"

/* 栈元素：同时记录左括号字符与其在表达式中的下标 */
typedef struct {
    char   ch;
    size_t pos;
} BracketItem;

/* 内部静态栈 */
static BracketItem bracket_stack[BRACKET_MAX_DEPTH];
static int         bracket_top;

/* 判断左右括号是否同类 */
static int is_pair(char left, char right)
{
    return (left == '(' && right == ')') ||
           (left == '[' && right == ']') ||
           (left == '{' && right == '}');
}

DsResult bracket_match(const char *expr, BracketError *err)
{
    size_t i = 0;

    if (expr == NULL || err == NULL) {
        return (DsResult){DS_NULL_PTR, "参数指针为空"};
    }

    bracket_top = -1;
    for (i = 0; expr[i] != '\0'; i++) {
        if (expr[i] == '(' || expr[i] == '[' || expr[i] == '{') {
            /* 左括号：字符与下标一并入栈，等待后继的同类右括号 */
            if (bracket_top == BRACKET_MAX_DEPTH - 1) {
                return (DsResult){DS_OVERFLOW, "括号嵌套过深，超出栈容量"};
            }
            bracket_stack[++bracket_top].ch = expr[i];
            bracket_stack[bracket_top].pos = i;
        } else if (expr[i] == ')' || expr[i] == ']' || expr[i] == '}') {
            /* 右括号：必须弹出一个左括号与它配对 */
            if (bracket_top == -1) {
                /*
                 * 栈已空：这个右括号前面没有左括号在等它，
                 * 报它自己的下标
                 */
                err->type = BRACKET_MISSING_LEFT;
                err->pos = i;
                return (DsResult){DS_OK, "检查完成，缺左括号"};
            }
            if (!is_pair(bracket_stack[bracket_top].ch, expr[i])) {
                /*
                 * 栈顶左括号与当前右括号不同类，如 "(...]"：
                 * 报引发冲突的右括号下标
                 */
                err->type = BRACKET_MISMATCH;
                err->pos = i;
                return (DsResult){DS_OK, "检查完成，括号类型不匹配"};
            }
            bracket_top--; /* 同类配对成功，弹出已配对的左括号 */
        }
        /* 其他字符（运算数、运算符、空白等）与匹配无关，跳过 */
    }

    if (bracket_top != -1) {
        /*
         * 扫描结束栈非空：这些左括号始终没等到右括号。
         * 报栈底元素 —— 即最早出现且未配对的左括号，
         * 符合"从头找第一个错"的阅读直觉
         */
        err->type = BRACKET_MISSING_RIGHT;
        err->pos = bracket_stack[0].pos;
        return (DsResult){DS_OK, "检查完成，缺右括号"};
    }

    err->type = BRACKET_OK;
    err->pos = 0;

    return (DsResult){DS_OK, "检查完成，括号完全匹配"};
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
