/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : i2c_bus.h
 * @Author       : lxf
 * @Date         : 2024-12-08 11:54:38
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-12-09 14:55:33
 * @Brief        :
 */

#ifndef __I2CBUS_H__
#define __I2CBUS_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device.h"

/*---------- macro ----------*/
#define IOCTL_I2CBUS_CTRL_RW   (IOCTL_USER_START + 0X00)

#define I2C_BUS_WR               0X0000
#define I2C_BUS_RD               (1u << 0)
#define I2C_ADDR_10BIT           (1u << 2)
#define I2C_NO_START             (1u << 4)
#define I2C_IGNORE_NACK          (1u << 5)
#define I2C_NO_READ_ACK          (1u << 6)
#define I2C_NO_STOP              (1u << 7)
/*---------- type define ----------*/

typedef struct {
    bool (*init)(void);
    void (*deinit)(void);
    void (*scl_set)(bool on);
    bool (*scl_get)(void);
    void (*sda_set)(bool on);
    bool (*sda_get)(void);
    void (*udelay)(uint32_t us);
    void (*lock)(void *data);
    void (*unlock)(void *data);
    uint32_t delay_us;
} i2c_bit_ops_t;

typedef struct {
    i2c_bit_ops_t *ops;
    uint32_t retries;
} i2cbus_describe_t;

typedef struct {
    uint16_t addr;
    uint16_t flags;
    uint16_t len;
    uint8_t *buf;
} i2c_msg_t;

typedef struct {
    i2c_msg_t *msgs;
    int32_t number;
} i2c_priv_data_t;

/*---------- variable prototype ----------*/

/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
