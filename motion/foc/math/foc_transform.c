/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_transform.c
 * @Author       : Codex
 * @Date         : 2026-03-16
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-17
 * @Brief        : Clarke/Park 变换实现
 */

/*---------- includes ----------*/
#include "foc_transform.h"
#include <stddef.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
/**
 * @brief  三相静止坐标系到 alpha-beta 坐标系变换
 * @param  out: 输出 alpha-beta 分量
 * @param  in: 输入三相量
 * @return 无
 */
void foc_clarke(foc_ab_t *out, const foc_abc_t *in)
{
    if ((out == NULL) || (in == NULL)) {
        return;
    }

    /* 当前默认按三相和为零场景处理 */
    out->alpha = in->a;
    out->beta = (in->a + (2.0f * in->b)) * FOC_ONE_BY_SQRT3;
}

/**
 * @brief  alpha-beta 坐标系到三相静止坐标系反变换
 * @param  out: 输出三相量
 * @param  in: 输入 alpha-beta 分量
 * @return 无
 */
void foc_inv_clarke(foc_abc_t *out, const foc_ab_t *in)
{
    if ((out == NULL) || (in == NULL)) {
        return;
    }

    out->a = in->alpha;
    out->b = (-0.5f * in->alpha) + (FOC_SQRT3_BY_2 * in->beta);
    out->c = (-0.5f * in->alpha) - (FOC_SQRT3_BY_2 * in->beta);
}

/**
 * @brief  alpha-beta 坐标系到 d-q 坐标系变换
 * @param  out: 输出 d-q 分量
 * @param  in: 输入 alpha-beta 分量
 * @param  angle: 电角度
 * @return 无
 */
void foc_park(foc_dq_t *out, const foc_ab_t *in, foc_angle_t angle)
{
    foc_trig_t trig;

    if ((out == NULL) || (in == NULL)) {
        return;
    }

    trig = foc_trig_from_angle(angle);
    out->d = (in->alpha * trig.cos_val) + (in->beta * trig.sin_val);
    out->q = (-in->alpha * trig.sin_val) + (in->beta * trig.cos_val);
}

/**
 * @brief  d-q 坐标系到 alpha-beta 坐标系反变换
 * @param  out: 输出 alpha-beta 分量
 * @param  in: 输入 d-q 分量
 * @param  angle: 电角度
 * @return 无
 */
void foc_inv_park(foc_ab_t *out, const foc_dq_t *in, foc_angle_t angle)
{
    foc_trig_t trig;

    if ((out == NULL) || (in == NULL)) {
        return;
    }

    trig = foc_trig_from_angle(angle);
    out->alpha = (in->d * trig.cos_val) - (in->q * trig.sin_val);
    out->beta = (in->d * trig.sin_val) + (in->q * trig.cos_val);
}
/*---------- end of file ----------*/
