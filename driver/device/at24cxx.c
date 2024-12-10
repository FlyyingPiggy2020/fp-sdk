/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : at24cxx.c
 * @Author       : lxf
 * @Date         : 2024-12-10 08:43:28
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-12-10 10:56:56
 * @Brief        :
 */

/*---------- includes ----------*/
#include "at24cxx.h"
#include "drv_err.h"
#include "driver.h"
#include <stdbool.h>
#include "i2c_bus.h"
/*---------- macro ----------*/

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t at24cxx_open(driver_t **pdrv);
static void at24cxx_close(driver_t **pdrv);
static int32_t at24cxx_write(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len);
static int32_t at24cxx_read(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len);
static int32_t at24xx_ioctl(driver_t **pdrv, uint32_t cmd, void *args);

//
static int32_t _at_24cxx_wrtie_page(driver_t **pdrv, uint8_t *buf, uint32_t addition, uint32_t len);
static int32_t _at_24cxx_read_page(driver_t **pdrv, uint8_t *buf, uint32_t addition, uint32_t len);

/*---------- variable ----------*/
DRIVER_DEFINED(at24cxx, at24cxx_open, at24cxx_close, at24cxx_write, at24cxx_read, at24xx_ioctl, NULL);

static struct protocol_callback ioctl_cbs[] = {
    { 0, NULL },
};
/*---------- function ----------*/
static int32_t at24cxx_open(driver_t **pdrv)
{
    at24cxx_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_WRONG_ARGS;
    void *bus = NULL;
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    
    ASSERT(pdrv != NULL);

    do {
        if (!pdesc) {
            break;
        }
        err = DRV_ERR_OK;
        if (pdesc->ops.init) {
            if (!pdesc->ops.init()) {
                err = DRV_ERR_ERROR;
                break;
            }
        }
        /* 绑定i2c总线 */
        if (NULL == (bus = device_open(pdesc->bus_name))) {
           TRACE("%s bind %s failed!\n",container_of(pdrv, device_t, pdrv)->dev_name, pdesc->bus_name);
           if (pdesc->ops.deinit) {
               pdesc->ops.deinit();
           }
           err = DRV_ERR_ERROR;
           break;
        }
        if (pdesc->ops.vcc_enable) {
            pdesc->ops.vcc_enable(1);
        }
        if (pdesc->ops.wp_enable) {
            pdesc->ops.wp_enable(0);
        }
        pdesc->bus = bus;
    } while (0);
    return err;
}
static void at24cxx_close(driver_t **pdrv)
{
    at24cxx_describe_t *pdesc = NULL;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if (pdesc) {
        if (pdesc->bus) {
            device_close(pdesc->bus);
            pdesc->bus = NULL;
        }
        if (pdesc->ops.deinit) {
            pdesc->ops.deinit();
        }
        if (pdesc->ops.vcc_enable) {
            pdesc->ops.vcc_enable(0);
        }
        if (pdesc->ops.wp_enable) {
            pdesc->ops.wp_enable(0);
        }
    }
    return;
}

