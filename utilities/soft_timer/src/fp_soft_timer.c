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
#define LOG_TAG "SOFT_TIME_KERNEL"
#include "fplog.h"
#define TIMER_TRACE(...) log_d(__VA_ARGS__)
#else
#define TIMER_TRACE(...)
#endif

#define IDLE_MEAS_PERIOD 500 /*[ms]*/
#define DEF_PERIOD       500
/*---------- type define ----------*/
/*---------- variable prototype ----------*/

static bool fp_timer_run = false;
// static uint8_t idle_last = 0;
static bool timer_deleted;
static bool timer_created;

static fp_timer_t *_fp_timer_act;

static uint32_t sys_time = 0;
static volatile uint8_t tick_irq_flag;
LIST_HEAD(_fp_timer_ll); // create a list head

/*---------- function prototype ----------*/

/*---------- variable ----------*/
/*---------- function ----------*/

/**
 * @brief 软件定时器使能
 * @return {*}
 */
void _fp_timer_core_init(void)
{
    fp_timer_enable(true);
}

/**
 * @brief 软件定时器tick 增加
 * @param {uint32_t} tick_period
 * @return {*}
 */
inline void fp_tick_inc(uint32_t tick_period)
{
    tick_irq_flag = 0;
    sys_time += tick_period;
}

uint32_t fp_tick_get(void)
{
    uint32_t result;
    do {
        tick_irq_flag = 1;
        result = sys_time;
    } while (!tick_irq_flag);
    return result;
}

uint32_t fp_tick_elaps(uint32_t prev_tick)
{
    uint32_t act_time = fp_tick_get();

    /* If there is no overflow in sys_time simple subtract */
    if (act_time >= prev_tick) {
        prev_tick = act_time - prev_tick;
    } else { /* If there is overflow in sys_time, we need to handle it */
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

bool fp_timer_del(fp_timer_t *timer)
{
    timer_deleted = true;
    bool ret = false;
    struct list_head *pos, *tmp;

    if (timer == NULL) {
        return ret;
    }

    list_for_each_safe(pos, tmp, &_fp_timer_ll)
    {
        fp_timer_t *entry = list_entry(pos, fp_timer_t, list);
        if (entry == timer) {
            list_del(pos);
            free(entry);
            ret = true;
            break;
        }
    }
    return ret;
}

/**
 * @brief it will delete timer when over time
 * @param {fp_timer_t} *timer
 * @return {*}
 */
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
    static bool already_running = false;
    if (already_running) {
        TIMER_TRACE("already running\n");
        return 1;
    }
    already_running = true;

    if (fp_timer_run == false) {
        already_running = false;
        return 2;
    }

    uint32_t handler_start = fp_tick_get();

    if (handler_start == 0) {
        static uint32_t run_cnt = 0;
        run_cnt++;
        if (run_cnt > 100) {
            run_cnt = 0;
            TIMER_TRACE("It seems fp_tick_inc() is not called\n");
        }
    }

    struct list_head *pos, *tmp;
    uint32_t time_till_next = FP_NO_TIMER_READY;
    do {
        list_for_each_safe(pos, tmp, &_fp_timer_ll)
        {
            timer_created = false;
            timer_deleted = false;
            _fp_timer_act = list_entry(pos, fp_timer_t, list);
            if (fp_timer_exec(_fp_timer_act)) {
                if (timer_created || timer_deleted) {
                    TIMER_TRACE("Start from the first timer again because a timer was created or deleted");
                    break;
                }
            }
        }
    } while (pos != (&_fp_timer_ll));

    list_for_each_safe(pos, tmp, &_fp_timer_ll)
    {
        _fp_timer_act = list_entry(pos, fp_timer_t, list);
        if (!_fp_timer_act->paused) {
            uint32_t delay = fp_timer_time_remaining(_fp_timer_act);
            if (delay < time_till_next) {
                time_till_next = delay;
            }
        }
    }

    already_running = false;
    // TIMER_TRACE("finished (%ld ms until the next timer call)\n", time_till_next);
    return time_till_next;
}

/**
 * @brief 创建一个软件定时器
 * @param {fp_tiemr_cb_t} timer_xcb 控制块
 * @param {uint32_t} period 周期
 * @param {void} *user_data 用户数据
 * @return {*}
 */
fp_timer_t *fp_timer_create(fp_tiemr_cb_t timer_xcb, uint32_t period, void *user_data)
{
    fp_timer_t *new_timer = NULL;

    new_timer = (fp_timer_t *)malloc(sizeof(fp_timer_t));
    if (new_timer == NULL) {
        return NULL;
    }
    list_add_tail(&new_timer->list, &_fp_timer_ll);
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

void fp_timer_pasue(fp_timer_t *timer)
{
    timer->paused = true;
}

void fp_timer_resume(fp_timer_t *timer)
{
    timer->paused = false;
}

void fp_timer_ready(fp_timer_t *timer)
{
    timer->last_run = fp_tick_get() - timer->period - 1;
}

void fp_timer_reset(fp_timer_t *timer)
{
    timer->last_run = fp_tick_get();
}

/**
 * @brief Set the number of times a timer will repeat.
 * @param {fp_timer_t} *timer pointer to the timer
 * @param {int32_t} repeat_count -1: infinite repeat, 0: no repeat, >0: repeat n times
 * @return {*}
 */
void fp_timer_set_repeat_count(fp_timer_t *timer, int32_t repeat_count)
{
    timer->repeat_count = repeat_count;
}

/**
 * @brief Set new period of the timer
 * @param {fp_timer_t} *timer
 * @param {uint32_t} period new period
 * @return {*}
 */
void fp_timer_set_period(fp_timer_t *timer, uint32_t period)
{
    timer->period = period;
}

fp_timer_t *fp_timer_get_next(fp_timer_t *timer)
{
    if (timer == NULL || timer->list.next == NULL) {
        return NULL;
    } else {
        return list_next_entry(timer, fp_timer_t, list);
    }
}
/*---------- end of file ----------*/
