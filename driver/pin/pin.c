/*
 * Copyright (c) 2024 by Moorgen Tech. Co, Ltd.
 * @FilePath     : pin.c
 * @Author       : lxf
 * @Date         : 2024-02-22 17:17:43
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-02-23 11:59:39
 * @Brief        : STM32 引脚
 */

/*---------- includes ----------*/

#include "inc/pin.h"

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/

static struct device_pin _hw_pin;

/**
 * @brief 通过driver manager来读取gpio
 * @param {device_t} *dev
 * @param {int} pos
 * @param {void} *buffer
 * @param {int} size
 * @return {*}
 */
static fp_size_t _pin_read(device_t *dev, int pos, void *buffer, int size)
{
    struct device_pin_value *value;
    struct device_pin *pin = (struct device_pin *)dev;

    assert(pin != NULL);

    value = (struct device_pin_value *)buffer;
    if (value == NULL || size != sizeof(*value)) {
        return 0;
    }

    value->value = pin->ops->pin_read(dev, value->pin);
    return size;
}

static fp_size_t _pin_write(device_t *dev, int pos, const void *buffer, int size)
{
    struct device_pin_value *value;
    struct device_pin *pin = (struct device_pin *)dev;

    /* check parameters */
    assert(pin != NULL);

    value = (struct device_pin_value *)buffer;
    if (value == NULL || size != sizeof(*value))
        return 0;

    pin->ops->pin_write(dev, (uint32_t)value->pin, (uint8_t)value->value);

    return size;
}
static fp_err_t _pin_control(device_t *dev, int cmd, void *args)
{
    struct device_pin_mode *mode;
    struct device_pin *pin = (struct device_pin *)dev;

    /* check parameters */
    assert(pin != NULL);

    mode = (struct device_pin_mode *)args;
    if (mode == NULL)
        return -FP_ERROR;

    pin->ops->pin_mode(dev, (uint32_t)mode->pin, (uint8_t)mode->mode);

    return 0;
}
// clang-format off
static device_ops pin_ops =
{
    NULL,
    NULL,
    NULL,
    _pin_read,
    _pin_write,
    _pin_control
};
// clang-format on

/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

/**
 * @brief register pin ops driver
 * @param {char} *name
 * @param {pin_ops} *ops
 * @return {*}
 */
int device_pin_register(const char *name, const struct pin_ops *ops)
{
    _hw_pin.parent.rx_indicate = NULL;
    _hw_pin.parent.tx_complete = NULL;

    _hw_pin.parent.ops = &pin_ops;

    _hw_pin.ops = ops;
    device_register(&_hw_pin.parent, name, DEVICE_FLAG_RDWR);
    return 0;
}

/**
 * @brief chang pin mode
 * @param {uint32_t} pin
 * @param {uint8_t} mode
 * @return {*}
 */
void pin_mode(uint32_t pin, uint8_t mode)
{
    assert(_hw_pin.ops != NULL);
    _hw_pin.ops->pin_mode(&_hw_pin.parent, pin, mode);
}

/**
 * @brief write pin level
 * @param {uint32_t} pin
 * @param {uint8_t} value
 * @return {*}
 */
void pin_write(uint32_t pin, uint8_t value)
{
    assert(_hw_pin.ops != NULL);
    _hw_pin.ops->pin_write(&_hw_pin.parent, pin, value);
}

/**
 * @brief read pin level
 * @param {uint32_t} pin
 * @param {uint8_t} value
 * @return {*}
 */
int8_t pin_read(uint32_t pin, uint8_t value)
{
    assert(_hw_pin.ops != NULL);
    return _hw_pin.ops->pin_read(&_hw_pin.parent, pin);
}
/*---------- end of file ----------*/
