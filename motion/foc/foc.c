/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : foc.c
 * @Author       : lxf
 * @Date         : 2026-04-02 16:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-04-06 09:37:24
 * @Brief        : 扁平 FOC 核心实现
 */

/*---------- includes ----------*/
#include "foc.h"
#include "foc_pi.h"
#include "foc_types.h"
#include "stdbool.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
struct foc_frame {
    struct foc_fast_sample sample;
    struct foc_abc read_abc_pu; //(读取到的)abc三相电流标幺值
    struct foc_ab read_ab_pu;   //(读取到的)α-β电流标幺值
    struct foc_dq read_dq_pu;   //(读取到的)d-q电流标幺值
    struct foc_dq voltage_dq_pu;
    struct foc_ab voltage_ab_pu;
    struct foc_pwm_out pwm;
    foc_angle_t electrical_angle_deg;
    foc_scalar_t electrical_speed_deg_s;
};
/*---------- variable prototype ----------*/
static bool _foc_config_valid(const struct foc_config *config, const struct foc_port *port);
static foc_scalar_t _foc_clamp(float value, float min_value, float max_value);
static foc_angle_t _foc_mech_to_electrical_deg(const struct foc_motor *motor, foc_angle_t mechanical_angle_deg);
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
static bool _foc_config_valid(const struct foc_config *config, const struct foc_port *port)
{
    if ((config == NULL) || (port == NULL)) {
        return false;
    }
    if ((port->read_fast_sample == NULL) || (port->get_tick_ms == NULL) || (port->write_pwm == NULL)) {
        return false;
    }
    if (config->motor.pole_pairs == 0U) {
        return false;
    }
    if ((config->motor.angle_direction != 1) && (config->motor.angle_direction != -1)) {
        return false;
    }
    if ((config->motor.current_base_a <= 0.0f) || (config->motor.voltage_base_v <= 0.0f)) {
        return false;
    }

    return true;
}

static foc_scalar_t _foc_clamp(float value, float min_value, float max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }

    return value;
}

static foc_angle_t _foc_mech_to_electrical_deg(const struct foc_motor *motor, foc_angle_t mechanical_angle_deg)
{
    foc_angle_t electrical_angle_deg = 0.0f;
    uint8_t pole_pairs = motor->cfg->motor.pole_pairs;
    int8_t angle_direction = motor->cfg->motor.angle_direction;
    if ((motor == NULL) || (motor->cfg == NULL)) {
        return 0.0f;
    }

    electrical_angle_deg = mechanical_angle_deg * pole_pairs * angle_direction;

    return foc_wrap_angle_deg(electrical_angle_deg);
}

bool foc_init(struct foc_motor *motor, const struct foc_config *config, const struct foc_port *port, void *port_ctx)
{
    if ((motor == NULL) || !_foc_config_valid(config, port)) {
        if (motor != NULL) {
            memset(motor, 0, sizeof(*motor));
            motor->state.mode = FOC_MODE_FAULT;
            motor->state.fault_mask = FOC_FAULT_INVALID_CONFIG;
        }
        return false;
    }

    memset(motor, 0, sizeof(*motor));
    motor->cfg = config;
    motor->port = port;
    motor->port_ctx = port_ctx;
    motor->state.mode = FOC_MODE_STOP;

    foc_pi_init(&motor->state.id_pi, config->ctrl.id_kp_pu, config->ctrl.id_ki_pu, config->ctrl.id_limit_pu);
    foc_pi_init(&motor->state.iq_pi, config->ctrl.iq_kp_pu, config->ctrl.iq_ki_pu, config->ctrl.iq_limit_pu);
    foc_pi_init(&motor->state.speed_pi, config->ctrl.speed_kp, config->ctrl.speed_ki, config->ctrl.speed_limit_pu);
    memset(&motor->debug_sample, 0, sizeof(motor->debug_sample));
    return true;
}

/**
 * @brief 让电机进入高阻态，可以滑行
 * @param {foc_motor} *motor
 * @return {*}
 */
