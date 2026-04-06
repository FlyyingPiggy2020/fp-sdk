/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : foc_port.h
 * @Author       : lxf
 * @Date         : 2026-04-02 16:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-04-06 09:28:17
 * @Brief        : FOC 板级端口接口
 */

#ifndef __FOC_PORT_H__
#define __FOC_PORT_H__

/*---------- includes ----------*/
#include <stdbool.h>
#include <stdint.h>
#include "foc_types.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
struct foc_fast_sample {
    foc_scalar_t ia;
    foc_scalar_t ib;
    foc_angle_t mech_angle_deg;
};

struct foc_mechanical_sample {
    foc_scalar_t angle_deg;
    foc_scalar_t speed_deg_s;
};

struct foc_pwm_out {
    float duty_a;
    float duty_b;
    float duty_c;
};

struct foc_port {
    bool (*read_fast_sample)(void *ctx, struct foc_fast_sample *sample);
    bool (*read_mech_angle)(void *ctx, struct foc_mechanical_sample *sample);
    uint32_t (*get_tick_ms)(void *ctx);
    bool (*write_pwm)(void *ctx, const struct foc_pwm_out *out);
    bool (*update_current_zero_drift)(void *ctx);
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#endif
