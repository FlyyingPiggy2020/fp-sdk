/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : lightc_map.h
 * @Author       : lxf
 * @Date         : 2025-03-18 09:13:30
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-10-13 15:56:34
 * @Brief        :
 * this drvier is control lightc brightness by frequence and duty map.
 * you need to provide a map, x-axis is brightness (0->100), y-axis is frequence(120hz->3000hz), z-axis is
 * duty(0.0f->1.0f).
 * [-] How to use:
 *   (1) you need to implement a time slice.
 *   (2) call device_irq_process funciton per time_slice_timebase in time slice.
 *   (3) implement lightc_map_describe_t
 *   (4) when dimming stops,"brightness_stop_callback" will be called.
 * 更新日志：
 *
 * 2025年10月13日 15:40:10     lxf     支持双色温接口
 * 2025年10月17日 14:17:37	   lxf	   支持非易失性存储保存关键参数
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
#define IOCTL_LIGHTC_CMD_OFF                      (IOCTL_USER_START + 0x01) // turn off light
#define IOCTL_LIGHTC_CMD_ON                       (IOCTL_USER_START + 0x02) // turn on light
#define IOCTL_LIGHTC_SET_BRIGHTNESS               (IOCTL_USER_START + 0x03) // set brightness [0,100]
#define IOCTL_LIGHTC_STEP_BRIGHTNESS_INC          (IOCTL_USER_START + 0x04) // step brightness inc (+5% per step)
#define IOCTL_LIGHTC_STEP_BRIGHTNESS_DEC          (IOCTL_USER_START + 0x05) // step brightness dec (-5% per step)
#define IOCTL_LIGHTC_CONTINUE_BRIGHTNESS_INC      (IOCTL_USER_START + 0x06) // continue brightness inc
#define IOCTL_LIGHTC_CONTINUE_BRIGHTNESS_DEC      (IOCTL_USER_START + 0x07) // continue brightness dec
#define IOCTL_LIGHTC_LIGHT_ADJUSTMENT_FINISH      (IOCTL_USER_START + 0x08) // finish continue brightness inc/dec
#define IOCTL_LIGHTC_LOOP_LIGHT_ADJ_START         (IOCTL_USER_START + 0x09) // loop light adjustment start (0->100->0->100...)
#define IOCTL_LIGHTC_LOOP_LIGHT_ADJ_STOP          (IOCTL_USER_START + 0x0A) // loop light adjustment stop (same as IOCTL_LIGHTC_LIGHT_ADJUSTMENT_FINISH)
#define IOCTL_LIGHTC_REVERSE                      (IOCTL_USER_START + 0x0B) // if light brigness is 50% now, first brightness goto 0%, then goto 50%
#define IOCTL_LIGHTC_SET_BRIGHTNESS_BY_TIME       (IOCTL_USER_START + 0x0C)
#define IOCTL_LIGHTC_REVERSE_EXT                  (IOCTL_USER_START + 0x0D) // 1：100%单键开关灯循环<br />3：最暖最冷色温循环 <br />4: 亮度调光反向 <br />5: 色温调光反向
#define IOCTL_LIGHTC_START                        (IOCTL_USER_START + 0x0F)
#define IOCTL_LIGHTC_PARAM_READ                   (IOCTL_USER_START + 0x10)
#define IOCTL_LIGHTC_PARAM_WRITE                  (IOCTL_USER_START + 0x11)
#define IOCTL_LIGHTC_LOOP_LIGHT_ADJ_START_BY_TIME (IOCTL_USER_START + 0x12)
#define IOCTL_LIGHTC_GET_BRIGHTNESS               (IOCTL_USER_START + 0x13)
#define IOCTL_LIGHTC_SET_VIRTUAL_BRIGHTNESS       (IOCTL_USER_START + 0x14)
#define IOCTL_LIGHTC_SET_COLOR                    (IOCTL_USER_START + 0x15)
#define IOCTL_LIGHTC_TIPS_START                   (IOCTL_USER_START + 0x16)
#define IOCTL_LIGHTC_TIPS_STOP                    (IOCTL_USER_START + 0x17)
#define IOCTL_LIGHTC_COLOR_REVERSE                (IOCTL_USER_START + 0x18) // 色温反向
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
} _bmap_node_t; // 亮度-占空比曲线节点

typedef struct {
    double brightness;
    double duty;
} _iadj_node_t; // 电流-占空比曲线节点

typedef struct {
    double brightness;
    double frequence;
} _fmap_node_t; // 亮度-频率曲线节点

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

