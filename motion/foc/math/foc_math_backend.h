/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_math_backend.h
 * @Author       : Codex
 * @Date         : 2026-03-16
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-23 17:00:20
 * @Brief        : FOC 数学后端抽象
 */

#ifndef __FOC_MATH_BACKEND_H__
#define __FOC_MATH_BACKEND_H__

/*---------- includes ----------*/
#include <math.h>
#include <stdint.h>

/*---------- macro ----------*/
#define FOC_TWO_PI       (6.28318530717958647692f)
#define FOC_ONE_BY_SQRT3 (0.57735026918962576451f)
#define FOC_SQRT3_BY_2   (0.86602540378443864676f)

/*---------- type define ----------*/
typedef float foc_scalar_t;
typedef float foc_angle_t;

typedef struct {
    /* 基于当前后端计算得到的正余弦值 */
    foc_scalar_t sin_val;
    foc_scalar_t cos_val;
} foc_trig_t;

/*---------- function ----------*/
/**
 * @brief 计算角度
 * @param {foc_angle_t} angle ： 单位°
 * @return {*}
 */
static inline foc_trig_t foc_trig_from_angle(foc_angle_t angle)
{
    foc_trig_t trig;
    float rad = ((float)angle * FOC_TWO_PI) / 360.0f;

    /* 第一版直接使用 float32 三角函数，后续可替换为查表或 CORDIC */
    trig.sin_val = sinf(rad);
    trig.cos_val = cosf(rad);
    return trig;
}

#endif
