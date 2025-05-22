/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : butter.h
 * @Author       : lxf
 * @Date         : 2024-12-27 08:50:49
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-12-27 11:25:02
 * @Brief        : 一阶巴特沃斯滤波器
 */

#ifndef __BUTTER_H__
#define __BUTTER_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/

#include <stdint.h>
#include <math.h>
/*---------- macro ----------*/
/*---------- type define ----------*/

// 定义滤波器系数结构体
typedef struct {
    int16_t b[2]; // 分子系数 Q14
    int16_t a[2]; // 分母系数 Q14
} FilterCoefficients;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
int16_t low_pass_filter(int16_t input);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
