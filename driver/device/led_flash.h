/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : led.h
 * @Author       : lxf
 * @Date         : 2025-05-20 10:49:59
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-05-20 13:24:13
 * @Brief        : LED驱动程序
 * 1.需要将irq_handler放入一个1ms的时间片中执行
 */

#ifndef __LED_H__
#define __LED_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device.h"
/*---------- macro ----------*/
#define IOCTL_LEDF_START_FLASH  (IOCTL_USER_START + 0x00)
#define IOCTL_LEDF_STOP_FLASH   (IOCTL_USER_START + 0x00)

#define LED_FLASH_FOREVER       0xffff
#define LED_FLASH_DELAY_FOREVER 0xffff
/*---------- type define ----------*/
/**
 * @brief LED闪烁控制逻辑优先级
 * @return {*}
 */
typedef enum led_flash_layer_name {
    LED_FLASH_HIGH_LAYER = 0,
    LED_FLASH_LOW_LAYER
} led_flash_layer_name_t;

typedef struct led_flash_data {
    uint16_t cnt;
    uint16_t dealy;
    uint16_t cnt_reload;
    uint16_t delay_reload;
} led_flash_data_t;

typedef struct led_flash_logic {
    led_flash_data_t layer[2];
} led_flash_logic_t;

typedef struct {
    uint8_t led_num; // led最大个数

    struct {
        bool (*init)(void);
        void (*deinit)(void);
        bool (*led_on)(uint8_t id);
        bool (*led_off)(uint8_t id);
        bool (*led_toggle)(uint8_t id);
    } ops;

    struct {
        led_flash_logic_t *led_flash_list;
    } priv;
} ledf_describe_t;

typedef union {
    struct {
        uint8_t id;// 从0开始。例如led_num为1，则led的id从0开始。
        uint16_t cnt;
        uint16_t delay;
        led_flash_layer_name_t layer;
    } start;

    struct {
        uint8_t id;
        led_flash_layer_name_t layer;
    } stop;
} ledf_api_param_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
