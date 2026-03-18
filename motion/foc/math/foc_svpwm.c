/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_svpwm.c
 * @Author       : Codex
 * @Date         : 2026-03-16
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-17
 * @Brief        : 归一化 SVPWM 实现
 */

/*---------- includes ----------*/
#include "foc_svpwm.h"
#include <stddef.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  将数值限制在 [0, 1] 范围内
 * @param  value: 输入值
 * @return 限幅后的结果
 */
static foc_scalar_t _clamp01(foc_scalar_t value);

/**
 * @brief  根据三相参考电压符号判断当前扇区
 * @param  va: A 相参考值
 * @param  vb: B 相参考值
 * @param  vc: C 相参考值
 * @return 扇区编号，0 表示无效
 */
static uint8_t _sector_from_phase(foc_scalar_t va, foc_scalar_t vb, foc_scalar_t vc);
/*---------- variable ----------*/
/*---------- function ----------*/
static foc_scalar_t _clamp01(foc_scalar_t value)
{
    if (value < 0.0f) {
        return 0.0f;
    }
    if (value > 1.0f) {
        return 1.0f;
    }
    return value;
}

static uint8_t _sector_from_phase(foc_scalar_t va, foc_scalar_t vb, foc_scalar_t vc)
{
    uint8_t code = 0;

    if (va >= 0.0f) {
        code |= 0x01U;
    }
    if (vb >= 0.0f) {
        code |= 0x02U;
    }
    if (vc >= 0.0f) {
        code |= 0x04U;
    }

    switch (code) {
    case 0x03U:
        return 1U;
    case 0x01U:
        return 2U;
    case 0x05U:
        return 3U;
    case 0x04U:
        return 4U;
    case 0x06U:
        return 5U;
    case 0x02U:
        return 6U;
    default:
        return 0U;
    }
}

/**
 * @brief  根据 alpha-beta 电压指令计算三相占空比
 * @param  out: 输出三相归一化占空比
 * @param  voltage_ab: 输入 alpha-beta 电压指令
 * @return 无
 */
void foc_svpwm_run(foc_pwm_duty_t *out, const foc_ab_t *voltage_ab)
{
    foc_scalar_t va;
    foc_scalar_t vb;
    foc_scalar_t vc;
    foc_scalar_t v_max;
    foc_scalar_t v_min;
    foc_scalar_t v_offset;

    if ((out == NULL) || (voltage_ab == NULL)) {
        return;
    }

    /* 先恢复三相参考电压 */
    va = voltage_ab->alpha;
    vb = (-0.5f * voltage_ab->alpha) + (FOC_SQRT3_BY_2 * voltage_ab->beta);
    vc = (-0.5f * voltage_ab->alpha) - (FOC_SQRT3_BY_2 * voltage_ab->beta);

    v_max = va;
    if (vb > v_max) {
        v_max = vb;
    }
    if (vc > v_max) {
        v_max = vc;
    }

    v_min = va;
    if (vb < v_min) {
        v_min = vb;
    }
    if (vc < v_min) {
        v_min = vc;
    }

    /* 注入零序分量后将调制结果压入有效占空比区间 */
    v_offset = 0.5f * (v_max + v_min);
    out->duty_a = _clamp01(0.5f + 0.5f * (va - v_offset));
    out->duty_b = _clamp01(0.5f + 0.5f * (vb - v_offset));
    out->duty_c = _clamp01(0.5f + 0.5f * (vc - v_offset));
    out->sector = _sector_from_phase(va, vb, vc);
}
/*---------- end of file ----------*/
