/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : hall.h
 * @Author       : lxf
 * @Date         : 2025-05-21 13:31:51
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-05-22 11:20:59
 * @Brief        : 
 */

#ifndef __FP_HALL_H__
#define __FP_HALL_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device.h"
/*---------- macro ----------*/
#define MDIRECTION_INC 0        // 行程增加
#define MDIRECTION_DEC 1        // 行程减少
/*---------- type define ----------*/
typedef struct {
    int32_t route;
    float speed;
    struct {
        bool (*init)(void);
        void (*deinit)(void);
        bool (*hall1_read)(void); // 读取霍尔1的电平;高电平true，低电平false
        bool (*hall2_read)(void); // 读取霍尔2的电平;
    } ops;

    struct {
        uint8_t dir; //方向
        uint16_t hall_state_last; // 上次的霍尔状态
        uint16_t hall_state; // 霍尔状态
        uint16_t time_200us; // 计时
        uint16_t pluse_width; // 脉宽
        uint16_t no_pluse_time;
        uint16_t p1_t0; // 用于测量脉宽
        uint16_t p1_t1;
        uint16_t p2_t0;
        uint16_t p2_t1;
    } priv;
} hall_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif // !__FP_HALL_H__


