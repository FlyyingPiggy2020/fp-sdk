/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : at24cxx.c
 * @Author       : lxf
 * @Date         : 2024-12-10 08:43:28
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-01-13 13:54:50
 * @Brief        : AT24C系列EEPROM驱动(I2C接口)
 * @features     :
 *               - 支持AT24C01/02/04/08/16/32/64/128/256等型号
 *               - 支持页写入模式(自动分页处理)
 *               - 写入后自动校验(回读比较)
 *               - 支持1/2字节地址模式
 *               - 可选写保护(WP)控制
 *               - 可选电源(VCC)控制
 *               - 通过I2C总线设备通信
 *
 * @usage        :
 *               @code
 *               // 1. 定义AT24C设备(在bsp层实现)
 *               static void my_wp_enable(bool on) {
 *                   // 控制写保护引脚
 *               }
 *               static at24cxx_describe_t at24c02 = {
 *                   .config = {
 *                       .ee_size = 256,           // 2Kb = 256字节
 *                       .ee_page_size = 8,        // 页大小8字节
 *                       .ee_dev_addr = 0xA0,      // 设备地址
 *                       .ee_addr_btyes = 1,       // 1字节地址
 *                   },
 *                   .ops = {
 *                       .wp_enable = my_wp_enable,
 *                   },
 *                   .bus_name = "i2c1",          // 绑定的I2C总线
 *                   .retries = 1000,// 页写入后需要等待内部写周期
 *               };
 *               DEVICE_DEFINED(eeprom, at24cxx, &at24c02);
 *
 *               // 2. 使用EEPROM
 *               device_t *eeprom = device_open("eeprom");
 *
 *               // 3. 写入数据(addition为地址偏移)
 *               uint8_t data[] = "Hello EEPROM!";
 *               device_write(eeprom, data, 0, sizeof(data));
 *
 *               // 4. 读取数据
 *               uint8_t rx_buf[32];
 *               device_read(eeprom, rx_buf, 0, sizeof(rx_buf));
 *
 *               // 5. 关闭设备
 *               device_close(eeprom);
 *               @endcode
 *
 * @note         AT24C系列EEPROM使用I2C接口,页写入后需要等待内部写周期(约5-10ms)。
 *               页写入延时通过I2C驱动内的重发次数来实现，这样等待时间最短
 *               驱动自动处理页边界,写入后回读校验确保数据正确。
 *
 * @warning      写入前会解除写保护(wp_enable=0),写入后重新使能写保护。
 *               页写入不能跨页边界,驱动会自动分页处理。
 */

/*---------- includes ----------*/
#include "at24cxx.h"
#include "options.h"
#include "driver.h"
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
/**
 * @Brief  打开AT24C设备,绑定I2C总线
 * @param  pdrv: 驱动指针
 * @return 0=成功, <0=失败
 */
static int32_t at24cxx_open(driver_t **pdrv)
{
    at24cxx_describe_t *pdesc = NULL;
    int32_t err = E_WRONG_ARGS;
    void *bus = NULL;
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;

    assert(pdrv != NULL);

    do {
        if (!pdesc) {
            break;
        }
        err = E_OK;

        // TODO:FIX 此处有bug，如果init函数为空，最后eeprom也会返回E_OK
        if (pdesc->ops.init) {
            if (!pdesc->ops.init()) {
                err = E_ERROR;
                break;
            }
        }
        /* 绑定i2c总线 */
        if (NULL == (bus = device_open(pdesc->bus_name))) {
            xlog_error("%s bind %s failed!\n", container_of(pdrv, device_t, pdrv)->dev_name, pdesc->bus_name);
            if (pdesc->ops.deinit) {
                pdesc->ops.deinit();
            }
            err = E_ERROR;
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

/**
 * @Brief  关闭AT24C设备,释放资源
 * @param  pdrv: 驱动指针
 * @return 无
 */
static void at24cxx_close(driver_t **pdrv)
{
    at24cxx_describe_t *pdesc = NULL;

    assert(pdrv);
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
    int32_t err = E_OK;

    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    uint16_t pageWriteSize = pdesc->config.ee_page_size - addition % pdesc->config.ee_page_size;
    uint8_t *ptr = buf;
    uint8_t compare_buf[256] = { 0 };
    uint8_t *compare_ptr = compare_buf;
    assert(pdesc);
    assert(pdesc->bus);

    if (addition + len > pdesc->config.ee_size) {
        err = E_WRONG_ARGS;
        return err;
    }
    if (pdesc->config.ee_page_size > 128) {
        err = E_WRONG_ARGS;
        return err;
    }
    if (pdesc && pdesc->bus) {
        if (pdesc->ops.wp_enable) {
            pdesc->ops.wp_enable(1);
        }
        if (pdesc->ops.unlock) {
            pdesc->ops.unlock(pdesc);
        }
        while (len) {
            if (len > pageWriteSize) {
                if (_at_24cxx_wrtie_page(pdrv, ptr, addition, pageWriteSize)) {
                    err = E_ERROR;
                    break;
                }
                if (_at_24cxx_read_page(pdrv, compare_ptr, addition, pageWriteSize)) {
                    err = E_ERROR;
                    break;
                }
                if (memcmp(compare_ptr, ptr, pageWriteSize)) {
                    err = E_ERROR;
                    break;
                }
                addition += pageWriteSize;
                ptr += pageWriteSize;
                len -= pageWriteSize;
                pageWriteSize = pdesc->config.ee_page_size;
            } else {
                if (_at_24cxx_wrtie_page(pdrv, ptr, addition, len)) {
                    err = E_ERROR;
                    break;
                }
                if (_at_24cxx_read_page(pdrv, compare_ptr, addition, len)) {
                    err = E_ERROR;
                    break;
                }
                if (memcmp(compare_ptr, ptr, len)) {
                    err = E_ERROR;
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
        err = E_POINT_NONE;
    }
    return err;
}
static int32_t at24cxx_read(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len)
{
    at24cxx_describe_t *pdesc = NULL;
    int32_t err = E_OK;
    uint16_t pageReadSize = pdesc->config.ee_page_size - addition % pdesc->config.ee_page_size;
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    uint8_t *ptr = buf;
    assert(pdesc);
    assert(pdesc->bus);

    if (addition + len > pdesc->config.ee_size) {
        err = E_WRONG_ARGS;
        return err;
    }
    if (pdesc && pdesc->bus) {
        if (pdesc->ops.wp_enable) {
            pdesc->ops.wp_enable(1);
        }
        if (pdesc->ops.unlock) {
            pdesc->ops.unlock(pdesc);
        }
        while (len) {
            if (len > pageReadSize) {
                if (_at_24cxx_read_page(pdrv, ptr, addition, pageReadSize)) {
                    err = E_ERROR;
                    break;
                }
                addition += pageReadSize;
                ptr += pageReadSize;
                len -= pageReadSize;
                pageReadSize = pdesc->config.ee_page_size;
            } else {
                if (_at_24cxx_read_page(pdrv, ptr, addition, len)) {
                    err = E_ERROR;
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
        err = E_POINT_NONE;
    }
    return err;
}
static int32_t at24xx_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    at24cxx_describe_t *pdesc = NULL;
    int32_t err = E_WRONG_ARGS;
    int32_t (*cb)(at24cxx_describe_t *, void *) = NULL;

    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if (!pdesc) {
            break;
        }
        cb = (int32_t (*)(at24cxx_describe_t *, void *))protocol_callback_find(cmd, ioctl_cbs, ARRAY_SIZE(ioctl_cbs));
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
 * @return {*} E_OK or E_ERROR
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
