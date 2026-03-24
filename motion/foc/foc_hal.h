/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_hal.h
 * @Author       : Codex
 * @Date         : 2026-03-16
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-16
 * @Brief        : FOC 通用端口接口
 */

#ifndef __FOC_HAL_H__
#define __FOC_HAL_H__

/*---------- includes ----------*/
#include "foc_types.h"
#include <stdbool.h>
#include <stdint.h>

/*---------- type define ----------*/
typedef struct {
    /* FOC 核心只依赖这组端口回调，不直接依赖任何具体驱动框架 */

    /* 读取与 PWM 采样点对齐的电流环输入样本 */
    void (*read_current_loop_sample)(void *user_data, foc_current_loop_sample_t *sample);

    /*
     * 根据目标时间戳返回一份可供当前电流环使用的电角度结果。
     * 这里允许角度源与电流采样异步：
     * 1. 磁编、霍尔等传感器可以先异步更新原始角度；
     * 2. 角度提供器再根据 target_tick_us 做对齐、插值或外推；
     * 3. 当前版本默认调用方认为返回角度可消费，非可消费状态后续再做专门策略。
     */
    void (*get_electrical_angle)(void *user_data, uint32_t target_tick_us, foc_angle_sample_t *sample);

    /* 输出归一化占空比到具体 PWM 外设 */
    void (*write_pwm_duty)(void *user_data, const foc_pwm_duty_t *duty);
    void (*set_output_enable)(void *user_data, bool enable);
} foc_hal_ops_t;

#endif
