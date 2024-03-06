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
 * @FilePath     : fp_def.h
 * @Author       : lxf
 * @Date         : 2023-12-20 16:57:16
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2023-12-20 16:58:28
 * @Brief        : 提供一些公共的定义
 */
#ifndef __FP_DEF_H__
#define __FP_DEF_H__
/*---------- includes ----------*/
#include "main.h"
#include "stdint.h"
/*---------- macro ----------*/

#define DISABLE_IRQ            __disable_irq
#define ENABLE_IRQ             __enable_irq
/**
 * @brief 错误码
 * @return {*}
 */
#define FP_EOK                 0 /**< 没有发生错误 */
#define FP_ERROR               1 /**< 通用错误/未知错误 */
#define FP_ETIMEOUT            2 /**< 超时错误 */
#define FP_EFULL               3 /**< 满 */
#define FP_EEMPTY              4 /**< 空 */
#define FP_ENOMEM              5 /**< 内存不足 */
#define FP_ENOSYS              6 /**< 函数未实现 */
#define FP_EBUSY               7 /**< 忙 */
#define FP_EIO                 8 /**< IO错误 */
// #define FP_EINTR    9  /**< Interrupted system call */
// #define FP_EINVAL   10 /**< Invalid argument */
// #define FP_ENOENT   11 /**< No entry */
// #define FP_ENOSPC   12 /**< No space left */
// #define FP_EPERM    13 /**< Operation not permitted */
// #define FP_ETRAP    14 /**< Trap event */
// #define FP_EFAULT   15 /**< Bad address */

/**
 * @brief 设备标志
 * @return {*}
 */
#define DEVICE_FLAG_DEACTIVATE 0x000 /**< 设备未初始化 */

#define DEVICE_FLAG_RDONLY     0x001 /**< 以只读方式打开设备 */
#define DEVICE_FLAG_WRONLY     0x002 /**< 以只写方式打开设备 */
#define DEVICE_FLAG_RDWR       0x003 /**< 以读写方式打开设备 */

#define DEVICE_FLAG_REMOVABLE  0x004 /**< 可移动设备标志，表示设备可插拔 */
#define DEVICE_FLAG_STANDALONE 0x008 /**< 独立设备，表示设备不依赖其他设备 */
#define DEVICE_FLAG_ACTIVATED  0x010 /**< 设备已经初始化表示 */
#define DEVICE_FLAG_SUSPENDED  0x020 /**< 设备挂起 */
#define DEVICE_FLAG_STREAM     0x040 /**< 流模式设备表示设备支持流式传输 */

#define DEVICE_FLAG_INT_RX     0x100 /**< INT mode on Rx */
#define DEVICE_FLAG_DMA_RX     0x200 /**< DMA mode on Rx */
#define DEVICE_FLAG_INT_TX     0x400 /**< INT mode on Tx */
#define DEVICE_FLAG_DMA_TX     0x800 /**< DMA mode on Tx */

/**
 * @brief 设备标志
 * @return {*}
 */
#define DEVICE_OFLAG_CLOSE     0x000 /**< 设备已关闭 */
#define DEVICE_OFLAG_RDONLY    0x001 /**< 设备只读 */
#define DEVICE_OFLAG_WRONLY    0x002 /**< 设备只写 */
#define DEVICE_OFLAG_RDWR      0x003 /**< 设备读写 */
#define DEVICE_OFLAG_OPEN      0x008 /**< 设备已打开 */
#define DEVICE_OFLAG_MASK      0xf0f /**< mask of open flag */

#if defined(__ARMCC_VERSION)
#define fp_section(x)    __attribute__((section(x))) /**< 将变量或函数放入指定的代码段 */
#define fp_used          __attribute__((used))       /**< 让编译器保留变量或函数即使它没被使用 */
#define fp_align(n)      __attribute__((aligned(n))) /**< 按n字节对齐 */
#define fp_weak          __attribute__((weak))       /**< 弱定义 */
#define fp_inline        static __inline             /**< 内联 */
#define fp_always_inline fp_inline                   /**< 内联 */
#endif
/*---------- type define ----------*/

typedef int32_t fp_err_t;  /**< 错误类型 */
typedef int32_t fp_size_t; /**< 数据大小*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#endif