static int32_t at24cxx_write(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len)
{
    at24cxx_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_OK;
    
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    uint16_t pageWriteSize = pdesc->config.ee_page_size - addition % pdesc->config.ee_page_size;
    uint8_t *ptr = buf;
    ASSERT(pdesc);
    ASSERT(pdesc->bus);

    if (addition + len > pdesc->config.ee_size) {
        err = DRV_ERR_WRONG_ARGS;
        return err;
    }
    if (pdesc && pdesc->bus) {
        if (pdesc->ops.wp_enable) {
            pdesc->ops.wp_enable(1);
        }
        if (pdesc->ops.unlock) {
            pdesc->ops.unlock(pdesc);
        }
        while(len) {
            if (len > pageWriteSize) {
                if (_at_24cxx_wrtie_page(pdrv, ptr, addition, pageWriteSize)) {
                    err = DRV_ERR_ERROR;
                    break;
                }
                addition += pageWriteSize;
                ptr += pageWriteSize;
                len -= pageWriteSize;
                pageWriteSize = pdesc->config.ee_page_size;
            } else {
                if (_at_24cxx_wrtie_page(pdrv, ptr, addition, len)) {
                    err = DRV_ERR_ERROR;
                    break;
                }
                len = 0;
            }
        }
        if (pdesc->ops.lock) {
            pdesc->ops.lock(pdesc);
        }
        if (pdesc->ops.wp_enable) {
            pdesc->ops.wp_enable(0);
        }
    } else {
        err = DRV_ERR_POINT_NONE;
    }
    return err;
}
static int32_t at24cxx_read(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len)
{
    at24cxx_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_OK;
    uint16_t pageReadSize = pdesc->config.ee_page_size - addition % pdesc->config.ee_page_size;
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    uint8_t *ptr = buf;
    ASSERT(pdesc);
    ASSERT(pdesc->bus);

    if (addition + len > pdesc->config.ee_size) {
        err = DRV_ERR_WRONG_ARGS;
        return err;
    }
    if (pdesc && pdesc->bus) {
        if (pdesc->ops.wp_enable) {
            pdesc->ops.wp_enable(1);
        }
        if (pdesc->ops.unlock) {
            pdesc->ops.unlock(pdesc);
        }
        while(len) {
            if (len > pageReadSize) {
                if (_at_24cxx_read_page(pdrv, ptr, addition, pageReadSize)) {
                    err = DRV_ERR_ERROR;
                    break;
                }
                addition += pageReadSize;
                ptr += pageReadSize;
                len -= pageReadSize;
                pageReadSize = pdesc->config.ee_page_size;
            } else {
                if (_at_24cxx_read_page(pdrv, ptr, addition, len)) {
                    err = DRV_ERR_ERROR;
                    break;
                }
                len = 0;
            }
        }
        if (pdesc->ops.lock) {
            pdesc->ops.lock(pdesc);
        }
        if (pdesc->ops.wp_enable) {
            pdesc->ops.wp_enable(0);
        }
    } else {
        err = DRV_ERR_POINT_NONE;
    }
    return err;
}
static int32_t at24xx_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    at24cxx_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_WRONG_ARGS;
    int32_t (*cb)(at24cxx_describe_t *, void *) = NULL;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if (!pdesc) {
            break;
        }
        cb = (int32_t(*)(at24cxx_describe_t *, void *))protocol_callback_find(cmd, ioctl_cbs, ARRAY_SIZE(ioctl_cbs));
        if (!cb) {
            break;
        }
        err = cb(pdesc, args);
    } while (0);

    return err;
}

int32_t _at_24cxx_read_page(driver_t **pdrv, uint8_t *buf, uint32_t addition, uint32_t len)
{
    at24cxx_describe_t *pdesc = NULL;
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    i2c_msg_t msgs[2];
    uint8_t addr_buf[2];

    msgs[0].addr = pdesc->config.ee_dev_addr;
    msgs[0].flags = I2C_BUS_WR;
    if (pdesc->config.ee_addr_btyes == 2) {
        addr_buf[0] = (addition >> 8) & 0xff;
        addr_buf[1] = addition & 0xff;
        msgs[0].buf = addr_buf;
        msgs[0].len = 2;
    } else {
        addr_buf[0] = addition & 0xff;
        msgs[0].buf = addr_buf;
        msgs[0].len = 1;
    }

    msgs[1].addr = pdesc->config.ee_dev_addr;
    msgs[1].flags = I2C_BUS_RD;
    msgs[1].buf = buf;
    msgs[1].len = len;

    i2c_priv_data_t arg = {
        .msgs = (i2c_msg_t *)&msgs,
        .number = 2,
    };
    return device_ioctl(pdesc->bus, IOCTL_I2CBUS_CTRL_RW, &arg);
}

/**
 * @brief eeprom按页写入
 * @param {driver_t} *
 * @param {void} *buf
 * @param {uint32_t} addition
 * @param {uint32_t} len
 * @return {*} DRV_ERR_OK or DRV_ERR_ERROR
 */
static int32_t _at_24cxx_wrtie_page(driver_t **pdrv, uint8_t *buf, uint32_t addition, uint32_t len)
{
    at24cxx_describe_t *pdesc = NULL;
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    i2c_msg_t msgs[2];
    uint8_t addr_buf[2];

    msgs[0].addr = pdesc->config.ee_dev_addr;
    msgs[0].flags = I2C_BUS_WR;
    if (pdesc->config.ee_addr_btyes == 2) {
        addr_buf[0] = (addition >> 8) & 0xff;
        addr_buf[1] = addition & 0xff;
        msgs[0].buf = addr_buf;
        msgs[0].len = 2;
    } else {
        addr_buf[0] = addition & 0xff;
        msgs[0].buf = addr_buf;
        msgs[0].len = 1;
    }

    msgs[1].addr = pdesc->config.ee_dev_addr;
    msgs[1].flags = I2C_BUS_WR | I2C_NO_START;
    msgs[1].buf = buf;
    msgs[1].len = len;

    i2c_priv_data_t arg = {
        .msgs = (i2c_msg_t *)&msgs,
        .number = 2,
    };
    return device_ioctl(pdesc->bus, IOCTL_I2CBUS_CTRL_RW, &arg);
}
/*---------- end of file ----------*/
