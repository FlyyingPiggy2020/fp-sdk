/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : foc_math.h
 * @Author       : lxf
 * @Date         : 2026-04-02 16:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-04-02 16:20:00
 * @Brief        : FOC 数学接口
 */

#ifndef __FOC_MATH_H__
#define __FOC_MATH_H__

/*---------- includes ----------*/
#include "foc_port.h"
#include "foc_types.h"
/*---------- macro ----------*/
#define FOC_ONE_BY_SQRT3 (0.57735026918962576451f)
#define FOC_SQRT3_BY_2   (0.86602540378443864676f)
/*---------- type define ----------*/

// 因为这里的transform中不需要c相，所以不定义c，c可以通过a和b计算得到
struct foc_abc {
    foc_scalar_t a;
    foc_scalar_t b;
};

struct foc_ab {
    foc_scalar_t alpha;
    foc_scalar_t beta;
};

struct foc_dq {
    foc_scalar_t d;
    foc_scalar_t q;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
foc_angle_t foc_wrap_angle_deg(foc_angle_t angle_deg);
void foc_clarke(struct foc_ab *out, const struct foc_abc *in);
void foc_park(struct foc_dq *out, const struct foc_ab *in, foc_angle_t angle_deg);
void foc_inv_park(struct foc_ab *out, const struct foc_dq *in, foc_angle_t angle_deg);
void foc_svpwm(struct foc_pwm_out *out, const struct foc_ab *voltage_ab);
/*---------- end of file ----------*/
#endif
