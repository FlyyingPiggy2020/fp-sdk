/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : pwmc.h
 * @Author       : lxf
 * @Date         : 2024-12-05 14:37:50
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2025-03-17 09:22:47
 * @Brief        :
 * ATTENTION: THIS DRIVER IS DESIGNED FOR ONE CHANNEL ONLY.
 * _ioctl_set_freq_duty THIS FUNCTION IS DEPEND ON TIM CLK.
 *
 * [-] this driver is control device pwm wave.user should implement follow function:
 *   (1) init, deinit , enable, updatre_duty_raw, get_update_duty_raw.
 *
 * [-] How to use:
 *   args data type is pwmc_ioctl_param.
 *   (1) device_open
 *   (2) Use IOCTL_PWMC_ENABLE_CHANNEL to enable channel.
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
#define IOCTL_PWMC_ENABLE          (IOCTL_USER_START + 0x00)
#define IOCTL_PWMC_DISABLE         (IOCTL_USER_START + 0x01)
#define IOCTL_PWMC_GET_FREQ        (IOCTL_USER_START + 0x02)
#define IOCTL_PWMC_SET_FREQ        (IOCTL_USER_START + 0x03)
#define IOCTL_PWMC_GET_DUTY        (IOCTL_USER_START + 0x05)
#define IOCTL_PWMC_SET_DUTY        (IOCTL_USER_START + 0x06)
#define IOCTL_PWMC_GET_DUTY_RAW    (IOCTL_USER_START + 0x07)
#define IOCTL_PWMC_SET_DUTY_RAW    (IOCTL_USER_START + 0x08)
#define IOCTL_PWMC_SET_IRQ_HANDLER (IOCTL_USER_START + 0x09)
#define IOCTL_PWMC_SET_FREQ_DUTY   (IOCTL_USER_START + 0x0A)
/*---------- type define ----------*/
typedef int32_t (*pwmc_irq_handler_fn)(uint32_t irq_handler, void *args, uint32_t len);

// this struct is describe one channel
typedef struct {
    bool is_enable;     // true:enable; false:disable
    uint32_t clock;     // timer clock. unit:hz
    uint32_t frequence; // timer frequence
    float duty;         // duty per unit
    struct {
        uint32_t prescaler;
        uint32_t arr;
        uint32_t crr;
    } priv;
    struct {
        bool (*init)(void);
        void (*deinit)(void);
        bool (*enable)(bool ctrl);
        /**
         * @brief update frequence by precaler and arr
         * @param {uint32_t} precaler
         * @param {uint32_t} arr
         * @return {*}
         */
        int32_t (*update_frequence_arr)(uint32_t precaler, uint32_t arr);
        /**
         * @brief update timer pwm ccr value. if crr equal to zero or arr, you should let gpio output direct.
         * @param {uint32_t} crr : timer ccr value
         * @return {*}
         */
        int32_t (*update_duty_crr)(uint32_t raw);
        /**
         * @brief get timer ccr value
         * @return {*}
         */
        int32_t (*get_duty_crr)(void);
        /**
         * @brief private date(callback function of irqhandler)
         * @return {*}
         */
        pwmc_irq_handler_fn irq_handler;
    } ops;
} pwmc_describe_t;

struct pwmc_ioctl_param {
    uint32_t freq;
    uint32_t crr; // ccr value.
    float duty;   // duty per unit.取值范围[0,1];
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
