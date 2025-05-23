/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : butter.c
 * @Author       : lxf
 * @Date         : 2024-12-27 08:50:35
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-12-27 11:28:11
 * @Brief        : 一阶巴特沃斯滤波器
 * 用于电流采样滤波
 */

/*---------- includes ----------*/

#include "butter.h"
/*---------- macro ----------*/

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/


/*---------- function ----------*/

/**
 * @brief 低通滤波器函数，执行时间15us
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

/*---------- end of file ----------*/
