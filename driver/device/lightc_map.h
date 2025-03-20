/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : lightc_map.h
 * @Author       : lxf
 * @Date         : 2025-03-18 09:13:30
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2025-03-18 09:21:33
 * @Brief        :
 * this drvier is control lightc brightness by frequence and duty map.
 * you need to provide a map, x-axis is brightness (0->100), y-axis is frequence(120hz->3000hz), z-axis is duty(0.0f->1.0f).
 * [-] How to use:
 *   (1) you need to implement a 10ms time slice.
 *   (2) call device_irq_process funciton per 10 ms in time slice.
 *   (3) implement lightc_map_describe_t
 */
#ifndef __LIGHTC_MAP_H__
#define __LIGHTC_MAP_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device.h"
/*---------- macro ----------*/
#define IOCTL_LIGHTC_SET_CURRENT_MODE        (IOCTL_USER_START + 0x00)
#define IOCTL_LIGHTC_CMD_OFF                 (IOCTL_USER_START + 0x01)
#define IOCTL_LIGHTC_CMD_ON                  (IOCTL_USER_START + 0x02)
#define IOCTL_LIGHTC_SET_BRIGHTNESS          (IOCTL_USER_START + 0x03)
#define IOCTL_LIGHTC_STEP_BRIGHTNESS_INC     (IOCTL_USER_START + 0x04)
#define IOCTL_LIGHTC_SETP_BRIGHTNESS_DEC     (IOCTL_USER_START + 0x05)
#define IOCTL_LIGHTC_CONTINUE_BRIGHTNESS_INC (IOCTL_USER_START + 0x06)
#define IOCTL_LIGHTC_CONTINUE_BRIGHTNESS_DEC (IOCTL_USER_START + 0x07)
#define IOCTL_LIGHTC_LIGHT_ADJUSTMENT_FINISH (IOCTL_USER_START + 0x08)
#define IOCTL_LIGHTC_LOOP_LIGHT_ADJ_START    (IOCTL_USER_START + 0x09)
#define IOCTL_LIGHTC_LOOP_LIGHT_ADJ_STOP     (IOCTL_USER_START + 0x10)
#define IOCTL_LIGHTC_REVERSE                 (IOCTL_USER_START + 0x11)
#define IOCTL_LIGHTC_SET_BRIGHTNESS_BY_TIME  (IOCTL_USER_START + 0x12)
#define IOCTL_LIGHTC_REVERSE_EXT             (IOCTL_USER_START + 0x13)
#define IOCTL_LIGHTC_REVERSE_BRIGHTNESS      (IOCTL_USER_START + 0x14)
/*---------- type define ----------*/
typedef struct {
    double brightness;
    uint32_t frequence;
    double duty;
} _map_node_t; // 1% to 100%

typedef struct {
    _map_node_t *node;
    uint8_t node_size;
} brightness_map_t;

typedef struct {
    brightness_map_t *map;
    double brightness; //[0,100] for example 50 means 50%
    struct {
        uint8_t light_type;          // default:0xff; reserve
        uint8_t dimming_start_point; // default:0;    unit:%
        uint8_t dimming_end_point;   // default:100;  unit:%
        uint8_t cut_start_point;     // default:0;    unit:%
        uint8_t cut_end_pint;        // default:100;  unit:%
        uint8_t start_delay;         // default:8;    unit:100ms (the time it taskes for brightness to go form 0% to 1%)
        uint8_t stop_delay;          // default:8;    unit:100ms (the time it taskes for brightness to go form 1% to 0%)
        uint8_t charge_duty;         // default:20;   unit:% (charge crr is [1% brightness crr value * charge duty])
        uint8_t fade_in_time;        // default:8;    unit:second (the time it taskes for brightness to go form 1% to 100%)
        uint8_t fade_out_time;       // default:8;    unit:second (the time it taskes for brightness to go form 100% to 1%)
        uint8_t start_state;         // default:0;    unit:0->off 1->on
    } param;

    struct {
        double brightness_position;
        uint32_t frequence;
        float duty;
        float brightness_actual;             //[0, 100]total brightness means the brightness without limit by "dimming_start_point" and "dimming_end_point"
        float brightness_step_1_percent_inc; // if brightness below to 1%,brightness change per 10ms.
        float brightness_step_1_percent_dec;
        float brightness_step_1_to_100_inc;
        float brightness_step_1_to_100_dec;

        uint16_t time_count;
    } priv;
    struct {
        bool (*init)(void);
        void (*deinit)(void);
        int32_t (*update_brightness)(uint32_t frequence, float duty);
    } ops;
} lightc_map_describe_t;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif // !__LIGHTC_MAP_H__
