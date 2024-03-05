/*
 * Copyright (c) 2024 by Moorgen Tech. Co, Ltd.
 * @FilePath     : pin.h
 * @Author       : lxf
 * @Date         : 2024-02-22 18:30:24
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-02-23 11:50:50
 * @Brief        : 引脚驱动
 */

#ifndef __PIN_H__
#define __PIN_H__
/*---------- includes ----------*/
#define LOG_TAG "Pin"

#include "fp_sdk.h"
/*---------- macro ----------*/

#define PIN_NONE                    (-1)

#define PIN_LOW                     0x00
#define PIN_HIGH                    0x01

#define PIN_MODE_OUTPUT             0x00
#define PIN_MODE_INPUT              0x01
#define PIN_MODE_INPUT_PULLUP       0x02
#define PIN_MODE_INPUT_PULLDOWN     0x03
#define PIN_MODE_OUTPUT_OD          0x04

#define PIN_IRQ_MODE_RISING         0x00
#define PIN_IRQ_MODE_FALLING        0x01
#define PIN_IRQ_MODE_RISING_FALLING 0x02
#define PIN_IRQ_MODE_HIGH_LEVEL     0x03
#define PIN_IRQ_MODE_LOW_LEVEL      0x04

#define PIN_IRQ_DISABLE             0x00
#define PIN_IRQ_ENABLE              0x01

#define PIN_IRQ_PIN_NONE            PIN_NONE
/*---------- type define ----------*/

/* pin device and operations for RT-Thread */
struct device_pin
{
    struct device parent;
    const struct pin_ops *ops;
};

struct device_pin_value
{
    uint32_t pin;
    uint8_t value; /* PIN_LOW or PIN_HIGH */
};

struct device_pin_mode
{
    uint32_t pin;
    uint8_t mode; /* e.g. PIN_MODE_OUTPUT */
};

struct rt_pin_irq_hdr
{
    uint32_t pin;
    uint8_t mode; /* e.g. PIN_IRQ_MODE_RISING */
    void (*hdr)(void *args);
    void *args;
};
struct pin_ops
{
    void (*pin_mode)(struct device *device, uint32_t pin, uint8_t mode);
    void (*pin_write)(struct device *device, uint32_t pin, uint8_t value);
    int8_t (*pin_read)(struct device *device, uint32_t pin);
    //
    // fp_err_t (*pin_attach_irq)(struct device *device, uint32_t pin, uint8_t mode, void (*hdr)(void *args), void *args);
    // fp_err_t (*pin_detach_irq)(struct device *device, uint32_t pin);
    // fp_err_t (*pin_irq_enable)(struct device *device, uint32_t pin, uint8_t enabled);
    // uint32_t (*pin_get)(const char *name);
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

int device_pin_register(const char *name, const struct pin_ops *ops);
/*---------- end of file ----------*/
#endif
