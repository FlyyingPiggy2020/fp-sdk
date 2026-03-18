/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_pi.h
 * @Author       : Codex
 * @Date         : 2026-03-16
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-17
 * @Brief        : FOC 域专用 PI 控制器
 */

#ifndef __FOC_PI_H__
#define __FOC_PI_H__

/*---------- includes ----------*/
#include "../math/foc_math_types.h"
#include <stdbool.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
typedef struct {
    /* 比例与积分参数 */
    foc_scalar_t kp;
    foc_scalar_t ki;

    /* 控制器内部状态 */
    foc_scalar_t integral;
    foc_scalar_t integral_min;
    foc_scalar_t integral_max;
    foc_scalar_t output;
    foc_scalar_t output_min;
    foc_scalar_t output_max;
    bool enabled;
} foc_pi_t;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  初始化 PI 控制器
 * @param  pi: PI 控制器对象
 * @param  kp: 比例系数
 * @param  ki: 积分系数
 * @return 无
 */
void foc_pi_init(foc_pi_t *pi, foc_scalar_t kp, foc_scalar_t ki);

/**
 * @brief  清零 PI 控制器内部状态
 * @param  pi: PI 控制器对象
 * @return 无
 */
void foc_pi_reset(foc_pi_t *pi);

/**
 * @brief  设置 PI 输出限幅
 * @param  pi: PI 控制器对象
 * @param  output_min: 输出下限
 * @param  output_max: 输出上限
 * @return 无
 */
void foc_pi_set_output_limit(foc_pi_t *pi, foc_scalar_t output_min, foc_scalar_t output_max);

/**
 * @brief  设置 PI 积分项限幅
 * @param  pi: PI 控制器对象
 * @param  integral_min: 积分下限
 * @param  integral_max: 积分上限
 * @return 无
 */
void foc_pi_set_integral_limit(foc_pi_t *pi, foc_scalar_t integral_min, foc_scalar_t integral_max);

/**
 * @brief  执行一次 PI 计算
 * @param  pi: PI 控制器对象
 * @param  reference: 参考输入
 * @param  feedback: 反馈输入
 * @return 控制器输出
 */
foc_scalar_t foc_pi_run(foc_pi_t *pi, foc_scalar_t reference, foc_scalar_t feedback);
/*---------- end of file ----------*/
#endif
