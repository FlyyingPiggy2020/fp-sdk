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
 * @FilePath     : fp_soft_timer.c
 * @Author       : lxf
 * @Date         : 2024-07-16 11:26:37
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-07-16 11:26:55
 * @Brief        : Implementation of software timers using linked lists.
 */


/*---------- includes ----------*/

#include "fp_soft_timer.h"
#include "stdbool.h"
#include "stdlib.h"
/*---------- macro ----------*/

#if FP_LOG_TRACE_TIMER
    #define TIMER_TRACE(...) printf(__VA_ARGS__)
#else
    #define TIMER_TRACE(...)
#endif
/*---------- type define ----------*/
/*---------- variable prototype ----------*/

static bool timer_created;
static bool fp_timer_run = false;
static uint8_t idle_last = 0;
static bool timer_deleted;
static bool timer_created;

static fp_timer_t *_fp_timer_act;

LIST_HEAD(_fp_timer_ll); //create a list head

/*---------- function prototype ----------*/

/*---------- variable ----------*/
/*---------- function ----------*/

void _fp_timer_core_init(void)
{
    fp_timer_enable(true);
}
uint32_t fp_tick_get(void)
{
    //TODO:
    return 0;
}

uint32_t fp_tick_elaps(uint32_t prev_tick)
{
    uint32_t act_time = fp_tick_get();

    /* If there is no overflow in sys_time simple subtract */
    if (act_time >= prev_tick) {
        prev_tick = act_time - prev_tick;
    }
    else { /* If there is overflow in sys_time, we need to handle it */
        prev_tick = (UINT32_MAX - prev_tick) + act_time + 1;
        prev_tick += act_time;
    }
    return prev_tick;
}

static uint32_t fp_timer_time_remaining(fp_timer_t *timer)
{
    /* Check if at least 'period' time elapsed */
    uint32_t elp = fp_tick_elaps(timer->last_run);
    if (elp >= timer->period) {
        return 0;
    }
    return timer->period - elp;
}

void fp_timer_del(fp_timer_t *timer)
{
    timer_deleted = true;
    free(timer);
}
static bool fp_timer_exec(fp_timer_t *timer)
{
    if (timer->paused) {
        return false;
    }
    bool exec = false;

    if (fp_timer_time_remaining(timer) == 0) {
        /* Decrement the repeat count before executing the timer_cb.
         * If any timer is deleted `if(timer->repeat_count == 0)` is not executed below
         * but at least the repeat count is zero and the timer can be deleted in the next round*/
        int32_t original_repeat_count = timer->repeat_count;
        if (timer->repeat_count > 0) {
            timer->repeat_count--;
        }
        timer->last_run = fp_tick_get();
        TIMER_TRACE("calling timer callback: %p", *((void **)&timer->timer_cb));
        if (timer->timer_cb && original_repeat_count != 0) {
            timer->timer_cb(timer->user_data);
            TIMER_TRACE("timer callback %p finished", *((void **)&timer->timer_cb));
            exec = true;
        }
    }

    if (timer_deleted == false) {
        if (timer->repeat_count == 0) { /*The repeat count is over, delete the timer*/
            TIMER_TRACE("deleting timer with %p callback because the repeat count is over", *((void **)&timer->timer_cb));
            fp_timer_del(timer);
        }
    }
    return exec;
}
uint32_t fp_timer_handler(void)
{
    TIMER_TRACE("begin\n");

    static bool already_running = false;
    if(already_running) {
        TIMER_TRACE("already running\n");
        return 1;
    }
    already_running = true;

    if (fp_timer_run == false) {
        already_running = false;
        return 1;
    }

    static uint32_t idle_period_start = 0;
    static uint32_t busy_time = 0;

    uint32_t handler_start = fp_tick_get();

    if (handler_start == 0) {
        static uint32_t run_cnt = 0;
        run_cnt++;
        if (run_cnt > 100) {
            run_cnt = 0;
            TIMER_TRACE("It seems fp_tick_inc() is not called\n");
        }
    }

    fp_timer_t *next;
    do {
        timer_created = false;
        timer_deleted = false;
        _fp_timer_act = NULL;
    } while (_fp_timer_act);
    
    return 0;
}

fp_timer_t *fp_timer_create(fp_tiemr_cb_t timer_xcb,uint32_t period, void *user_data)
{
    fp_timer_t *new_timer = NULL;

    new_timer = (fp_timer_t *)malloc(sizeof(fp_timer_t));
    if(new_timer == NULL)
    {
        return NULL;
    }
    new_timer->period = period;
    new_timer->timer_cb = timer_xcb;
    new_timer->repeat_count = -1;
    new_timer->paused = 0;
    new_timer->last_run = fp_tick_get();
    new_timer->user_data = user_data;

    timer_created = true;
    return new_timer;
}

void fp_timer_enable(bool en)
{
    fp_timer_run = en;
}
/*---------- end of file ----------*/


