/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : i2c_bus.h
 * @Author       : lxf
 * @Date         : 2024-12-08 11:54:38
 * @LastEditors  : lxf 154562451@qq.com
 * @LastEditTime : 2024-12-08 18:19:54
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
#define IOCTL_I2CBUS_VCCENABLE   (IOCTL_USER_START + 0X00)
#define IOCTL_I2CBUS_VCCDISABLE  (IOCTL_USER_START + 0X01)
#define IOCTL_I2CBUS_WPENABLE    (IOCTL_USER_START + 0X02)
#define IOCTL_I2CBUS_WPDISABLE   (IOCTL_USER_START + 0X03)
#define IOCTL_I2CBUS_START       (IOCTL_USER_START + 0X04)
#define IOCTL_I2CBUS_STOP        (IOCTL_USER_START + 0X05)
#define IOCTL_I2CBUS_WAITACK     (IOCTL_USER_START + 0X06)
#define IOCTL_I2CBUS_ACK         (IOCTL_USER_START + 0X07)
#define IOCTL_I2CBUS_NACK        (IOCTL_USER_START + 0X08)
#define IOCTL_I2CBUS_CHECKDEVICE (IOCTL_USER_START + 0X09)

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
/*---------- variable prototype ----------*/

/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
