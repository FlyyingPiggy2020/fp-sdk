/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_zero_calibration.c
 * @Author       : Codex
 * @Date         : 2026-03-31 10:30:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-31 10:13:30
 * @Brief        : 电角度零位标定通用能力实现
 */

/*---------- includes ----------*/
#include "foc_zero_calibration.h"
#include <stddef.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static foc_angle_t _foc_zero_calibration_wrap_angle_deg(foc_angle_t angle_deg);
/*---------- variable ----------*/
/*---------- function ----------*/
static foc_angle_t _foc_zero_calibration_wrap_angle_deg(foc_angle_t angle_deg)
{
    while (angle_deg >= 360.0f) {
        angle_deg -= 360.0f;
    }
    while (angle_deg < 0.0f) {
        angle_deg += 360.0f;
    }

    return angle_deg;
}

bool foc_zero_calibration_apply_alignment_vector(pmsm_foc_t *foc,
                                                 foc_angle_t target_angle_deg,
                                                 foc_scalar_t voltage_d_pu,
                                                 bool enable_output)
{
    foc_dq_t voltage_cmd_dq = { 0 };

    if (foc == NULL) {
        return false;
    }

    voltage_cmd_dq.d = voltage_d_pu;
    voltage_cmd_dq.q = 0.0f;

    return pmsm_foc_apply_voltage_vector(foc, &voltage_cmd_dq, target_angle_deg, enable_output);
}

bool foc_zero_calibration_is_mechanical_sample_valid(const foc_mechanical_angle_sample_t *sample)
{
    if (sample == NULL) {
        return false;
    }

    return (sample->status == FOC_ANGLE_STATUS_VALID) || (sample->status == FOC_ANGLE_STATUS_PREDICTED)
           || (sample->status == FOC_ANGLE_STATUS_ESTIMATED);
}

foc_angle_t
foc_zero_calibration_update_average(foc_angle_t current_avg_deg, uint16_t sample_count, foc_angle_t sample_deg)
{
    foc_scalar_t next_count = 0.0f;

    if (sample_count == UINT16_MAX) {
        return current_avg_deg;
    }

    next_count = (foc_scalar_t)(sample_count + 1U);

    return current_avg_deg + ((sample_deg - current_avg_deg) / next_count);
}

foc_angle_t foc_zero_calibration_calc_electrical_zero_offset(const pmsm_foc_t *foc,
                                                             foc_angle_t mechanical_angle_deg,
                                                             foc_angle_t target_angle_deg)
{
    const sensor_profile_t *sensor_profile = NULL;
    const motor_profile_t *motor_profile = NULL;
    foc_angle_t electrical_angle_without_zero = 0.0f;

    if (foc == NULL) {
        return 0.0f;
    }

    sensor_profile = foc->sensor_profile;
    motor_profile = foc->motor_profile;
    if ((sensor_profile == NULL) || (motor_profile == NULL)) {
        return 0.0f;
    }

    electrical_angle_without_zero =
        mechanical_angle_deg * (foc_scalar_t)sensor_profile->angle_direction * (foc_scalar_t)motor_profile->pole_pairs;

    return _foc_zero_calibration_wrap_angle_deg(target_angle_deg - electrical_angle_without_zero);
}
/*---------- end of file ----------*/
