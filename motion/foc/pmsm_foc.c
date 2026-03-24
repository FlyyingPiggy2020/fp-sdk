/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : pmsm_foc.c
 * @Author       : Codex
 * @Date         : 2026-03-16
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-23 16:44:28
 * @Brief        : PMSM FOC 控制器骨架
 */

/*---------- includes ----------*/
#include "foc_pi.h"
#include "foc_svpwm.h"
#include "foc_transform.h"
#include "pmsm_foc.h"
#include <string.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  检查控制器绑定的参数对象是否可用
 * @param  foc: 控制器对象
 * @return true=可用, false=不可用
 */
static bool _pmsm_foc_profile_ready(const pmsm_foc_t *foc);

/**
 * @brief  关闭功率输出并回到安全占空比
 * @param  foc: 控制器对象
 * @return 无
 */
static void _pmsm_foc_disable_output(pmsm_foc_t *foc);
/*---------- variable ----------*/
/*---------- function ----------*/
static bool _pmsm_foc_profile_ready(const pmsm_foc_t *foc)
{
    if (foc == NULL) {
        return false;
    }

    return motor_profile_is_valid(foc->motor_profile) && power_stage_profile_is_valid(foc->power_stage_profile)
           && sensor_profile_is_valid(foc->sensor_profile) && foc_ctrl_cfg_is_valid(foc->ctrl_cfg);
}

static void _pmsm_foc_disable_output(pmsm_foc_t *foc)
{
    foc_pwm_duty_t duty = { 0 };

    /* 停机时先回到 50% 占空比中点 */
    duty.duty_a = 0.5f;
    duty.duty_b = 0.5f;
    duty.duty_c = 0.5f;
    duty.sector = 0U;

    if ((foc != NULL) && (foc->hal_ops.write_pwm_duty != NULL)) {
        foc->hal_ops.write_pwm_duty(foc->hal_user_data, &duty);
    }
    if ((foc != NULL) && (foc->hal_ops.set_output_enable != NULL)) {
        foc->hal_ops.set_output_enable(foc->hal_user_data, false);
    }
}

/**
 * @brief  初始化 PMSM FOC 控制器
 * @param  foc: 控制器对象
 * @param  motor_profile: 电机参数
 * @param  power_stage_profile: 功率板参数
 * @param  sensor_profile: 传感器参数
 * @param  ctrl_cfg: 控制器配置
 * @param  hal_ops: 板级硬件操作接口
 * @param  hal_user_data: 板级私有上下文
 * @return true=成功, false=失败
 */
bool pmsm_foc_init(pmsm_foc_t *foc,
                   const motor_profile_t *motor_profile,
                   const power_stage_profile_t *power_stage_profile,
                   const sensor_profile_t *sensor_profile,
                   const foc_ctrl_cfg_t *ctrl_cfg,
                   const foc_hal_ops_t *hal_ops,
                   void *hal_user_data)
{
    if ((foc == NULL) || (motor_profile == NULL) || (power_stage_profile == NULL) || (sensor_profile == NULL)
        || (ctrl_cfg == NULL) || (hal_ops == NULL)) {
        FOC_LOG_ERROR("pmsm_foc_init args invalid\r\n");
        return false;
    }

    memset(foc, 0, sizeof(*foc));

    /* 绑定静态配置与板级回调 */
    foc->motor_profile = motor_profile;
    foc->power_stage_profile = power_stage_profile;
    foc->sensor_profile = sensor_profile;
    foc->ctrl_cfg = ctrl_cfg;
    foc->hal_ops = *hal_ops;
    foc->hal_user_data = hal_user_data;
    foc->runtime.mode = FOC_MODE_STOP;

    /* 初始化各控制环 */
    foc_pi_init(&foc->id_pi, ctrl_cfg->id_kp_pu, ctrl_cfg->id_ki_pu);
    foc_pi_init(&foc->id_pi, ctrl_cfg->id_kp_pu, ctrl_cfg->id_ki_pu);
    // foc_pi_init(&foc->speed_pi,
    //             ctrl_cfg->speed_kp * (foc->scale.current_to_pu / foc->scale.speed_to_pu),
    //             ctrl_cfg->speed_ki * (foc->scale.current_to_pu / foc->scale.speed_to_pu));

    foc_pi_set_output_limit(&foc->id_pi, -ctrl_cfg->voltage_limit_pu, ctrl_cfg->voltage_limit_pu);
    foc_pi_set_output_limit(&foc->iq_pi, -ctrl_cfg->voltage_limit_pu, ctrl_cfg->voltage_limit_pu);
    // foc_pi_set_output_limit(
    //     &foc->speed_pi, -ctrl_cfg->iq_limit * foc->scale.current_to_pu, ctrl_cfg->iq_limit *
    //     foc->scale.current_to_pu);

    if (!_pmsm_foc_profile_ready(foc)) {
        /* 参数非法时直接进入故障态，避免错误输出 */
        foc->runtime.mode = FOC_MODE_FAULT;
        foc->runtime.fault_mask |= FOC_FAULT_INVALID_PROFILE;
        FOC_LOG_ERROR("pmsm_foc_init profile invalid\r\n");
        return false;
    }

    return true;
}

