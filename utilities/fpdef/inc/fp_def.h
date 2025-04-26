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
#include "stdint.h"

#if (__ARMCC_VERSION >= 6000000)
#include "cmsis_armclang.h"
#endif
/*---------- macro ----------*/
#if defined(__ARMCC_VERSION)                         // armcc or armclang compliter
#define fp_section(x)    __attribute__((section(x))) /**< 将变量或函数放入指定的代码段 */
#define fp_used          __attribute__((used))       /**< 让编译器保留变量或函数即使它没被使用 */
#define fp_align(n)      __attribute__((aligned(n))) /**< 按n字节对齐 */
#define fp_weak          __attribute__((weak))       /**< 弱定义 */
#define fp_inline        static __inline             /**< 内联 */
#define fp_always_inline fp_inline                   /**< 内联 */
#define DISABLE_IRQ      __disable_irq
#define ENABLE_IRQ       __enable_irq
#elif defined(__GNUC__)                              // gcc compliter
#define fp_section(x)    __attribute__((section(x))) /**< 将变量或函数放入指定的代码段 */
#define fp_used          __attribute__((used))       /**< 让编译器保留变量或函数即使它没被使用 */
#define fp_align(n)      __attribute__((aligned(n))) /**< 按n字节对齐 */
#define fp_weak          __attribute__((weak))       /**< 弱定义 */
#define fp_inline        static __inline             /**< 内联 */
#define fp_always_inline fp_inline                   /**< 内联 */
#define DISABLE_IRQ
#define ENABLE_IRQ
#else
#error "don't support your arch"
#endif
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#endif
