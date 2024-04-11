/*MIT License

Copyright (c) 2023 Lu Xianfan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
/*
 * Copyright (c) 2024 by Moorgen Tech. Co, Ltd.
 * @FilePath     : si446x.c
 * @Author       : lxf
 * @Date         : 2024-03-27 11:23:20
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-03-27 11:23:52
 * @Brief        : Si446x's frequency bands from 142 to 1050 Mhz. Use WDS3 generate radio_config.h
 * Max output power:
 *		+20dBm (Si4463)
 * 		+16dBm (Si4461)
 *		+13dBm (Si4460)
 */

/*---------- includes ----------*/
#define LOG_TAG "si446x"
#include "inc/si446x.h"
#include "inc/radio_config.h"
/*---------- macro ----------*/
#define CTS                   (0XFF)
/**
 * @brief Command to power-up the device and select the operational mode and functionality.
 */
#define SI446X_POWER_UP       0X02
/**
 * @brief No Operation command.
 */
#define SI446X_NOP            0X00
/**
 * @brief Reports basic information about the device.
 */
#define SI446X_PART_INFO      0X01

/**
 * @brief 	Sets the value of one or more properties.
 */
#define SI446X_SET_PROPERTY   0X11
/**
 * @brief Returns the interrupt status of ALL the possible interrupt events (both STATUS and PENDING). Optionally, it may be used to clear latched (PENDING) interrupt events.
 */
#define SI446X_GET_INT_STATUS 0X20

/**
 * @brief Used to read CTS and the command response.
 */
#define SI446X_READ_CMD_BUFF  0X44
/*---------- type define ----------*/

/*---------- variable prototype ----------*/

/*---------- function prototype ----------*/

static fp_err_t si446x_init(device_t *dev);
static fp_err_t si446x_open(device_t *dev);
/*---------- variable ----------*/

const uint8_t radio_config[] = RADIO_CONFIGURATION_DATA_ARRAY;
/*---------- function ----------*/

device_ops si446x_device_ops = {
    .init = si446x_init,
    .close = NULL,
    .ioctl = NULL,
    .open = si446x_open,
    .read = NULL,
    .write = NULL,
};
/**
 * @brief si4460注册
 * @param {si446x_device} *si446x 句柄
 * @param {si446x_property} property si446x参数设置
 * @param {si446x_xfer} *xfer si446x方法
 * @return {*}
 */
void si446x_register(struct si446x_device *si446x, const char *name, si446x_property property, si446x_xfer *xfer)
{
    assert(si446x);
    assert(xfer);

    si446x->property = property;
    si446x->xfer = xfer;
    si446x->parent.ops = &si446x_device_ops;
    device_register(&si446x->parent, name, DEVICE_FLAG_RDWR);
}

/**
 * @brief 写命令
 * @param {device_t} *dev
 * @param {uint8_t} command
 * @return {*}
 */
static void _write_command(device_t *dev, const uint8_t *command, uint8_t size)
{
    struct si446x_device *si446x = container_of(dev, struct si446x_device, parent);
    si446x_xfer *xfer = si446x->xfer;

    xfer->cs(0);
    xfer->delay_us(2);
    for (uint8_t i = 0; i < size; i++) {
        xfer->spi_read_recv(command, NULL, 1);
    }
    xfer->cs(1);
    xfer->delay_us(2);

    return;
}

/**
 * @brief 等待CTS
 * @param {device_t} *dev
 * @return {*}
 */
static fp_err_t _wait_cts(device_t *dev)
{
    struct si446x_device *si446x = container_of(dev, struct si446x_device, parent);
    si446x_xfer *xfer = si446x->xfer;

    fp_err_t retval = FP_ERROR;

    uint8_t cts = 0;
    for (uint16_t i = 0; i < 100; ++i) {
        xfer->cs(0);
        xfer->delay_us(2);
        xfer->spi_read_recv((uint8_t[]){ CTS }, &cts, 1);
        xfer->cs(1);
        xfer->delay_us(2);
        if (cts == CTS) {
            retval = FP_EOK;
            break;
        }
        xfer->delay_us(20);
    }
    return retval;
}

/**
 * @brief 获取返回的信息(一直判断CTS)
 * @param {uint8_t} out
 * @param {uint8_t} size
 * @return {*}
 */
static fp_err_t _get_resp(device_t *dev, uint8_t *recv, uint8_t size)
{
    struct si446x_device *si446x = container_of(dev, struct si446x_device, parent);
    si446x_xfer *xfer = si446x->xfer;

    assert(xfer->spi_read_recv);
    fp_err_t ret = FP_ETIMEOUT;
    uint8_t cts = 0;

    do {
        for (uint8_t i = 0; i < 100; i++) {
            xfer->cs(0);
            xfer->delay_us(2);
            xfer->spi_read_recv((uint8_t[]){ SI446X_READ_CMD_BUFF }, NULL, 1);
            xfer->spi_read_recv((uint8_t[]){ CTS }, &cts, 1);
            if (cts == CTS) {
                ret = FP_EOK;
                break;
            }
            xfer->cs(1);
            xfer->delay_us(20);
        }
        if (ret != FP_EOK) {
            break;
        }
        for (uint8_t i = 0; i < size; i++) {
            xfer->spi_read_recv((uint8_t[]){ CTS }, &recv[i], 1);
        }
        xfer->cs(1);
    } while (0);
    return ret;
}

