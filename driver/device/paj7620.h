/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : paj7620.h
 * @Author       : lxf
 * @Date         : 2024-12-10 11:43:31
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2025-03-11 17:40:10
 * @Brief        :
 * [-] paj7620 is a gesure sensor developed by PixArt.
 * the datasheet of paj7620 is :https://files.seeedstudio.com/wiki/Grove_Gesture_V_1.0/res/PAJ7620U2_Datasheet_V0.8_20140611.pdf
 *
 * ATTENTTION:G_INT default is low level active.
 *
 * this driver enables real-time detection for 9 gestures: up, down, left, right,
 * forward,backward,clockwise,counter-clockwise and wave. It utilizes i2c communication
 * (defalut address: 0x73).
 *
 * [-] How to use:
 *   (++)1.register i2c bus name, init, delay_ms api in paj7620_describe_t.then use marco DEVICE_DEFINE.
 *   for example:
 *   static paj7620_describe_t paj_ops = {
 *      .bus_name = "i2c2",
 *      .ops = {
 *          .init = _init,
 *         .delay_ms = bsp_DelayMS,
 *     },
 *     .config = {
 *         .ee_dev_addr = 0x73,
 *         .dir = PAJ7620_DIR_0,
 *     },
 *   };
 *   DEVICE_DEFINED(paj_name, paj7620, &paj_ops);
 *
 *   (++)2. register event callback.
 *   for example:
 *   device_t *dev_gesture = devcie_open("paj_name");
 *   device_ioctl(dev_gesture, IOCTL_PAJ7620_SET_EVT_CB, _call_back);
 *
 *   (++)3. implement your logic in event callback.
 *   static void _call_back(paj7620_evt_t evt)
 *   {
 *       switch(evt) {
 *           case PAJ7620_EVT_UP:
 *           break;
 *           ....
 *       }
 *   }
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
