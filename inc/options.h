/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : options.h
 * @Author       : lxf
 * @Date         : 2025-08-04 11:53:56
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-20 13:52:10
 * @Brief        : 系统配置选项入口文件，包含系统API封装、内存管理算法选择、调试日志接口等
 */

#ifndef __OPTIONS_H__
#define __OPTIONS_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "define.h"
#include "misc.h"
#include "errorno.h"
#include "event.h"
#include "clists.h"
/**
 * @brief 你可以参考board_options_sample.h构建出自己工程的配置文件
 * @return {*}
 */
#ifdef FP_INCLDUE_BOARD_OPTIONS_FILE
#include "board_options.h"
#endif

#include "fmath.h"

// 动态内存管理算法选择
// 0: 用户自定义动态内存管理算法
// 1: 系统默认动态内存管理算法heap4
#if (0 == CONFIG_HEAP_TYPE)
#if !defined(__MALLOC_FREE_CHECKED__)
#define __MALLOC_FREE_CHECKED__
#if !defined(__malloc) || !defined(__free)
#error \
    "Error: CONFIG_HEAP_TYPE is set to 0 (user-defined), but __malloc or __free is not defined. Please define these functions or change CONFIG_HEAP_TYPE to 1."
#endif
#endif

#elif (1 == CONFIG_HEAP_TYPE)
#include "heap.h"
#undef __malloc
#undef __free
#define __malloc(size) __heap_malloc(size)
#define __free(ptr)    __heap_free(ptr)
#endif

/**
 * @brief 系统API
 * @return {*}
 */
#define delay_ms(ms)              __delay_ms(ms)
#define delay_us(us)              __delay_us(us)
#define get_ticks()               __get_ticks()
#define get_ticks_from_isr()      __get_ticks_from_isr()
#define reset_system()            __reset_system()
#define enter_critical()          __enter_critical()
#define exit_critical()           __exit_critical()
#define enter_critical_from_isr() __enter_critical_from_isr()
#define exit_critical_from_isr()  __exit_critical_from_isr()
#define malloc(size)              __malloc(size)
#define free(ptr)                 __free(ptr)
#define ticks2ms(ticks)           __ticks2ms(ticks)
#define ms2ticks(ms)              __ms2ticks(ms)

/*---------- macro ----------*/

/**
 * @brief 日志error输出接口
 * @return {*}
 */
#ifndef xlog_error
#define xlog_error(x, ...)
#endif

/**
 * @brief 原生调试口
 * @return {*}
 */
#ifndef xlog_count
#define xlog_count(x, ...)
#endif

/**
 * @brief __FILE_NAME__定义给assert断言函数使用
 * @return {*}
 */
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif

/**
 * @brief assert接口
 * @return {*}
 */
#ifndef assert
#define assert(expr)                                                  \
    do {                                                              \
        if (!(expr)) {                                                \
            xlog_error("Assert in %s:%d\n", __FILE_NAME__, __LINE__); \
            for (;;)                                                  \
                ;                                                     \
        }                                                             \
    } while (0)
#endif

/**
 * @brief 数组HEX输出
 * @return {*}
 */
#ifndef PRINT_BUFFER_CONTENT
#define PRINT_BUFFER_CONTENT(color, tag, buf, length) \
    do {                                              \
        if (!length) {                                \
            break;                                    \
        }                                             \
        xlog_count("%s%s: ", color, tag);             \
        for (uint32_t i = 0; i < length; ++i) {       \
            xlog_count("%02X ", buf[i]);              \
        }                                             \
        xlog_count("\b\n");                           \
    } while (0);
#endif
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
