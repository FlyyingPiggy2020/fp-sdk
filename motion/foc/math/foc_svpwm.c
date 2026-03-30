/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_svpwm.c
 * @Author       : Codex
 * @Date         : 2026-03-16
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-24 09:30:38
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
 * @brief  根据 alpha-beta 电压指令计算三相占空比
 * @param  out: 输出三相归一化占空比
 * @param  voltage_ab: 输入 alpha-beta 电压指令
 * @return 无
 */
void foc_svpwm_run(foc_pwm_duty_t *out, const foc_ab_t *voltage_ab)
{
    foc_scalar_t x, y, z, k;
    foc_scalar_t t1, t2, ta, tb, tc, da, db, dc, sumt, sumt_inv;
    uint8_t a, b, c;

    if ((out == NULL) || (voltage_ab == NULL)) {
        return;
    }
    k = 1;
    x = k * voltage_ab->beta;
    y = k * FOC_SQRT3_BY_2 * voltage_ab->alpha + 0.5 * voltage_ab->beta;
    z = k * (-FOC_SQRT3_BY_2) * voltage_ab->alpha + 0.5 * voltage_ab->beta;

    a = 0;
    b = 0;
    c = 0;
    if (x > 0) {
        a = 1;
    }
    if (z < 0) {
        b = 1;
    }
    if (y < 0) {
        c = 1;
    }
    out->sector = (c << 2) | (b << 1) | a;
    /* 标幺值系统下，可以抵消掉K */

    t1 = 0;
    t2 = 0;
    switch (out->sector) {
        case 3:
            t1 = -z;
            t2 = x;
            break;
        case 1:
            t1 = z;
            t2 = y;
            break;
        case 5:
            t1 = x;
            t2 = -y;
            break;
        case 4:
            t1 = -x;
            t2 = z;
            break;
        case 6:
            t1 = -y;
            t2 = -z;
            break;
        case 2:
            t1 = y;
            t2 = -x;
            break;
        default:
            break;
    }

    sumt = t1 + t2;
    if (sumt > 1.0f) {
        sumt_inv = 1.0f / sumt;
        t1 = t1 * sumt_inv;
        t2 = t2 * sumt_inv;
    }
    ta = (1.0 - t1 - t2) * 0.25f;
    tb = ta + (0.5 * t1);
    tc = tb + 0.5 * t2;

    /* 这里的计算和发波方式有关 */
    da = 1 - 2 * ta;
    db = 1 - 2 * tb;
    dc = 1 - 2 * tc;

    switch (out->sector) {
        case 3:
            out->duty_a = da;
            out->duty_b = db;
            out->duty_c = dc;
            break;
        case 1:
            out->duty_a = db;
            out->duty_b = da;
            out->duty_c = dc;

            break;
        case 5:
            out->duty_a = dc;
            out->duty_b = da;
            out->duty_c = db;

            break;
        case 4:
            out->duty_a = dc;
            out->duty_b = db;
            out->duty_c = da;

            break;
        case 6:
            out->duty_a = db;
            out->duty_b = dc;
            out->duty_c = da;

            break;
        case 2:
            out->duty_a = da;
            out->duty_b = dc;
            out->duty_c = db;

            break;
        default:
            break;
    }
}
/*---------- end of file ----------*/
