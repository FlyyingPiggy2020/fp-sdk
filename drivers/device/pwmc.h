/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : pwmc.h
 * @Author       : lxf
 * @Date         : 2024-12-05 14:37:50
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-18 14:08:57
 * @Brief        : pwm设备抽象,通道从0开始
 *
 * 2025年3月17日 lxf 初版
 * 2025年9月22日 lxf 支持多通道
 * 2026年3月18日 lxf 增加注释，针对Freq自动生成ARR和Prescaler的功能进行优化，增加手动设置频率的选项。
 */

#ifndef __PWMC_H__
#define __PWMC_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device.h"

/*---------- macro ----------*/
#define PWMC_CHANNEL1             0
#define PWMC_CHANNEL2             1
#define PWMC_CHANNEL3             2
#define PWMC_CHANNEL4             3
#define PWMC_CHANNEL5             4
#define PWMC_CHANNEL6             5
#define IOCTL_CONFIG_PWMC_CHANNEL 6 // channel max

#define IOCTL_PWMC_ENABLE         (IOCTL_USER_START + 0x00)
#define IOCTL_PWMC_DISABLE        (IOCTL_USER_START + 0x01)
#define IOCTL_PWMC_GET_FREQ       (IOCTL_USER_START + 0x02)
#define IOCTL_PWMC_SET_FREQ       (IOCTL_USER_START + 0x03) // (当手动配置Freq时，该函数无效)
#define IOCTL_PWMC_GET_DUTY       (IOCTL_USER_START + 0x05)
#define IOCTL_PWMC_SET_DUTY       (IOCTL_USER_START + 0x06)
#define IOCTL_PWMC_GET_DUTY_RAW   (IOCTL_USER_START + 0x07)
#define IOCTL_PWMC_SET_DUTY_RAW   (IOCTL_USER_START + 0x08)
#define IOCTL_PWMC_SET_FREQ_DUTY  (IOCTL_USER_START + 0x09) // 设置FREQ和DUTY(当手动配置Freq时，该函数只设置DUTY)
/*---------- type define ----------*/
typedef int32_t (*pwmc_irq_handler_fn)(uint32_t irq_handler, void *args, uint32_t len);

// this struct is describe one channel
typedef struct {
    bool is_enable;     // 配合ST等系列芯片的使能函数使用
    uint32_t clock;     // 定时器的时钟频率，单位Hz（当手动配置freq时，该参数无效）
    uint32_t frequence; // 该定时器的频率，单位Hz（当手动配置freq时，该参数无效）

    bool is_manual_freq; // 是否手动设置频率，如果为true，则不自动计算arr和prescaler，用户需要自己设置。

    struct {
        uint32_t prescaler; // 预分频器的值，实际值为该值+1
        uint32_t arr;       // 自动重装载寄存器的值，实际值为该值+1
        struct {
            bool used; // 该通道是否被使用
            float duty;
            uint32_t crr;
        } channel[IOCTL_CONFIG_PWMC_CHANNEL];
    } priv;

    struct {
        bool (*init)(void);
        void (*deinit)(void);
        bool (*enable)(bool ctrl);
        int32_t (*update_precaler_arr)(uint32_t precaler, uint32_t arr);
        int32_t (*update_crr)(uint8_t channel, uint32_t crr);
        pwmc_irq_handler_fn irq_handler;
    } ops;
} pwmc_describe_t;

union pwmc_ioctl_param {
    struct {
        uint8_t channel;
        uint32_t freq;
        uint32_t crr;
        float duty; // duty per unit.取值范围[0,1];
    } set;
    struct {
        uint8_t channel;
        uint32_t freq;
        uint32_t crr;
        float duty; // duty per unit.取值范围[0,1];
    } get;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
