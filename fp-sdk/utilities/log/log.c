/*MIT License

Copyright (c) 2023 Lu Xianfan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
/*
 * Copyright (c) 2023 by Moorgen Tech. Co, Ltd.
 * @FilePath     : log.c
 * @Author       : lxf
 * @Date         : 2023-12-08 13:58:19
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2023-12-08 14:06:50
 * @Brief        :
 */

/*---------- includes ----------*/

#include "log.h"

/*---------- macro ----------*/

/**
 * @brief 定义CSI颜色，更多的信息请查看https://en.wikipedia.org/wiki/ANSI_escape_code
 * @return {*}
 */
#define CSI_START "\033["
#define CSI_END   "\033[0m"
#define CSI_START "\033["
#define CSI_END   "\033[0m"

/**
 * @brief 字符颜色
 * @return {*}
 */
#define F_BLACK   "30;"
#define F_RED     "31;"
#define F_GREEN   "32;"
#define F_YELLOW  "33;"
#define F_BLUE    "34;"
#define F_MAGENTA "35;"
#define F_CYAN    "36;"
#define F_WHITE   "37;"

/**
 * @brief 背景颜色
 * @return {*}
 */
#define B_NULL
#define B_BLACK     "40;"
#define B_RED       "41;"
#define B_GREEN     "42;"
#define B_YELLOW    "43;"
#define B_BLUE      "44;"
#define B_MAGENTA   "45;"
#define B_CYAN      "46;"
#define B_WHITE     "47;"

/**
 * @brief 字符格式：加粗、下划线、闪烁、正常
 * @return {*}
 */
#define Sz_BOLD     "1m"
#define S_UNDERLINE "4m"
#define S_BLINK     "5m"
#define S_NORMAL    "22m"

/**
 * @brief 默认各个等级的输出颜色和格式
 * @return {*}
 */
#ifndef LOG_COLOR_ASSERT
#define LOG_COLOR_ASSERT (F_MAGENTA B_NULL S_NORMAL)
#endif
#ifndef LOG_COLOR_ERROR
#define LOG_COLOR_ERROR (F_RED B_NULL S_NORMAL)
#endif
#ifndef LOG_COLOR_WARN
#define LOG_COLOR_WARN (F_YELLOW B_NULL S_NORMAL)
#endif
#ifndef LOG_COLOR_INFO
#define LOG_COLOR_INFO (F_CYAN B_NULL S_NORMAL)
#endif
#ifndef LOG_COLOR_DEBUG
#define LOG_COLOR_DEBUG (F_GREEN B_NULL S_NORMAL)
#endif
#ifndef LOG_COLOR_VERBOSE
#define LOG_COLOR_VERBOSE (F_BLUE B_NULL S_NORMAL)
#endif

/*---------- type define ----------*/
/*---------- variable prototype ----------*/

/**
 * @brief 初始化完成的标志
 * @return {*}
 */
static bool is_init_ok = false;
/*---------- function prototype ----------*/

extern void log_port_output(const char *log, size_t size);
extern bool log_port_init(void);
/**
 * @brief 断言函数
 * @param {char} *expr ：条件
 * @param {char} *func ：函数
 * @param {size_t} line ：行号
 * @return {*}
 */
void (*log_assert_hook)(const char *expr, const char *func, size_t line);
/*---------- variable ----------*/

/**
 * @brief 用于日志输出的buffer
 * @return {*}
 */
static char log_buf[LOG_LINE_BUF_SIZE] = {0};

// clang-format off
static const char *color_output_info[] = {
        [LOG_LVL_ASSERT]  = LOG_COLOR_ASSERT,
        [LOG_LVL_ERROR]   = LOG_COLOR_ERROR,
        [LOG_LVL_WARN]    = LOG_COLOR_WARN,
        [LOG_LVL_INFO]    = LOG_COLOR_INFO,
        [LOG_LVL_DEBUG]   = LOG_COLOR_DEBUG,
        [LOG_LVL_VERBOSE] = LOG_COLOR_VERBOSE,
};

static const char *output_name[] = {
        [LOG_LVL_ASSERT]  = "ASSERT",
        [LOG_LVL_ERROR]   = "ERROR",
        [LOG_LVL_WARN]    = "WARN",
        [LOG_LVL_INFO]    = "INFO",
        [LOG_LVL_DEBUG]   = "DEBUG",
        [LOG_LVL_VERBOSE] = "VERBOSE",
};
// clang-format on
/*---------- function ----------*/

