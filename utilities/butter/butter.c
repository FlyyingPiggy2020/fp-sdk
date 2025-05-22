/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : butter.c
 * @Author       : lxf
 * @Date         : 2024-12-27 08:50:35
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-12-27 11:28:11
 * @Brief        : 一阶巴特沃斯滤波器(采样率2.5KHZ，截止频率10HZ)
 * 用于电流采样滤波
 */

/*---------- includes ----------*/

#include "butter.h"
/*---------- macro ----------*/

#define RangeLimit(min, x, max) (((x) >= (max)) ? (max) : (((x) >= (min)) ? (x) : (min)))
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/

/**
 * @brief matlab得到滤波器系数转为Q14定点数
 * @return {*}
 */
FilterCoefficients coeffs = {
    .b = { 203, 203 },     // 分子系数
    .a = { 16384, -15977 } // 分母系数
};
/*---------- function ----------*/

/**
 * @brief 低通滤波器函数，执行时间15us
 * @param {int16_t} input
 * @return {*}
 */
int16_t low_pass_filter(int16_t input)
{
    int32_t output;
    static int16_t last_input = 0;
    static int16_t last_output = 0;

    output = (coeffs.b[0] * input + coeffs.b[1] * last_input - coeffs.a[1] * last_output) >> 14;
    // 滤波器系数已经限幅，不需要额外再去做一次限幅
    last_input = input;
    last_output = output;
    return output;
}

/*---------- end of file ----------*/
