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
 * @FilePath     : log_cfg.h
 * @Author       : lxf
 * @Date         : 2023-12-12 15:36:14
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2023-12-12 15:36:15
 * @Brief        : 日志配置文件
 */

#ifndef __LOG_CFG_H__
#define __LOG_CFG_H__
/*---------- includes ----------*/
/*---------- macro ----------*/

/**
 * @brief 最大输出长度
 * @return {*}
 */
#define LOG_LINE_BUF_SIZE    256

/**
 * @brief 数字长度
 * @return {*}
 */
#define LOG_LINE_NUM_MAX_LEN 5
/**
 * @brief 换行标志
 * @return {*}
 */
#define LOG_NEWLINE_SIGN     "\r\n"
/**
 * @brief 日志输出级别
 * @return {*}
 */
#define LOG_OUTPUT_LVL       LOG_LVL_VERBOSE

/**
 * @brief 是否使用RTOS
 * @return {*}
 */
#define LOG_USE_RTOS        0

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#endif