/**
 * @brief  启动 FOC 控制器
 * @param  foc: 控制器对象
 * @param  mode: 启动模式
 * @return 无
 */
void pmsm_foc_start(pmsm_foc_t *foc, foc_mode_t mode)
{
    if (foc == NULL) {
        FOC_LOG_ERROR("pmsm_foc_start foc null\r\n");
        return;
    }
    if (!_pmsm_foc_profile_ready(foc)) {
        foc->runtime.mode = FOC_MODE_FAULT;
        foc->runtime.fault_mask |= FOC_FAULT_INVALID_PROFILE;
        FOC_LOG_ERROR("pmsm_foc_start profile invalid\r\n");
        _pmsm_foc_disable_output(foc);
        return;
    }

    foc_pi_reset(&foc->id_pi);
    foc_pi_reset(&foc->iq_pi);
    foc_pi_reset(&foc->speed_pi);
    foc->runtime.mode = mode;

    if (foc->hal_ops.set_output_enable != NULL) {
        foc->hal_ops.set_output_enable(foc->hal_user_data, true);
    }
}

/**
 * @brief  停止 FOC 控制器
 * @param  foc: 控制器对象
 * @return 无
 */
void pmsm_foc_stop(pmsm_foc_t *foc)
{
    if (foc == NULL) {
        return;
    }

    foc->runtime.mode = FOC_MODE_STOP;
    _pmsm_foc_disable_output(foc);
}

/**
 * @brief  设置 d-q 电流给定
 * @param  foc: 控制器对象
 * @param  id_ref: d 轴电流给定
 * @param  iq_ref: q 轴电流给定
 * @return 无
 */
void pmsm_foc_set_current_ref(pmsm_foc_t *foc, foc_scalar_t id_ref, foc_scalar_t iq_ref)
{
    if (foc == NULL) {
        return;
    }

    foc->runtime.current_ref_dq.d = id_ref;
    foc->runtime.current_ref_dq.q = iq_ref;
}

/**
 * @brief  设置速度给定
 * @param  foc: 控制器对象
 * @param  speed_ref: 速度参考值
 * @return 无
 */
void pmsm_foc_set_speed_ref(pmsm_foc_t *foc, foc_scalar_t speed_ref)
{
    if (foc == NULL) {
        return;
    }

    foc->runtime.speed_ref = speed_ref;
}

/**
 * @brief  执行一次电流环控制
 * @param  foc: 控制器对象
 * @return true=执行成功, false=执行失败
 */
