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
 * @FilePath     : fp_soft_timer.h
 * @Author       : lxf
 * @Date         : 2024-07-16 11:26:46
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-07-16 11:27:05
 * @Brief        : 
 */

#ifndef __FP_SOFT_TIMER_H__
#define __FP_SOFT_TIMER_H__
/*---------- includes ----------*/
#include "../../fp_sdk.h"
#include <stdint.h>
#include <stdio.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
struct _fp_timer_t;
typedef void (*fp_tiemr_cb_t)(struct _fp_timer_t *);

typedef struct _fp_timer_t {
    uint32_t period;
    uint32_t last_run;
    fp_tiemr_cb_t timer_cb;
    void *user_data;
    int32_t repeat_count;
    uint32_t paused :1;
    struct list_head list;
}fp_timer_t;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

fp_timer_t *fp_timer_create(fp_tiemr_cb_t timer_xcb,uint32_t period, void *user_data);
uint32_t fp_timer_handler(void);
void fp_timer_enable(bool en);
/*---------- end of file ----------*/
#endif