/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : fmath.h
 * @Author       : lxf
 * @Date         : 2025-12-29 16:00:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-12-29 16:00:00
 * @Brief        : 浮点运算抽象层
 *                根据 CONFIG_QFPLIB_ENABLE 自动选择 qfplib 或 标准库
 */

#ifndef __FMATH_H__
#define __FMATH_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdint.h>

#if CONFIG_QFPLIB_ENABLE
#include "qfplib-m0-full.h"
#else
#include <math.h>
#endif

/*---------- macro ----------*/
#define PI 3.1415926f

#if CONFIG_QFPLIB_ENABLE
/* qfplib浮点运算宏 */
#define F_ADD(x, y) qfp_fadd(x, y)
#define F_SUB(x, y) qfp_fsub(x, y)
#define F_MUL(x, y) qfp_fmul(x, y)
#define F_DIV(x, y) qfp_fdiv(x, y)
/* fabsf: 使用qfp_fcmp实现绝对值 */
#define F_ABS(x)    (qfp_fcmp(x, 0.0f) < 0 ? qfp_fsub(0.0f, x) : x)
#define F_LT(x, y)  (qfp_fcmp(x, y) < 0)
#define F_GT(x, y)  (qfp_fcmp(x, y) > 0)
#define F_LE(x, y)  (qfp_fcmp(x, y) <= 0)
#define F_GE(x, y)  (qfp_fcmp(x, y) >= 0)
#define F_EQ(x, y)  (qfp_fcmp(x, y) == 0)
#define F_SQRT(x)   qfp_fsqrt(x)
#define F_SIN(x)    qfp_fsin(x)
#else
/* 标准浮点运算宏 */
#define F_ADD(x, y) ((x) + (y))
#define F_SUB(x, y) ((x) - (y))
#define F_MUL(x, y) ((x) * (y))
#define F_DIV(x, y) ((x) / (y))
#define F_ABS(x)    fabsf(x)
#define F_LT(x, y)  ((x) < (y))
#define F_GT(x, y)  ((x) > (y))
#define F_LE(x, y)  ((x) <= (y))
#define F_GE(x, y)  ((x) >= (y))
#define F_EQ(x, y)  ((x) == (y))
#define F_SQRT(x)   sqrtf(x)
#define F_SIN(x)    sinf(x)
#endif
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
