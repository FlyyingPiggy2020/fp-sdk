/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_svpwm.h
 * @Author       : Codex
 * @Date         : 2026-03-16
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-17
 * @Brief        : 归一化 SVPWM 接口
 */

#ifndef __FOC_SVPWM_H__
#define __FOC_SVPWM_H__

/*---------- includes ----------*/
#include "foc_math_types.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  根据 alpha-beta 电压指令计算三相占空比
 * @param  out: 输出三相归一化占空比
 * @param  voltage_ab: 输入 alpha-beta 电压指令
 * @return 无
 */
void foc_svpwm_run(foc_pwm_duty_t *out, const foc_ab_t *voltage_ab);
/*---------- end of file ----------*/
#endif
