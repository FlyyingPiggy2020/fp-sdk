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
     * 根据目标时间戳返回一份可供当前电流环使用的机械角结果。
     * FOC 核心会在 SDK 内统一完成“机械角 -> 电角度”的换算，
     * 板级层只负责提供原始角度观测结果，不再承载极对数、方向和零位偏移逻辑。
     */
    void (*get_mechanical_angle)(void *user_data, uint32_t target_tick_us, foc_mechanical_angle_sample_t *sample);

    /* 输出归一化占空比到具体 PWM 外设 */
    void (*write_pwm_duty)(void *user_data, const foc_pwm_duty_t *duty);
    void (*set_output_enable)(void *user_data, bool enable);
} foc_hal_ops_t;

#endif
