/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_profile.c
 * @Author       : Codex
 * @Date         : 2026-03-16
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-17
 * @Brief        : 电机与 FOC 参数配置实现
 */

/*---------- includes ----------*/
#include "foc_profile.h"
#include <stddef.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
/**
 * @brief  初始化电机本体参数为默认值
 * @param  profile: 电机参数对象
 * @return 无
 */
void motor_profile_init(motor_profile_t *profile)
{
    if (profile == NULL) {
        return;
    }

    profile->pole_pairs = 1U;
    profile->electrical_direction = 1;
    profile->electrical_offset = 0U;
    profile->rs_ohm = 0.0f;
    profile->ld_h = 0.0f;
    profile->lq_h = 0.0f;
    profile->flux_linkage = 0.0f;
    profile->current_limit = 0.0f;
    profile->speed_limit_rpm = 0.0f;
}

/**
 * @brief  初始化功率板参数为默认值
 * @param  profile: 功率板参数对象
 * @return 无
 */
void power_stage_profile_init(power_stage_profile_t *profile)
{
    if (profile == NULL) {
        return;
    }

    profile->pwm_frequency_hz = 0U;
    profile->deadtime_ns = 0U;
    profile->adc_full_scale = 0U;
    profile->adc_ref_voltage = 0.0f;
    profile->shunt_resistor_ohm = 0.0f;
    profile->current_sense_gain = 0.0f;
    profile->bus_voltage_ratio = 0.0f;
    profile->over_current_limit = 0.0f;
}

/**
 * @brief  初始化传感器参数为默认值
 * @param  profile: 传感器参数对象
 * @return 无
 */
void sensor_profile_init(sensor_profile_t *profile)
{
    if (profile == NULL) {
        return;
    }

    profile->encoder_cpr = 0U;
    profile->phase_current_channel_count = 2U;
    profile->encoder_direction = 1;
    profile->electrical_offset = 0U;
    profile->current_a_offset = 0.0f;
    profile->current_b_offset = 0.0f;
    profile->current_c_offset = 0.0f;
}

/**
 * @brief  初始化控制器参数为默认值
 * @param  cfg: 控制器配置对象
 * @return 无
 */
void foc_ctrl_cfg_init(foc_ctrl_cfg_t *cfg)
{
    if (cfg == NULL) {
        return;
    }

    cfg->id_kp = 0.0f;
    cfg->id_ki = 0.0f;
    cfg->iq_kp = 0.0f;
    cfg->iq_ki = 0.0f;
    cfg->speed_kp = 0.0f;
    cfg->speed_ki = 0.0f;
    cfg->id_limit = 0.0f;
    cfg->iq_limit = 0.0f;
    cfg->voltage_limit = 1.0f;
    cfg->current_loop_hz = 0U;
    cfg->speed_loop_hz = 0U;
}

/**
 * @brief  校验电机本体参数是否合法
 * @param  profile: 电机参数对象
 * @return true=合法, false=非法
 */
bool motor_profile_is_valid(const motor_profile_t *profile)
{
    if (profile == NULL) {
        return false;
    }

    /* 这里只做基础校验，具体业务约束由上层继续补充 */
    return (profile->pole_pairs > 0U) && (profile->current_limit > 0.0f);
}

/**
 * @brief  校验功率板参数是否合法
 * @param  profile: 功率板参数对象
 * @return true=合法, false=非法
 */
bool power_stage_profile_is_valid(const power_stage_profile_t *profile)
{
    if (profile == NULL) {
        return false;
    }

    return (profile->pwm_frequency_hz > 0U) &&
           (profile->adc_full_scale > 0U) &&
           (profile->current_sense_gain > 0.0f);
}

/**
 * @brief  校验传感器参数是否合法
 * @param  profile: 传感器参数对象
 * @return true=合法, false=非法
 */
bool sensor_profile_is_valid(const sensor_profile_t *profile)
{
    if (profile == NULL) {
        return false;
    }

    return (profile->phase_current_channel_count >= 2U);
}

/**
 * @brief  校验控制器参数是否合法
 * @param  cfg: 控制器配置对象
 * @return true=合法, false=非法
 */
bool foc_ctrl_cfg_is_valid(const foc_ctrl_cfg_t *cfg)
{
    if (cfg == NULL) {
        return false;
    }

    return (cfg->current_loop_hz > 0U) &&
           (cfg->speed_loop_hz > 0U) &&
           (cfg->voltage_limit > 0.0f);
}
/*---------- end of file ----------*/