void foc_command_disable(struct foc_motor *motor)
{
    if (motor == NULL || motor->port == NULL || motor->port->write_pwm == NULL) {
        return;
    }

    motor->state.mode = FOC_MODE_STOP;
    /* 停止电流环、速度环 */
    foc_pi_stop(&motor->state.id_pi, 0);
    foc_pi_stop(&motor->state.iq_pi, 0);
    foc_pi_stop(&motor->state.speed_pi, 0);

    /* TODO 这里实际上应该实现高阻态滑行，但是笔者板卡是IR2103这个芯片的驱动电路和正常的有点不一样
     * 这里应该用一个回调函数兼容不同硬件的
     */
    struct foc_pwm_out pwm = {
        .duty_a = 0.5f,
        .duty_b = 0.5f,
        .duty_c = 0.5f,
    };
    motor->port->write_pwm(motor->port_ctx, &pwm);
}

/**
 * @brief 设置电流环参考值
 * @param {foc_motor} *motor
 * @param {foc_scalar_t} id_pu
 * @param {foc_scalar_t} iq_pu
 * @return {*}
 */
void foc_command_current(struct foc_motor *motor, foc_scalar_t id_pu, foc_scalar_t iq_pu)
{
    if (motor == NULL || motor->port == NULL) {
        return;
    }

    if (motor->state.mode == FOC_MODE_FAULT) {
        return;
    }

    motor->state.mode = FOC_MODE_CURRENT;
    motor->state.id_ref_pu = id_pu;
    motor->state.iq_ref_pu = iq_pu;
    foc_pi_start(&motor->state.id_pi);
    foc_pi_start(&motor->state.iq_pi);
}

/**
 * @brief 设置速度环参考值
 * @param {foc_motor} *motor
 * @param {foc_scalar_t} speed_ref
 * @return {*}
 */
void foc_command_speed(struct foc_motor *motor, foc_scalar_t speed_ref)
{
    foc_scalar_t speed_limit = 0.0f;

    if (motor == NULL || motor->port == NULL) {
        return;
    }

    if (motor->state.mode == FOC_MODE_FAULT) {
        return;
    }
    motor->state.mode = FOC_MODE_CURRENT;
    motor->state.speed_ref = speed_ref;
    foc_pi_start(&motor->state.speed_pi);
}

/**
 * @brief 电机磁编零位校准
 * @param {foc_motor} *motor
 * @return {*}
 */
void foc_command_align(struct foc_motor *motor)
{
    if ((motor == NULL) || (motor->cfg == NULL)) {
        return;
    }
    if (motor->state.mode == FOC_MODE_FAULT) {
        return;
    }

    motor->state.mode = FOC_MODE_ALIGN;
    memset(&motor->state.align, 0, sizeof(motor->state.align));
    foc_pi_start(&motor->state.id_pi);
    foc_pi_start(&motor->state.iq_pi);
}

/**
 * @brief
 * @param {foc_motor} *motor
 * @return {*}
 */