/**
 * @brief 初始化
 * @return {*}
 */
void log_init(void)
{
    is_init_ok = log_port_init();
}

/**
 * @brief 字符串复制
 * @param {size_t} cur_len
 * @param {char} *dst
 * @param {char} *src
 * @return {*}返回粘贴的长度
 */
size_t log_strcpy(size_t cur_len, char *dst, const char *src)
{
    const char *src_old = src;

    assert(dst);
    assert(src);

    while (*src != 0) {
        /* make sure destination has enough space */
        if (cur_len++ < LOG_LINE_BUF_SIZE) {
            *dst++ = *src++;
        }
        else {
            break;
        }
    }
    return src - src_old;
}

/**
 * @brief 日志输出
 * @param {uint8_t} level ：日志输出级别
 * @param {char} *tag ： 日志标志
 * @param {char} *file ：文件名称
 * @param {char} *func ： 函数名称
 * @param {long} line ： 行号
 * @param {char} *format ：格式化不定参
 * @return {*}
 */
void log_output(uint8_t level, const char *tag, const char *file, const char *func, const long line, const char *format, ...)
{
    size_t log_len = 0, fmt_result = 0, newline_len = strlen(LOG_NEWLINE_SIGN);
    char line_num[LOG_LINE_NUM_MAX_LEN + 1] = {0};

    va_list args;

    LOG_ASSERT(level <= LOG_LVL_VERBOSE);

    /* 增加颜色 */
    log_len += log_strcpy(log_len, log_buf + log_len, CSI_START);
    log_len += log_strcpy(log_len, log_buf + log_len, color_output_info[level]);
    /* 增加level */
    log_len += log_strcpy(log_len, log_buf + log_len, "[");
    log_len += log_strcpy(log_len, log_buf + log_len, output_name[level]);
    log_len += log_strcpy(log_len, log_buf + log_len, "]");
    /* 增加tag */
    log_len += log_strcpy(log_len, log_buf + log_len, "[");
    log_len += log_strcpy(log_len, log_buf + log_len, tag);
    log_len += log_strcpy(log_len, log_buf + log_len, "]");

    /* 增加function */
    log_len += log_strcpy(log_len, log_buf + log_len, "[");
    log_len += log_strcpy(log_len, log_buf + log_len, func);
    log_len += log_strcpy(log_len, log_buf + log_len, "(); ");
    /* 增加file */

    log_len += log_strcpy(log_len, log_buf + log_len, file);
    log_len += log_strcpy(log_len, log_buf + log_len, ":");
    /* 增加line */
    snprintf(line_num, LOG_LINE_NUM_MAX_LEN, "%ld", line);
    log_len += log_strcpy(log_len, log_buf + log_len, line_num);
    log_len += log_strcpy(log_len, log_buf + log_len, ":");

    log_len += log_strcpy(log_len, log_buf + log_len, "]");
    log_len += log_strcpy(log_len, log_buf + log_len, " - ");

    va_start(args, format);
    /* 用vsnprintf构建输出的buff */
    fmt_result = vsnprintf(log_buf + log_len, LOG_LINE_BUF_SIZE - log_len, format, args);
    va_end(args);

    /* 校验长度是否合法 */
    if ((log_len + fmt_result <= LOG_LINE_BUF_SIZE) && (fmt_result > -1)) {
        log_len += fmt_result;
    }
    else {
        log_len = LOG_LINE_BUF_SIZE;
    }

    /* 加上CSI_END标志和换行符之后长度是否合法 */
    if (log_len + (sizeof(CSI_END) - 1) + newline_len > LOG_LINE_BUF_SIZE) {
        /* using max length */
        log_len = LOG_LINE_BUF_SIZE;
        /* reserve some space for CSI end sign */
        log_len -= (sizeof(CSI_END) - 1);
        /* reserve some space for newline sign */
        log_len -= newline_len;
    }
    log_len += log_strcpy(log_len, log_buf + log_len, CSI_END);
    log_len += log_strcpy(log_len, log_buf + log_len, LOG_NEWLINE_SIGN);

    if (is_init_ok == true) {
        log_port_output(log_buf, log_len);
    }
}
/*---------- end of file ----------*/