/**
 * @brief 返回所有的中断状态
 * @param {device_t} *dev
 * @param {uint8_t} ph_clr_pend : If clear, clear any pending FILTER_MATCH interrupt. If set, leave interrupt pending.
 * @param {uint8_t} modem_clr : If clear, clear any pending FILTER_MATCH interrupt. If set, leave interrupt pending.
 * @param {uint8_t} chip_clr : If clear, clear any pending FILTER_MATCH interrupt. If set, leave interrupt pending.
 * @return {*}
 */
static fp_err_t _get_int_status(device_t *dev, uint8_t ph_clr_pend, uint8_t modem_clr_pend, uint8_t chip_clr_pend)
{
    struct si446x_device *si446x = container_of(dev, struct si446x_device, parent);
    fp_err_t ret = FP_EOK;
    uint8_t buf[8] = { 0 };

    do {
        _write_command(dev, (uint8_t[]){ SI446X_GET_INT_STATUS, ph_clr_pend, modem_clr_pend, chip_clr_pend }, 4);
        ret = _get_resp(dev, buf, 8);
        if (ret != FP_EOK) {
            break;
        }
        memcpy(&si446x->reg.int_status, buf, 8);
    } while (0);

    return ret;
}

/**
 * @brief 获取si446x的设备信息
 * @param {device_t} *dev
 * @return {*}
 */
static fp_err_t _get_part_info(device_t *dev)
{
    struct si446x_device *si446x = container_of(dev, struct si446x_device, parent);

    fp_err_t ret = FP_EOK;
    uint8_t buf[8] = { 0 };
    do {
        _write_command(dev, (uint8_t[]){ SI446X_PART_INFO }, 1);
        ret = _get_resp(dev, buf, sizeof(buf));
        if (ret != FP_EOK) {
            log_e("get si446x part info failed.");
            break;
        }
        memcpy(&si446x->reg.part_info, buf, 8);
    } while (0);
    return ret;
}

/**
 * @brief 设置参数
 * @param {device_t} *dev
 * @param {uint8_t} group 组
 * @param {uint8_t} start_prop 起始位置
 * @param {uint8_t} *pbuf 参数指针
 * @param {uint8_t} length 参数长度
 * @return {*}
 */
static fp_err_t _set_property(device_t *dev, uint8_t group, uint8_t start_prop, uint8_t *pbuf, uint8_t length)
{
    struct si446x_device *si446x = container_of(dev, struct si446x_device, parent);
    si446x_xfer *xfer = si446x->xfer;
    fp_err_t ret = FP_ERROR;
    uint8_t buf[16] = { 0 };

    if (length <= 12) {
        xfer->cs(0);
        xfer->delay_us(2);
        xfer->spi_read_recv((uint8_t[]){ SI446X_SET_PROPERTY }, NULL, 1);
        xfer->spi_read_recv((uint8_t[]){ group }, NULL, 1);
        xfer->spi_read_recv((uint8_t[]){ length }, NULL, 1);
        xfer->spi_read_recv((uint8_t[]){ start_prop }, NULL, 1);
        for (uint8_t i = 0; i < length; i++) {
            xfer->spi_read_recv(&pbuf[i], NULL, 1);
        }
        xfer->cs(1);
        xfer->delay_us(2);
        ret = _wait_cts(dev);
    }
    return ret;
}
static fp_err_t si446x_init(device_t *dev)
{
    struct si446x_device *si446x = container_of(dev, struct si446x_device, parent);
    si446x_xfer *xfer = si446x->xfer;
    si446x_reg *reg = NULL;
    const uint8_t *config_data = radio_config;
    assert(xfer->sdn);
    assert(xfer->delay_us);

    fp_err_t ret = FP_EOK;
    uint8_t buf[8] = { 0 };
    do {
        /*1. 硬件GPIO初始化*/
        if (xfer->hw_gpio_init) {
            xfer->hw_gpio_init();
        }

        /*2. POR */
        xfer->sdn(1);
        xfer->delay_us(5000);
        xfer->sdn(0);
        xfer->delay_us(5000); /* 这个延时是必须的 */

        /*3. 读取si446x info */
        if (_get_part_info(dev) != FP_EOK) {
            log_e("get si446x part info failed.");
            break;
        }

        /*4. 写入config 文件 */
        while (config_data[0] != 0x00) {
            _write_command(dev, &config_data[1], config_data[0]);
            ret = _wait_cts(dev);
            if (ret != FP_EOK) {
                log_e("config failed.");
                break;
            }
            config_data += config_data[0] + 1;
        }
        if (ret != FP_EOK) {
            break;
        }

        /* 5.清除状态寄存器  */
        if (_get_int_status(dev, 0, 0, 0) != FP_EOK) {
            log_e("get si446x int status failed.");
            break;
        }
    } while (0);

    if (ret == FP_EOK) {
        log_i("si446x init success");
    }

    return ret;
}

static fp_err_t si446x_open(device_t *dev)
{
    struct si446x_device *si446x = container_of(dev, struct si446x_device, parent);
    assert(dev);

    fp_err_t ret = FP_ERROR;
    log_i("si446x:");
    log_i("\tfrequence:%.2f", si446x->property.frequence);
    log_i("\tpartinfo:%02x%02x", si446x->reg.part_info.part[0], si446x->reg.part_info.part[1]);
    if (si446x->reg.part_info.part[0] == 0X44 && si446x->reg.part_info.part[1] == 0X60) {
        ret = FP_EOK;
    }
    return ret;
}

fp_size_t si446x_write(device_t *dev, int pos, const void *buffer, int size)
{
}
/*---------- end of file ----------*/
