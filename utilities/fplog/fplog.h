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
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : log.h
 * @Author       : lxf
 * @Date         : 2024-07-03 17:20:58
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-07-03 17:21:15
 * @Brief        : This file contains the implementation of the log functionality.
 */

#ifndef __FPLOG_H__
#define __FPLOG_H__
/*---------- includes ----------*/
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
/*---------- macro ----------*/
#define LOG_LVL_ASSERT  0
#define LOG_LVL_ERROR   1
#define LOG_LVL_WARN    2
#define LOG_LVL_INFO    3
#define LOG_LVL_DEBUG   4
#define LOG_LVL_VERBOSE 5

#ifndef LOG_TAG_BUFF_SIZE
#define LOG_TAG_BUFF_SIZE 66
#endif

/**
 * @brief The size of the line buffer used for logging.
 *
 * This macro defines the size of the line buffer used for logging in the application.
 * The line buffer is used to store log messages before they are written to the output device.
 * The value of this macro determines the maximum length of a log message that can be stored in the buffer.
 *
 * @note If this macro is not defined, a default value will be used.
 */
#ifndef LOG_CONTENT_SZIE
#define LOG_CONTENT_SZIE 256
#endif

#ifndef LOG_LINE_BUF_SIZE
#define LOG_LINE_BUF_SIZE (LOG_TAG_BUFF_SIZE + LOG_CONTENT_SZIE)
#endif

/**
 * @brief defines the macro LOG_NEWLINE_SIGN.
 *        LOG_NEWLINE_SIGN is used to specify the newline sign used in log messages.
 *        If LOG_NEWLINE_SIGN is not defined, a default newline sign will be used.
 */
#ifndef LOG_NEWLINE_SIGN
#define LOG_NEWLINE_SIGN "\r\n"
#endif

/**
 * @brief Defines the log output level.
 *
 * This file defines the logging output level and provides default values if not defined.
 */
#ifndef LOG_OUTPUT_LVL
#define LOG_OUTPUT_LVL LOG_LVL_INFO
#endif

#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__))
#endif

#ifndef LOG_TAG
#define LOG_TAG "NO_TAG"
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
#define log_d(...)               log_output(LOG_LVL_DEBUG, LOG_TAG, __FILENAME__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define log_hex(name, buf, size) log_hex_dump(name, 32, buf, size)
#else
#define log_d(...)   ((void)0);
#define log_hex(...) ((void)0);
#endif

#if LOG_OUTPUT_LVL >= LOG_LVL_VERBOSE
#define log_v(...) log_output(LOG_LVL_VERBOSE, LOG_TAG, __FILENAME__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define log_v(...) ((void)0);
#endif

#define LOG_ASSERT(EXPR)         \
    if (!(EXPR)) {               \
        log_a("assert failed."); \
    }

#undef assert
#define assert LOG_ASSERT
/*---------- type define ----------*/

typedef struct log_info_t {
    void (*log_output_handler)(const char *log, int len);
    void (*log_lock_handler)(void);
    void (*log_unlock_handler)(void);
    char log_buf[LOG_LINE_BUF_SIZE];
} log_info_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

unsigned char log_init(log_info_t *log_info);
void log_output(unsigned char level, const char *tag, const char *file, const char *func, const long line, const char *format, ...);
void log_hex_dump(const char *name, uint8_t width, const void *buf, uint16_t size);
/*---------- end of file ----------*/
#endif
