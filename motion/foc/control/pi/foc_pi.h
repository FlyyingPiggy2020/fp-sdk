/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : foc_pi.h
 * @Author       : lxf
 * @Date         : 2026-04-02 16:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-04-03 13:46:10
 * @Brief        : FOC PI 控制器接口
 */

#ifndef __FOC_PI_H__
#define __FOC_PI_H__

/*---------- includes ----------*/
#include <stdbool.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
struct foc_pi {
    float kp;
    float ki;
    float integral;
    float output;
    float integral_min; // 积分限幅
    float integral_max;
    float output_min; // 输出限幅
    float output_max;
    bool enabled;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
void foc_pi_init(struct foc_pi *pi, float kp, float ki, float limit_abs);
void foc_pi_start(struct foc_pi *pi);
void foc_pi_stop(struct foc_pi *pi, float output);
void foc_pi_set_output_limit(struct foc_pi *pi, float output_min, float output_max);
void foc_pi_set_integral_limit(struct foc_pi *pi, float integral_min, float integral_max);
void foc_pi_run(struct foc_pi *pi, float reference, float feedback);
/*---------- end of file ----------*/
#endif