// 灯光控制支持的参数
typedef struct {
    uint8_t light_type;          // default:0x00; 灯光类型
    uint8_t dimming_start_point; // default:0;    unit:%
    uint8_t dimming_end_point;   // default:100;  unit:%
    uint8_t start_delay;         // default:8;    unit:100ms (the time it taskes for brightness to go form 0% to 1%)
    uint8_t stop_delay;          // default:8;    unit:100ms (the time it taskes for brightness to go form 1% to 0%)
    uint8_t charge_duty;         // default:20;   unit:% (charge crr is [1% brightness crr value * charge duty])
    uint8_t fade_in_time;        // default:8;    unit:second (the time it takes for brightness to go form 1% to 100%)
    uint8_t fade_out_time;       // default:8;    unit:second (the time it takes for brightness to go form 100% to 1%)
    uint8_t fade_cold_time;      // default:8;    unit:second (the time it takes for color temperature 2700k to 4500k)
    uint8_t fade_warm_time;      // default:8;    unit:second (the time it takes for color temperature 4500k to 2700k)
    uint8_t start_state;         // default:0;    unit:0->off 1->on 2->维持上一次亮度
    uint8_t poweron_brightness;  // default:100;
} lightc_map_param_t;

typedef struct {
    brightness_map_t *bmap;
    frequenct_map_t *fmap;
    iadj_map_t *imap;
    int nvs_fd;
    double brightness;             //[0,100] for example 50 means 50%
    double color;                  // color
    bool is_virtual;               // 虚拟化(虚拟化之后将调用xfer接口，而不是实际的灯)
    int8_t virtual_brightness;     // 虚拟化的灯的亮度(0-100)
    uint16_t time_slice_frequence; // the frequence of function(_light_irq_handler) be called. default 100; unit:hz;

    lightc_map_param_t param;
    struct {
        lightc_status_e status;
        lightc_status_e color_status;
        lightc_status_e last_status;
        lightc_mode_e mode;
        lightc_mode_e color_mode;
        uint8_t remeber_brightness; // default:100  device will remeber last brightness.the cmd "light on" means change the brightness to "remeber brightness".
        uint16_t remeber_color;
        uint8_t poweron_brightness;
        double last_brightness_postion;
        double brightness_position;
        double color_postion;
        uint16_t color_min; // color tempure
        uint16_t color_max;
        uint32_t frequence;
        float iadj;
        float duty;
        float color_duty;
        float brightness_actual; //[0, 100]total brightness means the brightness without limit by "dimming_start_point"
                                 // and "dimming_end_point"
        float brightness_step_1_percent_inc; // if brightness below to 1%,brightness change per 10ms.
        float brightness_step_1_percent_dec;
        float brightness_step_1_to_100_inc;
        float brightness_step_1_to_100_dec;
        float color_step_inc;
        float color_step_dec;
        float step_temp; // for IOCTL_LIGHTC_SET_BRIGHTNESS_BY_TIME API
        struct {
            uint8_t last_brightness; // last brightness before tips
            bool is_tips;            // is in tips status
        } tips;
    } priv;

    struct {
        bool is_off;
    } status;
    struct {
        bool (*init)(void);
        void (*deinit)(void);
        int (*nvs_sction_create)(uint8_t *buf, uint32_t len);
        bool (*nvs_save)(int fd, uint32_t ms);
        bool (*nvs_load)(int fd);
        int32_t (*update_brightness)(uint32_t frequence, float duty, float iadj); // 修改亮度
        int32_t (*update_color)(float duty);                                      // 修改色温
    } ops;

    // 虚拟接口
    struct {
        void (*lightc_cmd_off)(void);                         // light off
        void (*lightc_cmd_on)(void);                          // light on
        void (*lightc_cmd_set_brightness)(double brightness); // set brightness
        void (*lightc_cmd_step_brightness_inc)(void);         // step brightness inc
        void (*lightc_cmd_step_brightness_dec)(void);         // step brightness dec
        void (*lightc_cmd_continue_brightness_inc)(void);     // continue brightness inc
        void (*lightc_cmd_continue_brightness_dec)(void);     // continue brightness dec
        void (*lightc_cmd_continue_brightness_finish)(void);  // continue brightness stop
        void (*lightc_cmd_loop_light_adj_start)(void);        // loop light adjustment start
        void (*lightc_cmd_loop_light_adj_stop)(void);         // loop light adjustment stop
        void (*lightc_cmd_reverse)(void);                     // reverse brightness
        void (*lightc_cmd_reverse_ext)(uint8_t cmd);
        void (*lightc_cmd_start)(void);                        // start
        void (*lightc_cmd_param_write)(void);                  // write light param
        void (*lightc_cmd_param_read)(void);                   // read light param
        void (*lightc_cmd_loop_light_adj_start_by_time)(void); // loop light adjustment start by time
        void (*lightc_cmd_set_color)(uint16_t color);
    } xfer;
    struct {
        // void (*lightc_stop_callback)(void); // when dimming stops,"lightc_stop_callback" will be called.
        /**
         * @brief
         * when brightness change stop, this function will be called once.It be used to remeber last
         * brightness due to some hardware not have a power down detection circuit.
         *
         * A more standard approach is storage brightness when device power down.
         */
        void (*brightness_stop_callback)(uint8_t remeber_brightness);
        void (*color_stop_callback)(void);
    } cb;
} lightc_map_describe_t;

union lightc_map_param {
    struct {
        uint8_t brightness;
        uint16_t color;
        uint16_t move_time; // unit: second
        uint8_t resever_cmd;
    } set;

    lightc_map_param_t param;

    struct {
        uint8_t brightness;
        uint8_t remeber_brightness;
    } get;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif // !__LIGHTC_MAP_H__
