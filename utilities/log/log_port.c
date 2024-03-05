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
 * @FilePath     : log_port.c
 * @Author       : lxf
 * @Date         : 2023-12-12 15:07:00
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2023-12-12 15:07:01
 * @Brief        : 日志移植接口
 */

/*---------- includes ----------*/

#include "inc/log.h"
#include "inc/log_cfg.h"

#if (LOG_USE_RTOS == 1)
#include "cmsis_os.h"
#include "main.h"

#endif
/*---------- macro ----------*/
/*---------- type define ----------*/

/*---------- variable prototype ----------*/

#if (LOG_USE_RTOS == 1)
static osMutexId_t mutex_id;
#endif
/*---------- function prototype ----------*/

void (*log_port_output)(const char *log, size_t size) = NULL;
/*---------- variable ----------*/

/*---------- function ----------*/

/**
 * @brief 初始化输出
 * @return {*}
 */
bool log_port_init(void)
{
#if (LOG_USE_RTOS == 1)
    mutex_id = osMutexNew(NULL);
#endif
    if (log_port_output == NULL) {
        return false;
    }
    return true;
}
/**
 * @brief 定义输出函数
 * @return {*}
 */
void set_log_port_output(void *output)
{
    log_port_output = output;
}

/**
 * @brief 日志输出上锁，防止多线程时调用顺序出错。
 * @return {*}
 */
void log_output_lock(void)
{
#if (LOG_USE_RTOS == 1)
    osMutexAcquire(mutex_id, osWaitForever);
#endif
    return;
}

/**
 * @brief 日志输出解锁。
 * @return {*}
 */
void log_output_unlock(void)
{
#if (LOG_USE_RTOS == 1)
    osMutexRelease(mutex_id);
#endif
    return;
}
/*---------- end of file ----------*/
