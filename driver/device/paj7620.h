/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : paj7620.h
 * @Author       : lxf
 * @Date         : 2024-12-10 11:43:31
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-12-10 13:48:05
 * @Brief        :
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