bool pmsm_foc_current_loop(pmsm_foc_t *foc)
{
    foc_current_loop_sample_t current_sample = { 0 };
    foc_angle_sample_t angle_sample = { 0 };
    foc_abc_t phase_current = { 0 };
    foc_scalar_t bus_voltage_pu = 0.0f;
    foc_ab_t current_ab;

    if ((foc == NULL) || (foc->hal_ops.read_current_loop_sample == NULL)
        || (foc->hal_ops.get_electrical_angle == NULL)) {
        FOC_LOG_ERROR("pmsm_foc_current_loop hal invalid\r\n");
        return false;
    }
    if ((foc->runtime.mode != FOC_MODE_CURRENT) && (foc->runtime.mode != FOC_MODE_SPEED)) {
        FOC_LOG_WARN("pmsm_foc_current_loop mode invalid:%d\r\n", foc->runtime.mode);
        return false;
    }

    /* 电流样本与 PWM 采样同步，角度可来自异步传感器或观测器 */
    foc->hal_ops.read_current_loop_sample(foc->hal_user_data, &current_sample);
    foc->hal_ops.get_electrical_angle(foc->hal_user_data, current_sample.sample_tick_us, &angle_sample);

    /* 电流环主链路：原始电流值 -> 标幺化 -> Clarke/Park -> PI -> 反变换 -> SVPWM */
    /* 1. 计算三相电流的标幺值以及母线电压的标幺值 */
#if (FOC_CURRENT_SENSE_MODE == 1)
// TODO 单电阻采样方案
#elif (FOC_CURRENT_SENSE_MODE == 2)
    phase_current.a = current_sample.a_real / foc->motor_profile->inv_i_base;
    phase_current.b = current_sample.b_real / foc->motor_profile->inv_i_base;
#elif (FOC_CURRENT_SENSE_MODE == 3)
    // TODO 三电阻采样方案
#endif
    foc->runtime.bus_voltage_pu = current_sample.bus_voltage / foc->motor_profile->inv_v_base;
    foc->runtime.current_sample_tick_us = current_sample.sample_tick_us;

    foc->runtime.electrical_angle = angle_sample.electrical_angle;
    foc->runtime.electrical_speed = angle_sample.electrical_speed;
    foc->runtime.angle_sample_tick_us = angle_sample.angle_tick_us;
    foc->runtime.angle_status = angle_sample.status;

    /* 2. 将电流的标幺值转换到dq轴 */
    foc_clarke(&current_ab, &phase_current);
    foc_park(&foc->runtime.current_meas_dq, &current_ab, foc->runtime.electrical_angle);

    foc->runtime.voltage_cmd_dq.d =
        foc_pi_run(&foc->id_pi, foc->runtime.current_ref_dq.d, foc->runtime.current_meas_dq.d);
    foc->runtime.voltage_cmd_dq.q =
        foc_pi_run(&foc->iq_pi, foc->runtime.current_ref_dq.q, foc->runtime.current_meas_dq.q);

    foc_inv_park(&foc->runtime.voltage_cmd_ab, &foc->runtime.voltage_cmd_dq, foc->runtime.electrical_angle);
    foc_svpwm_run(&foc->runtime.pwm_duty, &foc->runtime.voltage_cmd_ab);

    if (foc->hal_ops.write_pwm_duty != NULL) {
        foc->hal_ops.write_pwm_duty(foc->hal_user_data, &foc->runtime.pwm_duty);
    }
    return true;
}

/**
 * @brief  执行一次速度环控制
 * @param  foc: 控制器对象
 * @return 无
 */
void pmsm_foc_speed_loop(pmsm_foc_t *foc)
{
    if (foc == NULL) {
        return;
    }
    if (foc->runtime.mode != FOC_MODE_SPEED) {
        return;
    }

    /* 当前函数只负责速度环，后续可挂状态机、保护和观测器 */
    foc->runtime.current_ref_dq.q = foc_pi_run(&foc->speed_pi, foc->runtime.speed_ref, foc->runtime.speed_feedback);
}
/*---------- end of file ----------*/
