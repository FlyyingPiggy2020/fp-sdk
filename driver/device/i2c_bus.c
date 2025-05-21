/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : i2c_bus.c
 * @Author       : lxf
 * @Date         : 2024-12-08 11:54:33
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-12-10 11:55:23
 * @Brief        :
 */

/*---------- includes ----------*/
#include "i2c_bus.h"
#include "drv_err.h"
#include "driver.h"
#include <stdbool.h>
/*---------- macro ----------*/
#define SET_SDA(ops, val) ops->sda_set(val)
#define SET_SCL(ops, val) ops->scl_set(val)
#define GET_SDA(ops)      ops->sda_get()
#define GET_SCL(ops)      ops->scl_get()

#define SDA_L(ops)        SET_SDA(ops, false)
#define SDA_H(ops)        SET_SDA(ops, true)
#define SCL_L(ops)        SET_SCL(ops, false)
#define SCL_H(ops)        SET_SCL(ops, true)
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t i2c_open(driver_t **pdrv);
static void i2c_close(driver_t **pdrv);
static int32_t i2c_ioctl(driver_t **pdrv, uint32_t cmd, void *args);

// static int32_t i2c_master_send(i2cbus_describe_t *bus, uint16_t addr, uint16_t flags, const uint8_t *buf, uint32_t count);
// static int32_t i2c_master_recv(i2cbus_describe_t *bus, uint16_t addr, uint16_t flags, uint8_t *buf, uint32_t count);
static int32_t i2c_bus_device_control(i2cbus_describe_t *bus, void *arg);
static int32_t i2c_transfer(i2cbus_describe_t *bus, i2c_msg_t msgs[], uint32_t num);

//
static void i2c_stop(i2c_bit_ops_t *ops);
/*---------- variable ----------*/
DRIVER_DEFINED(i2c_bus, i2c_open, i2c_close, NULL, NULL, i2c_ioctl, NULL);

static struct protocol_callback ioctl_cbs[] = {
    { IOCTL_I2CBUS_CTRL_RW, i2c_bus_device_control },
};
/*---------- function ----------*/
static int32_t i2c_open(driver_t **pdrv)
{
    i2cbus_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_WRONG_ARGS;

    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    ASSERT(pdrv != NULL);

    do {
        if (!pdesc) {
            break;
        }
        err = DRV_ERR_OK;
        if (pdesc->ops->init) {
            if (!pdesc->ops->init()) {
                err = DRV_ERR_ERROR;
                break;
            }
            i2c_stop(pdesc->ops);
        }
    } while (0);
    return err;
}

static void i2c_close(driver_t **pdrv)
{
    i2cbus_describe_t *pdesc = NULL;
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    ASSERT(pdesc != NULL);
    if (pdesc->ops->deinit) {
        pdesc->ops->deinit();
    }
}

static int32_t i2c_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    i2cbus_describe_t *pdesc = NULL;
    int32_t (*cb)(i2cbus_describe_t *, void *) = NULL;
    ASSERT(pdrv);

    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;

    do {
        if (!pdesc) {
            break;
        }
        cb = (int32_t(*)(i2cbus_describe_t *, void *))protocol_callback_find(cmd, ioctl_cbs, ARRAY_SIZE(ioctl_cbs));
        if (!cb) {
            break;
        }
        err = cb(pdesc, args);
    } while (0);

    return err;
}

static inline void i2c_delay(i2c_bit_ops_t *ops)
{
    ops->udelay(ops->delay_us);
}

static void i2c_recover(i2c_bit_ops_t *ops)
{
    for (uint8_t i = 0; i < 32; i++) {
        SCL_L(ops);
        i2c_delay(ops);
        SCL_H(ops);
        i2c_delay(ops);
        if (GET_SDA(ops)) {
            break;
        }
    }
}

static void i2c_restart(i2c_bit_ops_t *ops)
{
    SDA_H(ops);
    SCL_H(ops);
    i2c_delay(ops);
    SDA_L(ops);
    i2c_delay(ops);
    SCL_L(ops);
}

