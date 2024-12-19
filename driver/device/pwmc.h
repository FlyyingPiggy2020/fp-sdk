/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : pwmc.h
 * @Author       : lxf
 * @Date         : 2024-12-05 14:37:50
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-12-05 14:41:01
 * @Brief        : 抄袭岑老板的代码
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
#define IOCTL_PWMC_ENABLE                (IOCTL_USER_START + 0x00)
#define IOCTL_PWMC_DISABLE               (IOCTL_USER_START + 0x01)
#define IOCTL_PWMC_GET_FREQ              (IOCTL_USER_START + 0x02)
#define IOCTL_PWMC_SET_FREQ              (IOCTL_USER_START + 0x03)
#define IOCTL_PWMC_GET_NUMBER_OF_CHANNEL (IOCTL_USER_START + 0x04)
#define IOCTL_PWMC_GET_DUTY              (IOCTL_USER_START + 0x05)
#define IOCTL_PWMC_SET_DUTY              (IOCTL_USER_START + 0x06)
#define IOCTL_PWMC_GET_DUTY_RAW          (IOCTL_USER_START + 0x07)
#define IOCTL_PWMC_SET_DUTY_RAW          (IOCTL_USER_START + 0x08)
#define IOCTL_PWMC_ENABLE_CHANNEL        (IOCTL_USER_START + 0x09)
#define IOCTL_PWMC_DISABLE_CHANNEL       (IOCTL_USER_START + 0x0A)
#define IOCTL_PWMC_SET_IRQ_HANDLER       (IOCTL_USER_START + 0x0B)
#define IOCTL_PWMC_GET_DUTY_RAW_MAX      (IOCTL_USER_START + 0x0C)
/*---------- type define ----------*/
typedef int32_t (*pwmc_irq_handler_fn)(uint32_t irq_handler, void *args, uint32_t len);

typedef struct {
    uint32_t freq;
    uint32_t number_of_channel;
    uint32_t raw_max;
    struct {
        bool (*init)(void);
        void (*deinit)(void);
        bool (*enable)(bool ctrl);
        bool (*update_duty_raw)(uint32_t channel, uint32_t raw);
        uint32_t (*get_duty_raw)(uint32_t channel);
        bool (*channel_ctrl)(uint32_t channel, bool ctrl);
        pwmc_irq_handler_fn irq_handler;
    } ops;
} pwmc_describe_t;

union pwmc_ioctl_param {
    struct {
        uint32_t channel;
        uint32_t raw;
    } duty_raw;
    struct {
        uint32_t channel;
        float duty; // 取值范围[0,1];
    } duty;
};

struct pwmc_irq_param {
    enum {
        PWMC_IRQ_TYPE_UPDATED,
        PWMC_IRQ_TYPE_CHANNEL
    } type;
    uint32_t channel;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
