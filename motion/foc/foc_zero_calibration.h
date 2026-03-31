/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_zero_calibration.h
 * @Author       : Codex
 * @Date         : 2026-03-31 10:30:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-31 10:30:00
 * @Brief        : 电角度零位标定通用能力接口
 */

#ifndef __FOC_ZERO_CALIBRATION_H__
#define __FOC_ZERO_CALIBRATION_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "pmsm_foc.h"
#include <stdbool.h>
#include <stdint.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  输出固定对齐矢量
 * @param  foc: FOC 控制器对象
 * @param  target_angle_deg: 对齐目标电角度
 * @param  voltage_d_pu: d 轴对齐电压标幺值
 * @param  enable_output: true=输出前使能, false=保持当前使能状态
 * @return true=成功, false=失败
 */
bool foc_zero_calibration_apply_alignment_vector(pmsm_foc_t *foc,
                                                 foc_angle_t target_angle_deg,
                                                 foc_scalar_t voltage_d_pu,
                                                 bool enable_output);

/**
 * @brief  判断机械角样本是否可用于零位标定
 * @param  sample: 机械角样本
 * @return true=可用, false=不可用
 */
bool foc_zero_calibration_is_mechanical_sample_valid(const foc_mechanical_angle_sample_t *sample);

/**
 * @brief  更新机械角平均值
 * @param  current_avg_deg: 当前平均值
 * @param  sample_count: 当前已累计样本数
 * @param  sample_deg: 本次样本角度
 * @return 更新后的平均值
 */
foc_angle_t
foc_zero_calibration_update_average(foc_angle_t current_avg_deg, uint16_t sample_count, foc_angle_t sample_deg);

/**
 * @brief  根据机械角计算电角度零位偏移
 * @param  foc: FOC 控制器对象
 * @param  mechanical_angle_deg: 机械角平均值
 * @param  target_angle_deg: 对齐目标电角度
 * @return 归一化后的电角度零位偏移
 */
foc_angle_t foc_zero_calibration_calc_electrical_zero_offset(const pmsm_foc_t *foc,
                                                             foc_angle_t mechanical_angle_deg,
                                                             foc_angle_t target_angle_deg);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __FOC_ZERO_CALIBRATION_H__ */
