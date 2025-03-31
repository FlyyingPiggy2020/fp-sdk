/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : lightc_map.h
 * @Author       : lxf
 * @Date         : 2025-03-18 09:13:30
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2025-03-25 15:09:13
 * @Brief        :
 * this drvier is control lightc brightness by frequence and duty map.
 * you need to provide a map, x-axis is brightness (0->100), y-axis is frequence(120hz->3000hz), z-axis is duty(0.0f->1.0f).
 * [-] How to use:
 *   (1) you need to implement a time slice.
 *   (2) call device_irq_process funciton per time_slice_timebase in time slice.
 *   (3) implement lightc_map_describe_t
 *   (4) when dimming stops,"lightc_stop_callback" will be called.
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
#define IOCTL_LIGHTC_LOOP_LIGHT_ADJ_STOP     (IOCTL_USER_START + 0x0A)
#define IOCTL_LIGHTC_REVERSE                 (IOCTL_USER_START + 0x0B) // if light brigness is 50% now, first brightness goto 0%, then goto 50%
#define IOCTL_LIGHTC_SET_BRIGHTNESS_BY_TIME  (IOCTL_USER_START + 0x0C)
#define IOCTL_LIGHTC_REVERSE_EXT             (IOCTL_USER_START + 0x0D) // if light brigness is 50% now, first brightness goto 0%, then goto 100%
#define IOCTL_LIGHTC_REVERSE_BRIGHTNESS      (IOCTL_USER_START + 0x0E) // if light brigness is 50% now, first brightness goto 100%, then goto 0%
#define IOCTL_LIGHTC_START                   (IOCTL_USER_START + 0x0F)
#define IOCTL_LIGHTC_PARAM_READ              (IOCTL_USER_START + 0x10)
#define IOCTL_LIGHTC_PARAM_WRITE             (IOCTL_USER_START + 0x11)
/*---------- type define ----------*/
typedef enum {
    LIGHTC_MAP_MODE_NORMAL,                 // normal mode
    LIGHTC_MAP_MODE_SET_BRIGHTNESS_BY_TIME, //
    LIGHTC_MAP_MODE_LOOP,                   // loop mode
} lightc_mode_e;

typedef enum {
    LIGHTC_MAP_STATUS_STOP,
    LIGHTC_MAP_STATUS_INC,
    LIGHTC_MAP_STATUS_DEC,
} lightc_status_e;

typedef struct {
    double brightness;
    double duty;
} _map_node_t; // 1% to 100%

typedef struct {
    _map_node_t *node;
    uint8_t node_size;
} brightness_map_t;

typedef struct {
    brightness_map_t *map;
    double brightness;             //[0,100] for example 50 means 50%
    uint16_t time_slice_frequence; // default 100; unit:hz;

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
        lightc_status_e status;
        lightc_status_e last_status;
        lightc_mode_e mode;
        uint8_t remeber_brightness; // default:100  device will remeber last brightness.the cmd "light on" means change the brightness to "remeber brightness".
        double brightness_position;
        uint32_t frequence;
        float duty;
        float brightness_actual;             //[0, 100]total brightness means the brightness without limit by "dimming_start_point" and "dimming_end_point"
        float brightness_step_1_percent_inc; // if brightness below to 1%,brightness change per 10ms.
        float brightness_step_1_percent_dec;
        float brightness_step_1_to_100_inc;
        float brightness_step_1_to_100_dec;
        float step_temp; // for IOCTL_LIGHTC_SET_BRIGHTNESS_BY_TIME API
    } priv;

    struct {
        bool is_off;
    } status;
    struct {
        bool (*init)(void);
        void (*deinit)(void);
        int32_t (*update_brightness)(uint32_t frequence, float duty);
    } ops;
    struct {
        void (*lightc_stop_callback)(void); // when dimming stops,"lightc_stop_callback" will be called.
    } cb;
} lightc_map_describe_t;

struct lightc_map_param {
    uint8_t brightness;
    uint16_t move_time; // unit: second
    struct {
        uint8_t start;
        uint8_t size;
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
    } param;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif // !__LIGHTC_MAP_H__
