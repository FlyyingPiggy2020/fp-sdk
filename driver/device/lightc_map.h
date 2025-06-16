/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : lightc_map.h
 * @Author       : lxf
 * @Date         : 2025-03-18 09:13:30
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-05-27 10:45:23
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
#define IOCTL_LIGHTC_SET_CURRENT_MODE             (IOCTL_USER_START + 0x00)
#define IOCTL_LIGHTC_CMD_OFF                      (IOCTL_USER_START + 0x01)
#define IOCTL_LIGHTC_CMD_ON                       (IOCTL_USER_START + 0x02)
#define IOCTL_LIGHTC_SET_BRIGHTNESS               (IOCTL_USER_START + 0x03)
#define IOCTL_LIGHTC_STEP_BRIGHTNESS_INC          (IOCTL_USER_START + 0x04)
#define IOCTL_LIGHTC_STEP_BRIGHTNESS_DEC          (IOCTL_USER_START + 0x05)
#define IOCTL_LIGHTC_CONTINUE_BRIGHTNESS_INC      (IOCTL_USER_START + 0x06)
#define IOCTL_LIGHTC_CONTINUE_BRIGHTNESS_DEC      (IOCTL_USER_START + 0x07)
#define IOCTL_LIGHTC_LIGHT_ADJUSTMENT_FINISH      (IOCTL_USER_START + 0x08)
#define IOCTL_LIGHTC_LOOP_LIGHT_ADJ_START         (IOCTL_USER_START + 0x09)
#define IOCTL_LIGHTC_LOOP_LIGHT_ADJ_STOP          (IOCTL_USER_START + 0x0A)
#define IOCTL_LIGHTC_REVERSE                      (IOCTL_USER_START + 0x0B) // if light brigness is 50% now, first brightness goto 0%, then goto 50%
#define IOCTL_LIGHTC_SET_BRIGHTNESS_BY_TIME       (IOCTL_USER_START + 0x0C)
// #define IOCTL_LIGHTC_REVERSE_EXT                  (IOCTL_USER_START + 0x0D) // if light brigness is 50% now, first brightness goto 0%, then goto 100%
// #define IOCTL_LIGHTC_REVERSE_BRIGHTNESS           (IOCTL_USER_START + 0x0E) // if light brigness is 50% now, first brightness goto 100%, then goto 0%
#define IOCTL_LIGHTC_START                        (IOCTL_USER_START + 0x0F)
#define IOCTL_LIGHTC_PARAM_READ                   (IOCTL_USER_START + 0x10)
#define IOCTL_LIGHTC_PARAM_WRITE                  (IOCTL_USER_START + 0x11)
#define IOCTL_LIGHTC_LOOP_LIGHT_ADJ_START_BY_TIME (IOCTL_USER_START + 0x12)
#define IOCTL_LIGHTC_GET_BRIGHTNESS               (IOCTL_USER_START + 0x13)
#define IOCTL_LIGHTC_SET_VIRTUAL_BRIGHTNESS       (IOCTL_USER_START + 0x14) // 设置虚拟行程
/*---------- type define ----------*/
typedef enum {
    LIGHTC_MAP_MODE_NORMAL,                 // normal mode
    LIGHTC_MAP_MODE_SET_BRIGHTNESS_BY_TIME, //
    LIGHTC_MAP_MODE_LOOP,                   // loop mode
    LIGHTC_MAP_MODE_LOOP_BY_TIME,
} lightc_mode_e;

typedef enum {
    LIGHTC_MAP_STATUS_STOP,
    LIGHTC_MAP_STATUS_INC,
    LIGHTC_MAP_STATUS_DEC,
} lightc_status_e;

typedef struct {
    double brightness;
    double duty;
} _bmap_node_t;

typedef struct {
    double brightness;
    double duty;
} _iadj_node_t;

typedef struct {
    double brightness;
    double frequence;
} _fmap_node_t;

typedef struct {
    _bmap_node_t *node;
    uint8_t node_size;
} brightness_map_t;

typedef struct {
    _fmap_node_t *node;
    uint8_t node_size;
} frequenct_map_t;

typedef struct {
    _iadj_node_t *node;
    uint8_t node_size;
} iadj_map_t;

typedef struct {
    brightness_map_t *bmap;
    frequenct_map_t *fmap;
    iadj_map_t *imap;
    double brightness;             //[0,100] for example 50 means 50%
    bool is_virtual;               // 虚拟化(虚拟化之后将调用xfer接口，而不是实际的灯)
    int8_t virtual_brightness;     // 虚拟化的灯的亮度(0-100)
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
        double last_brightness_postion;
        double brightness_position;
        uint32_t frequence;
        float iadj;
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
        int32_t (*update_brightness)(uint32_t frequence, float duty, float iadj);
    } ops;
    struct {
        void (*lightc_cmd_off)(void);                          // light off
        void (*lightc_cmd_on)(void);                           // light on
        void (*lightc_cmd_set_brightness)(double brightness);  // set brightness
        void (*lightc_cmd_step_brightness_inc)(void);          // step brightness inc
        void (*lightc_cmd_step_brightness_dec)(void);          // step brightness dec
        void (*lightc_cmd_continue_brightness_inc)(void);      // continue brightness inc
        void (*lightc_cmd_continue_brightness_dec)(void);      // continue brightness dec
        void (*lightc_cmd_continue_brightness_finish)(void);   // continue brightness stop
        void (*lightc_cmd_loop_light_adj_start)(void);         // loop light adjustment start
        void (*lightc_cmd_loop_light_adj_stop)(void);          // loop light adjustment stop
        void (*lightc_cmd_reverse)(void);                      // reverse brightness
                                                               //        void (*lightc_cmd_reverse_ext)(void); // reverse brightness
                                                               //        void (*lightc_cmd_reverse_brightness)(void); // reverse brightness
        void (*lightc_cmd_start)(void);                        // start
        void (*lightc_cmd_param_write)(void);                  // write light param
        void (*lightc_cmd_param_read)(void);                   // read light param
        void (*lightc_cmd_loop_light_adj_start_by_time)(void); // loop light adjustment start by time
    } xfer;
    struct {
        void (*lightc_stop_callback)(void); // when dimming stops,"lightc_stop_callback" will be called.
    } cb;
} lightc_map_describe_t;

union lightc_map_param {
    struct {
        uint8_t brightness;
        uint16_t move_time; // unit: second
    } set;
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
    struct {
        uint8_t brightness;
    } get;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif // !__LIGHTC_MAP_H__
