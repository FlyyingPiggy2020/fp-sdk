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
 * @FilePath     : log.h
 * @Author       : lxf
 * @Date         : 2023-12-08 13:58:24
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2023-12-08 14:09:25
 * @Brief        : 日志打印函数
 */
#ifndef _LOG_H_
#define _LOG_H_

/*---------- includes ----------*/

#include "stdarg.h"
#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"

/*---------- macro ----------*/

/**
 * @brief 定义日志的输出级别
 * @return {*}
 */
#define LOG_LVL_ASSERT  0
#define LOG_LVL_ERROR   1
#define LOG_LVL_WARN    2
#define LOG_LVL_INFO    3
#define LOG_LVL_DEBUG   4
#define LOG_LVL_VERBOSE 5

#if (USE_ESP == 1)
/**
 * @brief 定义日志断言函数(条件不成立就会输出断言信息)
 * @return {*}
 */
#define LOG_ASSERT(EXPR)                                    \
    if (!(EXPR)) {                                          \
        if (log_assert_hook == NULL) {                      \
            log_a("(%s) has assert failed.", #EXPR);        \
        } else {                                            \
            log_assert_hook(#EXPR, __FUNCTION__, __LINE__); \
        }                                                   \
    }
#else
/**
 * @brief 定义日志断言函数(条件不成立就会输出断言信息)
 * @return {*}
 */
#define LOG_ASSERT(EXPR)                                    \
    if (!(EXPR)) {                                          \
        if (log_assert_hook == NULL) {                      \
            log_a("(%s) has assert failed.", #EXPR);        \
            while (1)                                       \
                ;                                           \
        } else {                                            \
            log_assert_hook(#EXPR, __FUNCTION__, __LINE__); \
        }                                                   \
    }
#endif
/**
 * @brief 定义一个宏，从__FILE__中查找到文件名,把目录都删去了
 * @return {*}
 */
#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__))
#endif

/**
 * @brief 如果要使用log_x这种函数，必须事先定义LOG_TAG和LOG_OUTPUT_LVL
 * @return {*}
 */
#if !defined(LOG_TAG)
#define LOG_TAG "NO_TAG"
#endif
#if !defined(LOG_OUTPUT_LVL)
#define LOG_OUTPUT_LVL LOG_LVL_VERBOSE
#endif

#if LOG_OUTPUT_LVL >= LOG_LVL_ASSERT
#define log_a(...) log_output(LOG_LVL_ASSERT, LOG_TAG, __FILENAME__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define log_a(...) ((void)0);
#endif

#if LOG_OUTPUT_LVL >= LOG_LVL_ERROR
#define log_e(...) log_output(LOG_LVL_ERROR, LOG_TAG, __FILENAME__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define log_e(...) ((void)0);
#endif

#if LOG_OUTPUT_LVL >= LOG_LVL_WARN
#define log_w(...) log_output(LOG_LVL_WARN, LOG_TAG, __FILENAME__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define log_w(...) ((void)0);
#endif

#if LOG_OUTPUT_LVL >= LOG_LVL_INFO
#define log_i(...) log_output(LOG_LVL_INFO, LOG_TAG, __FILENAME__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define log_i(...) ((void)0);
#endif

#if LOG_OUTPUT_LVL >= LOG_LVL_DEBUG
#define log_d(...) log_output(LOG_LVL_DEBUG, LOG_TAG, __FILENAME__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define log_d(...) ((void)0);
#endif

#if LOG_OUTPUT_LVL >= LOG_LVL_VERBOSE
#define log_v(...) log_output(LOG_LVL_VERBOSE, LOG_TAG, __FILENAME__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define log_v(...) ((void)0);
#endif

#undef assert
#ifndef assert
#define assert LOG_ASSERT
#endif
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

int log_init(void);
void log_output(uint8_t level, const char *tag, const char *file, const char *func, const long line, const char *format, ...);
void set_log_port_output(void *output);
extern void (*log_assert_hook)(const char *expr, const char *func, size_t line);

/*---------- end of file ----------*/
#endif