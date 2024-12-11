/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : paj7620.h
 * @Author       : lxf
 * @Date         : 2024-12-10 11:43:31
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-12-10 13:48:05
 * @Brief        :
 * 默认G_INT低电平使能
 */

#ifndef __PAJ7620__
#define __PAJ7620__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device.h"
/*---------- macro ----------*/

#define IOCTL_PAJ7620_SET_EVT_CB (IOCTL_USER_START + 0x00)

/*---------- type define ----------*/
typedef enum {
    PAJ7620_EVT_NONE,
    PAJ7620_EVT_UP,
    PAJ7620_EVT_DOWN,
    PAJ7620_EVT_LEFT,
    PAJ7620_EVT_RIGHT,
    PAJ7620_EVT_FORWARD,
    PAJ7620_EVT_BACKWARD,
    PAJ7620_EVT_CW,
    PAJ7620_EVT_CCW,
    PAJ7620_EVT_WAVE,
} paj7620_evt_t;

typedef enum {
    PAJ7620_DIR_0,
    PAJ7620_DIR_90,
    PAJ7620_DIR_180,
    PAJ7620_DIR_270,
} paj7620_dir_t;
typedef struct {
    struct {
        bool en;
        uint16_t timer;
    } state;
    struct {
        void (*event)(paj7620_evt_t evt);
    } cb;
    struct {
        bool (*init)(void);
        void (*deinit)(void);
        void (*delay_ms)(uint32_t ms);
        void (*lock)(void *data);
        void (*unlock)(void *data);
    } ops;
    struct {
        uint8_t ee_dev_addr;
        uint8_t dir; // 传感器安装的方向(顺时针)
    } config;
    char *bus_name;
    void *bus;
} paj7620_describe_t;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
