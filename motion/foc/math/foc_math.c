/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : foc_math.c
 * @Author       : lxf
 * @Date         : 2026-04-02 16:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-04-02 16:20:00
 * @Brief        : FOC 数学实现
 */

/*---------- includes ----------*/
#include "foc_math.h"
#include <math.h>
#include <stddef.h>
/*---------- macro ----------*/
#define FOC_TWO_PI (6.28318530717958647692f)
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
static foc_scalar_t _foc_clamp_unit(foc_scalar_t value);
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
static foc_scalar_t _foc_clamp_unit(foc_scalar_t value)
{
    if (value < 0.0f) {
        return 0.0f;
    }
    if (value > 1.0f) {
        return 1.0f;
    }

    return value;
}

foc_angle_t foc_wrap_angle_deg(foc_angle_t angle_deg)
{
    while (angle_deg >= 360.0f) {
        angle_deg -= 360.0f;
    }
    while (angle_deg < 0.0f) {
        angle_deg += 360.0f;
    }

    return angle_deg;
}

void foc_clarke(struct foc_ab *out, const struct foc_abc *in)
{
    if ((out == NULL) || (in == NULL)) {
        return;
    }

    out->alpha = in->a;
    out->beta = (in->a + (2.0f * in->b)) * FOC_ONE_BY_SQRT3;
}

void foc_park(struct foc_dq *out, const struct foc_ab *in, foc_angle_t angle_deg)
{
    foc_scalar_t rad = 0.0f;
    foc_scalar_t sin_val = 0.0f;
    foc_scalar_t cos_val = 0.0f;

    if ((out == NULL) || (in == NULL)) {
        return;
    }

    rad = (angle_deg * FOC_TWO_PI) / 360.0f;
    sin_val = sinf(rad);
    cos_val = cosf(rad);

    out->d = (in->alpha * cos_val) + (in->beta * sin_val);
    out->q = (-in->alpha * sin_val) + (in->beta * cos_val);
}

void foc_inv_park(struct foc_ab *out, const struct foc_dq *in, foc_angle_t angle_deg)
{
    foc_scalar_t rad = 0.0f;
    foc_scalar_t sin_val = 0.0f;
    foc_scalar_t cos_val = 0.0f;

    if ((out == NULL) || (in == NULL)) {
        return;
    }

    rad = (angle_deg * FOC_TWO_PI) / 360.0f;
    sin_val = sinf(rad);
    cos_val = cosf(rad);

    out->alpha = (in->d * cos_val) - (in->q * sin_val);
    out->beta = (in->d * sin_val) + (in->q * cos_val);
}

void foc_svpwm(struct foc_pwm_out *out, const struct foc_ab *voltage_ab)
{
    foc_scalar_t x = 0.0f;
    foc_scalar_t y = 0.0f;
    foc_scalar_t z = 0.0f;
    foc_scalar_t t1 = 0.0f;
    foc_scalar_t t2 = 0.0f;
    foc_scalar_t ta = 0.0f;
    foc_scalar_t tb = 0.0f;
    foc_scalar_t tc = 0.0f;
    foc_scalar_t da = 0.0f;
    foc_scalar_t db = 0.0f;
    foc_scalar_t dc = 0.0f;
    foc_scalar_t sumt = 0.0f;
    foc_scalar_t sumt_inv = 0.0f;
    uint8_t sector = 0U;
    uint8_t a = 0U;
    uint8_t b = 0U;
    uint8_t c = 0U;

    if ((out == NULL) || (voltage_ab == NULL)) {
        return;
    }

    x = voltage_ab->beta;
    y = (FOC_SQRT3_BY_2 * voltage_ab->alpha) + (0.5f * voltage_ab->beta);
    z = (-FOC_SQRT3_BY_2 * voltage_ab->alpha) + (0.5f * voltage_ab->beta);

    if (x > 0.0f) {
        a = 1U;
    }
    if (z < 0.0f) {
        b = 1U;
    }
    if (y < 0.0f) {
        c = 1U;
    }
    sector = (uint8_t)((c << 2U) | (b << 1U) | a);

    switch (sector) {
        case 3U:
            t1 = -z;
            t2 = x;
            break;
        case 1U:
            t1 = z;
            t2 = y;
            break;
        case 5U:
            t1 = x;
            t2 = -y;
            break;
        case 4U:
            t1 = -x;
            t2 = z;
            break;
        case 6U:
            t1 = -y;
            t2 = -z;
            break;
        case 2U:
            t1 = y;
            t2 = -x;
            break;
        default:
            t1 = 0.0f;
            t2 = 0.0f;
            break;
    }

    sumt = t1 + t2;
    if (sumt > 1.0f) {
        sumt_inv = 1.0f / sumt;
        t1 *= sumt_inv;
        t2 *= sumt_inv;
    }

    ta = (1.0f - t1 - t2) * 0.25f;
    tb = ta + (0.5f * t1);
    tc = tb + (0.5f * t2);

    da = 1.0f - (2.0f * ta);
    db = 1.0f - (2.0f * tb);
    dc = 1.0f - (2.0f * tc);

    switch (sector) {
        case 3U:
            out->duty_a = da;
            out->duty_b = db;
            out->duty_c = dc;
            break;
        case 1U:
            out->duty_a = db;
            out->duty_b = da;
            out->duty_c = dc;
            break;
        case 5U:
            out->duty_a = dc;
            out->duty_b = da;
            out->duty_c = db;
            break;
        case 4U:
            out->duty_a = dc;
            out->duty_b = db;
            out->duty_c = da;
            break;
        case 6U:
            out->duty_a = db;
            out->duty_b = dc;
            out->duty_c = da;
            break;
        case 2U:
            out->duty_a = da;
            out->duty_b = dc;
            out->duty_c = db;
            break;
        default:
            out->duty_a = 0.5f;
            out->duty_b = 0.5f;
            out->duty_c = 0.5f;
            break;
    }

    out->duty_a = _foc_clamp_unit(out->duty_a);
    out->duty_b = _foc_clamp_unit(out->duty_b);
    out->duty_c = _foc_clamp_unit(out->duty_c);
}
/*---------- end of file ----------*/
