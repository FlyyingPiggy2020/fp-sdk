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
 * @FilePath     : log.c
 * @Author       : lxf
 * @Date         : 2024-07-03 17:20:52
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-07-03 17:21:06
 * @Brief        : This file contains the implementation of the log functionality.
 */

/*---------- includes ----------*/

#include "fplog.h"

/*---------- macro ----------*/

#define CSI_START "\033["
#define CSI_END   "\033[0m"
#define CSI_START "\033["
#define CSI_END   "\033[0m"

#define F_BLACK   "30;"
#define F_RED     "31;"
#define F_GREEN   "32;"
#define F_YELLOW  "33;"
#define F_BLUE    "34;"
#define F_MAGENTA "35;"
#define F_CYAN    "36;"
#define F_WHITE   "37;"

#define B_NULL
#define B_BLACK     "40;"
#define B_RED       "41;"
#define B_GREEN     "42;"
#define B_YELLOW    "43;"
#define B_BLUE      "44;"
#define B_MAGENTA   "45;"
#define B_CYAN      "46;"
#define B_WHITE     "47;"

#define Sz_BOLD     "1m"
#define S_UNDERLINE "4m"
#define S_BLINK     "5m"
#define S_NORMAL    "22m"

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
static void _log_output_handler(const char *log, int len);
static void _log_lock_handler(void);
static void _log_unlock_handler(void);
static log_info_t s_log_info = {
    .log_output_handler = _log_output_handler,
    .log_lock_handler = _log_lock_handler,
    .log_unlock_handler = _log_unlock_handler,
};

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
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
static void _log_output_handler(const char *log, int len)
{
    return;
}
static void _log_lock_handler(void)
{
    return;
}
static void _log_unlock_handler(void)
{
    return;
}
/**
 * Initializes the log module with the provided log information.
 *
 * @param log_info Pointer to the log_info_t structure containing the log information.
 * @return Returns 1 if the initialization is successful, 0 otherwise.
 */
unsigned char log_init(log_info_t *log_info)
{
    if (log_info == NULL || log_info->log_output_handler == NULL) {
        return 0;
    }

    s_log_info.log_output_handler = log_info->log_output_handler;

    if (log_info->log_lock_handler != NULL && log_info->log_unlock_handler != NULL) {
        s_log_info.log_lock_handler = log_info->log_lock_handler;
        s_log_info.log_unlock_handler = log_info->log_unlock_handler;
    }
    return 1;
}

/**
 * Copies a null-terminated string from source to destination.
 *
 * @param cur_len The current length of the destination string.
 * @param dst The destination string where the copied string will be stored.
 * @param src The source string to be copied.
 * @return The number of characters copied from the source string.
 */
unsigned int log_strcpy(unsigned int cur_len, char *dst, const char *src)
{
    const char *src_old = src;

    while (*src != 0) {
        /* make sure destination has enough space */
        if (cur_len++ < LOG_LINE_BUF_SIZE) {
            *dst++ = *src++;
        } else {
            break;
        }
    }
    return src - src_old;
}

/**
 * @brief Outputs a log message with the specified level, tag, file, function, line number, and format.
 *
 * This function constructs a log message with the provided information and outputs it using the registered log output handler.
 * The log message includes the log level, tag, file, function, line number, and the formatted message.
 *
 * @param level The log level of the message.
 * @param tag The tag associated with the log message.
 * @param file The name of the source file where the log message is generated.
 * @param func The name of the function where the log message is generated.
 * @param line The line number in the source file where the log message is generated.
 * @param format The format string for the log message.
 * @param ... Additional arguments to be formatted according to the format string.
 */
