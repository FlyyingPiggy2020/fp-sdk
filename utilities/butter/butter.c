/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : butter.c
 * @Author       : lxf
 * @Date         : 2024-12-27 08:50:35
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-12-30 10:00:43
 * @Brief        : 一阶巴特沃斯滤波器
 * 用于电流采样滤波
 */

/*---------- includes ----------*/

#include "butter.h"
#include "fmath.h"
/*---------- macro ----------*/

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/

/*---------- function ----------*/

/**
 * @brief 低通滤波器函数，执行时间15us(64Mhz)
 * @param {int16_t} input
 * @return {*}
 */
int16_t low_pass_filter(FilterCoefficients *coeffs, int16_t input)
{
    int32_t output;

    output = (coeffs->b[0] * input + coeffs->b[1] * coeffs->last_input - coeffs->a[1] * coeffs->last_output) >> 14;
    // 滤波器系数已经限幅，不需要额外再去做一次限幅
    coeffs->last_input = input;
    coeffs->last_output = output;
    return output;
}


/**
 * @brief 低通滤波器函数 (浮点数版本)
 * @param coeffs: 滤波器系数结构体指针
 * @param input: 输入值
 * @return 滤波后的输出值
 */
float low_pass_filter_f(FilterCoefficientsFloat *coeffs, float input)
{
    float output;
    float term1, term2, term3;

    term1 = F_MUL(coeffs->b[0], input);
    term2 = F_MUL(coeffs->b[1], coeffs->last_input);
    term3 = F_MUL(coeffs->a[1], coeffs->last_output);
    output = F_SUB(F_ADD(term1, term2), term3);

    coeffs->last_input = input;
    coeffs->last_output = output;
    return output;
}

/*---------- end of file ----------*/