static void i2c_start(i2c_bit_ops_t *ops)
{
    if (!GET_SDA(ops)) {
        i2c_recover(ops);
    }
    SDA_H(ops);
    SCL_H(ops);
    i2c_delay(ops);
    SDA_L(ops);
    i2c_delay(ops);
    SCL_L(ops);
    i2c_delay(ops);
}
static void i2c_stop(i2c_bit_ops_t *ops)
{
    SDA_L(ops);
    i2c_delay(ops);
    SCL_H(ops);
    i2c_delay(ops);
    SDA_H(ops);
    i2c_delay(ops);
}

/**
 * @brief
 * @param {i2c_bit_ops_t} *ops
 * @return {*}
 */
static int32_t i2c_waitack(i2c_bit_ops_t *ops)
{
    int32_t re;
    SDA_H(ops);
    i2c_delay(ops);
    SCL_H(ops);
    i2c_delay(ops);
    if (GET_SDA(ops)) {
        re = DRV_ERR_ERROR;
    } else {
        re = DRV_ERR_EOK;
    }
    SCL_L(ops);
    i2c_delay(ops);
    return re;
}

static uint32_t i2c_writeb(i2cbus_describe_t *bus, uint8_t data)
{
    i2c_bit_ops_t *ops = bus->ops;

    for (uint8_t i = 0; i < 8; i++) {
        if (data & 0x80) {
            SDA_H(ops);
        } else {
            SDA_L(ops);
        }
        i2c_delay(ops);
        SCL_H(ops);
        i2c_delay(ops);
        SCL_L(ops);
        if (i == 7) {
            SDA_H(ops);
        }
        data <<= 1;
        i2c_delay(ops);
    }
    return 0;
}

static uint8_t i2c_readb(i2cbus_describe_t *bus)
{
    uint8_t i;
    uint8_t value;
    i2c_bit_ops_t *ops = bus->ops;
    value = 0;
    for (i = 0; i < 8; i++) {
        value <<= 1;
        SCL_H(ops);
        i2c_delay(ops);
        if (GET_SDA(ops)) {
            value++;
        }
        SCL_L(ops);
        i2c_delay(ops);
    }
    return value;
}

/**
 * @brief
 * @param {i2cbus_describe_t} *bus
 * @param {i2c_msg_t} *msg
 * @return {*} 没收到应答返回-3
 */
static int32_t i2c_send_bytes(i2cbus_describe_t *bus, i2c_msg_t *msg)
{
    int32_t bytes = 0;
    const uint8_t *ptr = msg->buf;
    int32_t count = msg->len;
    i2c_bit_ops_t *ops = bus->ops;

    while (count > 0) {
        i2c_writeb(bus, *ptr);
        if (i2c_waitack(ops) != DRV_ERR_EOK) {
            return DRV_ERR_TIME_OUT;
        }
        count--;
        ptr++;
        bytes++;
    }
    return bytes;
}

static int32_t i2c_ack(i2cbus_describe_t *bus)
{
    i2c_bit_ops_t *ops = bus->ops;

    SDA_L(ops);
    i2c_delay(ops);
    SCL_H(ops);
    i2c_delay(ops);
    SCL_L(ops);
    i2c_delay(ops);
    SDA_H(ops);
    return 0;
}

static int32_t i2c_nack(i2cbus_describe_t *bus)
{
    i2c_bit_ops_t *ops = bus->ops;

    SDA_H(ops);
    i2c_delay(ops);
    SCL_H(ops);
    i2c_delay(ops);
    SCL_L(ops);
    i2c_delay(ops);
    return 0;
}

static int32_t i2c_recv_bytes(i2cbus_describe_t *bus, i2c_msg_t *msg)
{
    int32_t val;
    int32_t bytes = 0; /* actual byetes */
    uint8_t *ptr = msg->buf;
    int32_t count = msg->len;
    uint32_t flags = msg->flags;

    while (count > 0) {
        val = i2c_readb(bus);
        *ptr = val;
        bytes++;
        ptr++;
        count--;
        if (!(flags & I2C_NO_READ_ACK)) {
            if (count) {
                i2c_ack(bus);
            } else {
                i2c_nack(bus);
            }
        }
    }
    return bytes;
}

static int32_t i2c_send_address(i2cbus_describe_t *bus, uint8_t addr, int32_t retries)
{
    i2c_bit_ops_t *ops = bus->ops;

    int32_t i;
    int32_t ret = DRV_ERR_TIME_OUT;

    for (i = 0; i <= retries; i++) {
        i2c_writeb(bus, addr);
        ret = i2c_waitack(ops);
        if (ret == DRV_ERR_OK || i == retries) {
            break;
        }
        i2c_stop(ops);
        i2c_delay(ops);
        i2c_start(ops);
    }
    return ret;
}

