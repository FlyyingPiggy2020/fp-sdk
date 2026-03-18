/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_math_types.h
 * @Author       : Codex
 * @Date         : 2026-03-16
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-16
 * @Brief        : FOC 数学公共类型
 */

#ifndef __FOC_MATH_TYPES_H__
#define __FOC_MATH_TYPES_H__

/*---------- includes ----------*/
#include "foc_math_backend.h"
#include <stdint.h>

/*---------- type define ----------*/
typedef struct {
    /* 三相量，常用于相电流或相电压 */
    foc_scalar_t a;
    foc_scalar_t b;
    foc_scalar_t c;
} foc_abc_t;

typedef struct {
    /* 静止坐标系 alpha-beta 分量 */
    foc_scalar_t alpha;
    foc_scalar_t beta;
} foc_ab_t;

typedef struct {
    /* 同步旋转坐标系 d-q 分量 */
    foc_scalar_t d;
    foc_scalar_t q;
} foc_dq_t;

typedef struct {
    /* 归一化后的三相占空比输出 */
    foc_scalar_t duty_a;
    foc_scalar_t duty_b;
    foc_scalar_t duty_c;
    uint8_t sector;
} foc_pwm_duty_t;

#endif