void log_output(unsigned char level, const char *tag, const char *file, const char *func, const long line, const char *format, ...)
{
    s_log_info.log_lock_handler();
    int log_len = 0, fmt_result = 0;
    char *log_buf = s_log_info.log_buf;
    int newline_len = strlen(LOG_NEWLINE_SIGN);

    va_list args;

    /* 增加颜色 */
    log_len += log_strcpy(log_len, log_buf + log_len, CSI_START);
    log_len += log_strcpy(log_len, log_buf + log_len, color_output_info[level]);

    /* 增加level */
    log_len += snprintf(log_buf + log_len, LOG_LINE_BUF_SIZE - log_len, "[%s]", output_name[level]); // 固定宽度
    /* 增加tag function file line */
    char func_with_brackets[LOG_TAG_BUFF_SIZE];
    snprintf(func_with_brackets, sizeof(func_with_brackets), "[%s][%s();%s:%ld]", tag, func, file, line);
    log_len += snprintf(log_buf + log_len, LOG_LINE_BUF_SIZE - log_len, "%-*s\t", LOG_TAG_BUFF_SIZE, func_with_brackets); // 固定宽度

    va_start(args, format);
    /* 用vsnprintf构建输出的buff */
    fmt_result = vsnprintf(log_buf + log_len, (LOG_LINE_BUF_SIZE - log_len), format, args);
    va_end(args);

    /* 校验长度是否合法 */
    if ((log_len + fmt_result <= LOG_LINE_BUF_SIZE) && (fmt_result > -1)) {
        log_len += fmt_result;
    } else {
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

    if (s_log_info.log_output_handler != NULL) {
        s_log_info.log_output_handler(log_buf, log_len);
    }
    s_log_info.log_unlock_handler();
}

/**
 * @brief dump the hex format data to log(port from easylogger https://github.com/armink/EasyLogger/blob/cd93d9c768415f4b7279f2d3ef2366ce15ea087c/easylogger/src/elog.c#L863)
 * @param {char} *name
 * @param {uint8_t} width
 * @param {void} *buf
 * @param {uint16_t} size
 * @return {*}
 */
void log_hex_dump(const char *name, uint8_t width, const void *buf, uint16_t size)
{
#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')

    uint16_t i, j;
    uint16_t log_len = 0;
    const uint8_t *buf_p = buf;
    char dump_string[8] = { 0 };
    int fmt_result;
    char *log_buf = s_log_info.log_buf;
    /* lock output */
    s_log_info.log_lock_handler();

    for (i = 0; i < size; i += width) {
        /* package header */
        fmt_result = snprintf(log_buf, LOG_LINE_BUF_SIZE, "D/HEX %s: %04X-%04X: ", name, i, i + width - 1);
        /* calculate log length */
        if ((fmt_result > -1) && (fmt_result <= LOG_LINE_BUF_SIZE)) {
            log_len = fmt_result;
        } else {
            log_len = LOG_LINE_BUF_SIZE;
        }
        /* dump hex */
        for (j = 0; j < width; j++) {
            if (i + j < size) {
                snprintf(dump_string, sizeof(dump_string), "%02X ", buf_p[i + j]);
            } else {
                strncpy(dump_string, "   ", sizeof(dump_string));
            }
            log_len += log_strcpy(log_len, log_buf + log_len, dump_string);
            if ((j + 1) % 8 == 0) {
                log_len += log_strcpy(log_len, log_buf + log_len, " ");
            }
        }
        log_len += log_strcpy(log_len, log_buf + log_len, "  ");
        /* dump char for hex */
        for (j = 0; j < width; j++) {
            if (i + j < size) {
                snprintf(dump_string, sizeof(dump_string), "%c", __is_print(buf_p[i + j]) ? buf_p[i + j] : '.');
                log_len += log_strcpy(log_len, log_buf + log_len, dump_string);
            }
        }
        /* overflow check and reserve some space for newline sign */
        if (log_len + strlen(LOG_NEWLINE_SIGN) > LOG_LINE_BUF_SIZE) {
            log_len = LOG_LINE_BUF_SIZE - strlen(LOG_NEWLINE_SIGN);
        }
        /* package newline sign */
        log_len += log_strcpy(log_len, log_buf + log_len, LOG_NEWLINE_SIGN);
        /* do log output */
        s_log_info.log_output_handler(log_buf, log_len);
    }
    /* unlock output */
    s_log_info.log_unlock_handler();
}
/*---------- end of file ----------*/
