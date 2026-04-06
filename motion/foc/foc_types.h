/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : foc_types.h
 * @Author       : lxf
 * @Date         : 2026-04-02 16:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-04-06 09:05:36
 * @Brief        : FOC 公共类型定义
 */

#ifndef __FOC_TYPES_H__
#define __FOC_TYPES_H__

/*---------- includes ----------*/
#include "foc_pi.h"
#include <stdbool.h>
#include <stdint.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
typedef float foc_scalar_t;
typedef float foc_angle_t;

enum foc_mode {
    FOC_MODE_STOP = 0,
    FOC_MODE_ALIGN,
    FOC_MODE_CURRENT,
    FOC_MODE_SPEED,
    FOC_MODE_FAULT,
};

enum foc_fault_mask {
    FOC_FAULT_NONE = 0x00000000UL,
    FOC_FAULT_INVALID_CONFIG = 0x00000001UL,
    FOC_FAULT_SAMPLE = 0x00000002UL,
    FOC_FAULT_OUTPUT = 0x00000004UL,
    FOC_FAULT_ALIGN = 0x00000008UL,
};

struct foc_motor_cfg {
    uint8_t pole_pairs;
    int8_t angle_direction;
    foc_scalar_t current_base_a; // 电流基值：安培
    foc_scalar_t voltage_base_v; // 电压基值：伏特
};

struct foc_ctrl_cfg {
    foc_scalar_t id_kp_pu;
    foc_scalar_t id_ki_pu;
    foc_scalar_t id_limit_pu; // id环输出限幅和积分限幅
    foc_scalar_t iq_kp_pu;
    foc_scalar_t iq_ki_pu;
    foc_scalar_t iq_limit_pu; // iq环输出限幅和积分限幅
    foc_scalar_t speed_kp;
    foc_scalar_t speed_ki;
    foc_scalar_t speed_limit_pu; // 速度环输出限幅和积分限幅
    uint32_t current_loop_hz;
    uint32_t speed_loop_hz;
};

struct foc_align_cfg {
    foc_scalar_t voltage_d_pu;
};

struct foc_config {
    struct foc_motor_cfg motor;
    struct foc_ctrl_cfg ctrl;
    struct foc_align_cfg align;
};

struct foc_align_state {
    bool is_started;
    uint64_t start_tick_ms;
    uint64_t sample_tick_ms;
    foc_angle_t msum_deg;
    foc_angle_t mavg_deg;
    uint16_t sample_count;
};

struct foc_state {
    enum foc_mode mode;
    uint32_t fault_mask;

    foc_scalar_t id_ref_pu;
    foc_scalar_t iq_ref_pu;
    foc_scalar_t speed_ref;
    foc_scalar_t speed_feedback;

    foc_angle_t electrical_angle_deg;
    foc_scalar_t electrical_speed_deg_s;
    foc_angle_t electrical_zero_offset_deg;

    struct foc_pi id_pi;
    struct foc_pi iq_pi;
    struct foc_pi speed_pi;
    struct foc_align_state align;
};

struct foc_debug_sample {
    uint32_t tick_us;
    uint16_t flags;

    foc_scalar_t ia;
    foc_scalar_t ib;
    foc_scalar_t ic;
    foc_angle_t mech_angle_deg;
    foc_angle_t electrical_angle_deg;

    foc_scalar_t i_alpha_pu;
    foc_scalar_t i_beta_pu;
    foc_scalar_t id_ref_pu;
    foc_scalar_t iq_ref_pu;
    foc_scalar_t current_d_pu;
    foc_scalar_t current_q_pu;

    foc_scalar_t voltage_d_pu;
    foc_scalar_t voltage_q_pu;
    foc_scalar_t duty_a;
    foc_scalar_t duty_b;
    foc_scalar_t duty_c;

    foc_scalar_t speed_ref;
    foc_scalar_t speed_feedback;
};

struct foc_port;

struct foc_motor {
    const struct foc_config *cfg;
    const struct foc_port *port;
    void *port_ctx; // 端口相关的上下文指针，由用户自定义，FOC 核心不使用该指针，仅在调用 port 函数时原样传递

    struct foc_state state;
    struct foc_debug_sample debug_sample;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#endif
