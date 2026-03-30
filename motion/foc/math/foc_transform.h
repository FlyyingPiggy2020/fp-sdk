/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_transform.h
 * @Author       : Codex
 * @Date         : 2026-03-16
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-17
 * @Brief        : Clarke/Park 变换接口
 */

#ifndef __FOC_TRANSFORM_H__
#define __FOC_TRANSFORM_H__

/*---------- includes ----------*/
#include "foc_math_types.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  三相静止坐标系到 alpha-beta 坐标系变换
 * @param  out: 输出 alpha-beta 分量
 * @param  in: 输入三相量
 * @return 无
 */
void foc_clarke(foc_ab_t *out, const foc_abc_t *in);

/**
 * @brief  alpha-beta 坐标系到三相静止坐标系反变换
 * @param  out: 输出三相量
 * @param  in: 输入 alpha-beta 分量
 * @return 无
 */
void foc_inv_clarke(foc_abc_t *out, const foc_ab_t *in);

/**
 * @brief  alpha-beta 坐标系到 d-q 坐标系变换
 * @param  out: 输出 d-q 分量
 * @param  in: 输入 alpha-beta 分量
 * @param  angle: 电角度
 * @return 无
 */
void foc_park(foc_dq_t *out, const foc_ab_t *in, foc_angle_t angle);

/**
 * @brief  d-q 坐标系到 alpha-beta 坐标系反变换
 * @param  out: 输出 alpha-beta 分量
 * @param  in: 输入 d-q 分量
 * @param  angle: 电角度
 * @return 无
 */
void foc_inv_park(foc_ab_t *out, const foc_dq_t *in, foc_angle_t angle);
/*---------- end of file ----------*/
#endif
