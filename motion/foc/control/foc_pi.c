/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_pi.c
 * @Author       : Codex
 * @Date         : 2026-03-16
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-17
 * @Brief        : FOC 域专用 PI 控制器
 */

/*---------- includes ----------*/
#include "foc_pi.h"
#include <stddef.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  对 PI 中间量执行通用限幅
 * @param  value: 输入值
 * @param  min_value: 下限
 * @param  max_value: 上限
 * @return 限幅后的结果
 */
static foc_scalar_t _foc_pi_clamp(foc_scalar_t value, foc_scalar_t min_value, foc_scalar_t max_value);
/*---------- variable ----------*/
/*---------- function ----------*/
static foc_scalar_t _foc_pi_clamp(foc_scalar_t value, foc_scalar_t min_value, foc_scalar_t max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

/**
 * @brief  初始化 PI 控制器
 * @param  pi: PI 控制器对象
 * @param  kp: 比例系数
 * @param  ki: 积分系数
 * @return 无
 */
void foc_pi_init(foc_pi_t *pi, foc_scalar_t kp, foc_scalar_t ki)
{
    if (pi == NULL) {
        return;
    }

    pi->kp = kp;
    pi->ki = ki;
    pi->integral = 0.0f;
    pi->integral_min = -1.0f;
    pi->integral_max = 1.0f;
    pi->output = 0.0f;
    pi->output_min = -1.0f;
    pi->output_max = 1.0f;
    pi->enabled = true;
}

/**
 * @brief  清零 PI 控制器内部状态
 * @param  pi: PI 控制器对象
 * @return 无
 */
void foc_pi_reset(foc_pi_t *pi)
{
    if (pi == NULL) {
        return;
    }

    pi->integral = 0.0f;
    pi->output = 0.0f;
}

/**
 * @brief  设置 PI 输出限幅
 * @param  pi: PI 控制器对象
 * @param  output_min: 输出下限
 * @param  output_max: 输出上限
 * @return 无
 */
void foc_pi_set_output_limit(foc_pi_t *pi, foc_scalar_t output_min, foc_scalar_t output_max)
{
    if ((pi == NULL) || (output_min > output_max)) {
        return;
    }

    pi->output_min = output_min;
    pi->output_max = output_max;
}

/**
 * @brief  设置 PI 积分项限幅
 * @param  pi: PI 控制器对象
 * @param  integral_min: 积分下限
 * @param  integral_max: 积分上限
 * @return 无
 */
void foc_pi_set_integral_limit(foc_pi_t *pi, foc_scalar_t integral_min, foc_scalar_t integral_max)
{
    if ((pi == NULL) || (integral_min > integral_max)) {
        return;
    }

    pi->integral_min = integral_min;
    pi->integral_max = integral_max;
}

/**
 * @brief  执行一次 PI 计算
 * @param  pi: PI 控制器对象
 * @param  reference: 参考输入
 * @param  feedback: 反馈输入
 * @return 控制器输出
 */
foc_scalar_t foc_pi_run(foc_pi_t *pi, foc_scalar_t reference, foc_scalar_t feedback)
{
    foc_scalar_t error;

    if ((pi == NULL) || !pi->enabled) {
        return 0.0f;
    }

    error = reference - feedback;

    /* 第一版先使用最基础的 PI 形式，后续再补 anti-windup 等能力 */
    pi->integral += pi->ki * error;
    pi->integral = _foc_pi_clamp(pi->integral, pi->integral_min, pi->integral_max);

    pi->output = (pi->kp * error) + pi->integral;
    pi->output = _foc_pi_clamp(pi->output, pi->output_min, pi->output_max);
    return pi->output;
}
/*---------- end of file ----------*/
