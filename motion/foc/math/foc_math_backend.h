/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_math_backend.h
 * @Author       : Codex
 * @Date         : 2026-03-16
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-16
 * @Brief        : FOC 数学后端抽象
 */

#ifndef __FOC_MATH_BACKEND_H__
#define __FOC_MATH_BACKEND_H__

/*---------- includes ----------*/
#include <math.h>
#include <stdint.h>

/*---------- macro ----------*/
#define FOC_ANGLE_PERIOD      (65536.0f)
#define FOC_TWO_PI            (6.28318530717958647692f)
#define FOC_ONE_BY_SQRT3      (0.57735026918962576451f)
#define FOC_SQRT3_BY_2        (0.86602540378443864676f)

/*---------- type define ----------*/
typedef float foc_scalar_t;
typedef uint16_t foc_angle_t;

typedef struct {
    /* 基于当前后端计算得到的正余弦值 */
    foc_scalar_t sin_val;
    foc_scalar_t cos_val;
} foc_trig_t;

/*---------- function ----------*/
static inline foc_scalar_t foc_scalar_from_float(float value)
{
    return (foc_scalar_t)value;
}

static inline float foc_scalar_to_float(foc_scalar_t value)
{
    return (float)value;
}

static inline foc_angle_t foc_angle_from_turn_fraction(float turn_fraction)
{
    /* 使用一圈归一化角度，便于后续切换到定点实现 */
    while (turn_fraction >= 1.0f) {
        turn_fraction -= 1.0f;
    }
    while (turn_fraction < 0.0f) {
        turn_fraction += 1.0f;
    }
    return (foc_angle_t)(turn_fraction * FOC_ANGLE_PERIOD);
}

static inline float foc_angle_to_radian(foc_angle_t angle)
{
    return ((float)angle * FOC_TWO_PI) / FOC_ANGLE_PERIOD;
}

static inline foc_trig_t foc_trig_from_angle(foc_angle_t angle)
{
    foc_trig_t trig;
    float rad = foc_angle_to_radian(angle);

    /* 第一版直接使用 float32 三角函数，后续可替换为查表或 CORDIC */
    trig.sin_val = sinf(rad);
    trig.cos_val = cosf(rad);
    return trig;
}

#endif
