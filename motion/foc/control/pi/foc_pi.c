/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : foc_pi.c
 * @Author       : lxf
 * @Date         : 2026-04-02 16:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-04-03 13:51:22
 * @Brief        : FOC PI 控制器实现
 */

/*---------- includes ----------*/
#include "foc_pi.h"
#include "stdbool.h"
#include <stddef.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
static float _foc_pi_clamp(float value, float min_value, float max_value);
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
static float _foc_pi_clamp(float value, float min_value, float max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }

    return value;
}

void foc_pi_init(struct foc_pi *pi, float kp, float ki, float limit_abs)
{
    if (pi == NULL) {
        return;
    }

    pi->kp = kp;
    pi->ki = ki;
    pi->integral = 0.0f;
    pi->output = 0.0f;
    pi->integral_min = -limit_abs;
    pi->integral_max = limit_abs;
    pi->output_min = -limit_abs;
    pi->output_max = limit_abs;
    pi->enabled = false;
}

/**
 * @brief 启动pi回路计算
 * @param {foc_pi} *pi
 * @param {float} output
 * @return {*}
 */
void foc_pi_start(struct foc_pi *pi)
{
    if (pi == NULL) {
        return;
    }
    pi->enabled = true;
}

/**
 * @brief 停止pi回路计算。停止后可以维持output
 * @param {foc_pi} *pi
 * @param {float} output
 * @return {*}
 */
void foc_pi_stop(struct foc_pi *pi, float output)
{
    if (pi == NULL) {
        return;
    }
    pi->integral = 0.0f;
    pi->output = output;
    pi->enabled = false;
}

void foc_pi_set_output_limit(struct foc_pi *pi, float output_min, float output_max)
{
    if ((pi == NULL) || (output_min > output_max)) {
        return;
    }

    pi->output_min = output_min;
    pi->output_max = output_max;
}

void foc_pi_set_integral_limit(struct foc_pi *pi, float integral_min, float integral_max)
{
    if ((pi == NULL) || (integral_min > integral_max)) {
        return;
    }

    pi->integral_min = integral_min;
    pi->integral_max = integral_max;
}

/**
 * @brief pi闭环
 * @param {foc_pi} *pi
 * @param {float} reference 目标
 * @param {float} feedback 实际
 * @return {*}
 */
void foc_pi_run(struct foc_pi *pi, float reference, float feedback)
{
    float error = 0.0f;
    float output = 0.0f;

    if ((pi == NULL) || !pi->enabled) {
        return;
    }

    error = reference - feedback;
    pi->integral += pi->ki * error;
    pi->integral = _foc_pi_clamp(pi->integral, pi->integral_min, pi->integral_max);

    output = (pi->kp * error) + pi->integral;
    pi->output = _foc_pi_clamp(output, pi->output_min, pi->output_max);
}
/*---------- end of file ----------*/