bool foc_run_fast(struct foc_motor *motor)
{
    struct foc_frame frame = { 0 };
    struct foc_debug_sample *debug_sample = NULL;
    struct foc_align_state *align = NULL;
    foc_angle_t mechanical_angle_deg = 0.0f;
    foc_scalar_t voltage_limit = 0.0f;
    bool run_result = true;
    uint64_t now_ms = 0;

    if ((motor == NULL) || (motor->cfg == NULL) || (motor->port == NULL)) {
        return false;
    }

    if ((motor->port->read_fast_sample == NULL) || (motor->port->write_pwm == NULL)) {
        return false;
    }

    /* 停止状态下，进行电流零飘校准 */
    if ((motor->state.mode == FOC_MODE_STOP) && (motor->state.fault_mask == 0U)
        && (motor->port->update_current_zero_drift != NULL)) {
        (void)motor->port->update_current_zero_drift(motor->port_ctx);
    }

    /* 错误状态下，直接停止输出 */
    if (motor->state.mode == FOC_MODE_FAULT) {
        // TODO 电机保护
        return false;
    }

    /* 读取电流、角度的真实值 */
    if (!motor->port->read_fast_sample(motor->port_ctx, &frame.sample)) {
        // TODO 读取参数失败
        return false;
    }

    /* 机械角转电角度 */
    mechanical_angle_deg = frame.sample.mech_angle_deg - motor->state.electrical_zero_offset_deg;
    frame.electrical_angle_deg = _foc_mech_to_electrical_deg(motor, mechanical_angle_deg);
    /* AB相电流转标幺值 */
    frame.read_abc_pu.a = frame.sample.ia / motor->cfg->motor.current_base_a;
    frame.read_abc_pu.b = frame.sample.ib / motor->cfg->motor.current_base_a;
    /* 将AB相电流转换到DQ坐标系 */
    foc_clarke(&frame.read_ab_pu, &frame.read_abc_pu);
    foc_park(&frame.read_dq_pu, &frame.read_ab_pu, frame.electrical_angle_deg);
    motor->state.electrical_angle_deg = frame.electrical_angle_deg;
    align = &motor->state.align;

    switch (motor->state.mode) {
        case FOC_MODE_ALIGN:
            now_ms = motor->port->get_tick_ms(motor->port_ctx);
            if (motor->state.align.is_started == false) {
                motor->state.align.is_started = true;
                align->start_tick_ms = now_ms;
            }
            /* 锁轴 */
            frame.voltage_dq_pu.d = motor->cfg->align.voltage_d_pu;
            frame.voltage_dq_pu.q = 0.0f;
            frame.electrical_angle_deg = 0;
            foc_inv_park(&frame.voltage_ab_pu, &frame.voltage_dq_pu, frame.electrical_angle_deg);

            if ((now_ms - align->start_tick_ms) < 200) {
                motor->state.align.sample_tick_ms = now_ms;
                break;
            }

            if (now_ms - motor->state.align.sample_tick_ms > 10) {
                motor->state.align.sample_tick_ms = now_ms;
                align->sample_count++;
                align->msum_deg += frame.sample.mech_angle_deg;
            }

            if (align->sample_count == 8) {
                align->mavg_deg = align->msum_deg / 8;
                motor->state.electrical_zero_offset_deg = align->mavg_deg;
                foc_command_disable(motor);
            }
            break;
        case FOC_MODE_STOP:
        case FOC_MODE_CURRENT:
        case FOC_MODE_SPEED:
            /* 输入PI控制器 */
            foc_pi_run(&motor->state.id_pi, motor->state.id_ref_pu, frame.read_dq_pu.d);
            foc_pi_run(&motor->state.iq_pi, motor->state.iq_ref_pu, frame.read_dq_pu.q);
            frame.voltage_dq_pu.d = motor->state.id_pi.output;
            frame.voltage_dq_pu.q = motor->state.iq_pi.output;
            foc_inv_park(&frame.voltage_ab_pu, &frame.voltage_dq_pu, frame.electrical_angle_deg);
            break;
        case FOC_MODE_FAULT:
            break;
        default:
            break;
    }

    foc_svpwm(&frame.pwm, &frame.voltage_ab_pu);
    motor->port->write_pwm(motor->port_ctx, &frame.pwm);

    debug_sample = &motor->debug_sample;
    memset(debug_sample, 0, sizeof(*debug_sample));
    debug_sample->flags = (uint16_t)motor->state.mode;
    debug_sample->ia = frame.sample.ia;
    debug_sample->ib = frame.sample.ib;
    debug_sample->ic = -(frame.sample.ia + frame.sample.ib);
    debug_sample->mech_angle_deg = frame.sample.mech_angle_deg;
    debug_sample->electrical_angle_deg = motor->state.electrical_angle_deg;
    debug_sample->i_alpha_pu = frame.read_ab_pu.alpha;
    debug_sample->i_beta_pu = frame.read_ab_pu.beta;
    debug_sample->id_ref_pu = motor->state.id_ref_pu;
    debug_sample->iq_ref_pu = motor->state.iq_ref_pu;
    debug_sample->current_d_pu = frame.read_dq_pu.d;
    debug_sample->current_q_pu = frame.read_dq_pu.q;
    debug_sample->voltage_d_pu = frame.voltage_dq_pu.d;
    debug_sample->voltage_q_pu = frame.voltage_dq_pu.q;
    debug_sample->duty_a = frame.pwm.duty_a;
    debug_sample->duty_b = frame.pwm.duty_b;
    debug_sample->duty_c = frame.pwm.duty_c;
    debug_sample->speed_ref = motor->state.speed_ref;
    debug_sample->speed_feedback = motor->state.speed_feedback;

    return true;
}

void foc_run_slow(struct foc_motor *motor, uint32_t now_ms)
{
    (void)motor;
    (void)now_ms;
}
/*---------- end of file ----------*/