static int32_t i2c_bit_send_address(i2cbus_describe_t *bus, i2c_msg_t *msg)
{
    uint16_t flags = msg->flags;
    uint16_t ignore_nack = msg->flags & I2C_IGNORE_NACK;
    i2c_bit_ops_t *ops = bus->ops;

    uint8_t addr1, addr2;
    int32_t retries;
    int32_t ret;

    retries = ignore_nack ? 0 : bus->retries;

    if (flags & I2C_ADDR_10BIT) {
        addr1 = 0xf0 | ((msg->addr >> 7) & 0x06);
        addr2 = msg->addr & 0Xff;
        ret = i2c_send_address(bus, addr1, retries);
        if ((ret != DRV_ERR_EOK) && !ignore_nack) {
            return ret;
        }

        i2c_writeb(bus, addr2);
        ret = i2c_waitack(ops);
        if (ret != DRV_ERR_OK && !ignore_nack) {
            return ret;
        }
        if (flags & I2C_BUS_RD) {
            i2c_restart(ops);
            addr1 |= 0x01;
            ret = i2c_send_address(bus, addr1, retries);
            if ((ret != DRV_ERR_OK) && !ignore_nack) {
                return ret;
            }
        }
    } else {
        /* 7 bit addr */
        addr1 = msg->addr << 1;
        if (flags & I2C_BUS_RD) {
            addr1 |= 1;
        }
        ret = i2c_send_address(bus, addr1, retries);
        if ((ret != DRV_ERR_OK) && !ignore_nack) {
            return ret;
        }
    }
    return DRV_ERR_OK;
}

static int32_t i2c_bit_xfer(i2cbus_describe_t *bus, i2c_msg_t msgs[], uint32_t num)
{
    i2c_msg_t *msg;
    i2c_bit_ops_t *ops = bus->ops;
    int32_t ret = 0;
    uint32_t i;
    uint16_t ignore_nack;

    if (num == 0) {
        return 0;
    }

    for (i = 0; i < num; i++) {
        msg = &msgs[i];
        ignore_nack = msg->flags & I2C_IGNORE_NACK;
        if (!(msg->flags & I2C_NO_START)) {
            if (i) {
                i2c_restart(ops);
            } else {
                i2c_start(ops);
            }
            ret = i2c_bit_send_address(bus, msg);
            if (ret != DRV_ERR_OK && !ignore_nack) {
                goto out;
            }
        }
        if (msg->flags & I2C_BUS_RD) {
            i2c_recv_bytes(bus, msg);
            ret = DRV_ERR_OK;
        } else {
            ret = i2c_send_bytes(bus, msg);
            if (ret < msg->len) {
                if (ret >= 0) {
                    ret = DRV_ERR_WRONG_ARGS;
                }
                goto out;
            }
        }
    }
    ret = i;
out:
    if (!(msg->flags & I2C_NO_STOP)) {
        i2c_stop(ops);
    }
    return ret;
}

static int32_t i2c_transfer(i2cbus_describe_t *bus, i2c_msg_t msgs[], uint32_t num)
{
    int32_t ret;

    i2c_bit_ops_t *ops = bus->ops;

    if (ops->lock) {
        ops->lock(ops);
    }
    ret = i2c_bit_xfer(bus, msgs, num);
    if (ops->unlock) {
        ops->unlock(ops);
    }
    return ret;
}

/**
 * @brief
 * @param {i2cbus_describe_t} *bus
 * @param {void} *arg
 * @return {*} 返回值 实际执行的msg number
 */
static int32_t i2c_bus_device_control(i2cbus_describe_t *bus, void *arg)
{
    i2c_priv_data_t *priv_data;
    int32_t ret;
    ASSERT(bus);
    ASSERT(bus->ops);

    priv_data = arg;
    ret = i2c_transfer(bus, priv_data->msgs, priv_data->number);
    return (ret == priv_data->number) ? DRV_ERR_OK : DRV_ERR_ERROR;
}
/*---------- end of file ----------*/
