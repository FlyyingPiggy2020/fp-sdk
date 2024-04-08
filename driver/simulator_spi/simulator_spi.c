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
 * @FilePath     : simulator_spi.c
 * @Author       : lxf
 * @Date         : 2024-03-27 11:18:51
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-03-27 11:43:57
 * @Brief        :
 */

/*---------- includes ----------*/

#include "inc/simulator_spi.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

/**
 * @brief 初始化spi gpio 的默认状态
 * @param {spi_dev_device} *spi
 * @return {*}
 */
void spi_gpio_level_init(struct spi_dev_device *spi)
{
    spi->cs(1);                     /* 保持片选信号高电平 */
    spi->ops->clk(spi->ops->sckpl); /* 保持空闲电平 */
}
/**
 * @brief spi 8 bit 读写
 * @param {ops_spi_bus_device} *spi_bus
 * @param {int8_t} data
 * @return {*}
 */
int8_t ops_spi_send_byte(struct spi_ops *spi_bus, int8_t data)
{
    uint8_t i = 0;
    uint8_t inDate = data;
    uint8_t outBit = 0;
    uint8_t outDate = 0;
    uint8_t sck_idle_level = 0;

    /* sckpl(时钟极性)等于0时，clk空闲时为低电平,反之为高电平。 */
    if (spi_bus->sckpl == 0) {
        sck_idle_level = 0;
    } else {
        sck_idle_level = 1;
    }
    spi_bus->clk(sck_idle_level); /* 保持空闲时的电平为sckpl */
    /* sckph(时钟相位)等于0时，先输出sdo，再变换clk，反之亦反。 */
    if (spi_bus->sckph == 0) {
        for (i = 0; i < 8; i++) {
            if (inDate & 0x80) {
                spi_bus->sdo(1);
            } else {
                spi_bus->sdo(0);
            }
            outBit = spi_bus->sdi();
            if (outBit) {
                outDate |= 0x1;
            }
            inDate <<= 1;
            if (i < 7) {
                outDate <<= 1;
            }
            spi_bus->clk(!sck_idle_level);
            spi_bus->delayus(1);
            spi_bus->clk(sck_idle_level);
            spi_bus->delayus(1);
        }
    } else {
        for (i = 0; i < 8; i++) {
            spi_bus->clk(!sck_idle_level);
            if (inDate & 0x80) {
                spi_bus->sdo(1);
            } else {
                spi_bus->sdo(0);
            }
            outBit = spi_bus->sdi();
            if (outBit) {
                outDate |= 0x1;
            }
            inDate <<= 1;
            if (i < 7) {
                outDate <<= 1;
            }
            spi_bus->delayus(1);
            spi_bus->clk(sck_idle_level);
            spi_bus->delayus(1);
        }
    }
    spi_bus->clk(sck_idle_level); /* 保持空闲时的电平为sckpl */
    spi_bus->delayus(2);
    return outDate;
}

/**
 * @brief 16bit数据读写
 * @param {ops_spi_bus_device} *spi_bus
 * @param {int16_t} data
 * @return {*}
 */
int16_t ops_spi_send_2byte(struct spi_ops *spi_bus, int16_t data)
{ /* 16bit todo 未测试 */
    int16_t i, recv = 0;

    for (i = 0x8000; i != 0; i >>= 1) {
        spi_bus->clk(0);
        spi_bus->sdo((data & i) ? 1 : 0);
        spi_bus->delayus(1);
        spi_bus->clk(1);
        spi_bus->delayus(1);
        if (spi_bus->sdi())
            recv |= i;
    }
    spi_bus->clk(0);

    return recv;
}

int ops_spi_bus_xfer(struct spi_dev_device *spi_dev, simulator_spi_message *msg)
{
    int size;

    size = msg->length;
    if (msg->cs_take) { /* take CS */
        spi_dev->cs(0);
    }
    if (spi_dev->data_width <= 8) { /* 8bit */
        const unsigned char *send_ptr = msg->send_buf;
        unsigned char *recv_ptr = msg->recv_buf;

        while (size--) {
            unsigned char data = 0xFF;
            if (send_ptr != 0) {
                data = *send_ptr++;
            }
            data = ops_spi_send_byte(spi_dev->ops, data);
            if (recv_ptr != 0) {
                *recv_ptr++ = data;
            }
        }
    } else if (spi_dev->data_width <= 16) { /* 16bit */
        const unsigned short *send_ptr = msg->send_buf;
        unsigned short *recv_ptr = msg->recv_buf;

        while (size--) {
            unsigned short data = 0xFF;

            if (send_ptr != 0) {
                data = *send_ptr++;
            }
            data = ops_spi_send_2byte(spi_dev->ops, data);
            if (recv_ptr != 0) {
                *recv_ptr++ = data;
            }
        }
    }
    if (msg->cs_release) { /* release CS */
        spi_dev->cs(1);
    }

    return msg->length;
}

/**
 * @brief 标准spi，常规操作，发送完一帧再接收，如读取某芯片寄存器的值；
 * @param {spi_dev_device} *spi_dev
 * @param {void} *send_buff
 * @param {unsigned short} send_size
 * @param {void} *recv_buff
 * @param {unsigned short} recv_size
 * @return {*}
 */
int spi_send_then_recv(struct spi_dev_device *spi_dev, const void *send_buff, unsigned short send_size, void *recv_buff, unsigned short recv_size)
{

    simulator_spi_message message;

    message.length = send_size;
    message.send_buf = send_buff;
    message.recv_buf = 0;
    message.cs_take = 1;
    message.cs_release = 0;
    ops_spi_bus_xfer(spi_dev, &message);

    message.length = recv_size;
    message.send_buf = 0;
    message.recv_buf = recv_buff;
    message.cs_take = 0;
    message.cs_release = 1;
    ops_spi_bus_xfer(spi_dev, &message);

    return 0;
}
/**
 * @brief 标准spi，常规操作，发送完一帧再发送，如向某芯片寄存器（地址）写入数据；
 * @param {spi_dev_device} *spi_dev
 * @param {void} *send_buff1
 * @param {unsigned short} send_size1
 * @param {void} *send_buff2
 * @param {unsigned short} send_size2
 * @return {*}
 */
int spi_send_then_send(struct spi_dev_device *spi_dev, const void *send_buff1, unsigned short send_size1, const void *send_buff2, unsigned short send_size2)
{
    simulator_spi_message message;

    message.length = send_size1;
    message.send_buf = send_buff1;
    message.recv_buf = 0;
    message.cs_take = 1;
    message.cs_release = 0;
    ops_spi_bus_xfer(spi_dev, &message);

    message.length = send_size2;
    message.send_buf = send_buff2;
    message.recv_buf = 0;
    message.cs_take = 0;
    message.cs_release = 1;
    ops_spi_bus_xfer(spi_dev, &message);
    return 0;
}
/**
 * @brief 非标spi，具体看芯片时序图，产生时钟信号，发送完成的同时，也接收完成；第二种情况是，只接收，发送动作只是用来产生时钟信号，如一些AD芯片；
 * @param {spi_dev_device} *spi_dev
 * @param {void} *send_buff
 * @param {void} *recv_buff
 * @param {unsigned short} data_size
 * @return {*}
 */
int spi_send_recv(struct spi_dev_device *spi_dev, const void *send_buff, void *recv_buff, unsigned short data_size)
{
    simulator_spi_message message;

    message.length = data_size;
    message.send_buf = send_buff;
    message.recv_buf = recv_buff;
    message.cs_take = 1;
    message.cs_release = 1;
    ops_spi_bus_xfer(spi_dev, &message);
    return 0;
}
/**
 * @brief 标准或非标spi都使用，只发送无返回值或者无须理会返回值，如spi LCD屏。
 * @param {spi_dev_device} *spi_dev
 * @param {void} *send_buff
 * @param {unsigned short} send_size
 * @return {*} 0
 */
int spi_send(struct spi_dev_device *spi_dev, const void *send_buff, unsigned short send_size)
{
    simulator_spi_message message;

    message.length = send_size;
    message.send_buf = send_buff;
    message.recv_buf = 0;
    message.cs_take = 1;
    message.cs_release = 1;
    ops_spi_bus_xfer(spi_dev, &message);
    return 0;
}

/**
 * @brief 非标准操作，不控制cs引脚;例如si4460芯片是否拉高CS和CTS寄存器的值有关..此时CS引脚需要交给具体的设备去管理。
 * @param {spi_dev_device} *spi_dev
 * @param {void} *send_buff
 * @param {void} *recv_buff
 * @param {unsigned short} data_size
 * @return {*}
 */
int spi_send_recv_without_cs(struct spi_dev_device *spi_dev, const void *send_buff, void *recv_buff, unsigned short data_size)
{
    simulator_spi_message message;

    message.length = data_size;
    message.send_buf = send_buff;
    message.recv_buf = recv_buff;
    message.cs_take = 0;
    message.cs_release = 0;
    ops_spi_bus_xfer(spi_dev, &message);
    return 0;
}

/*---------- end of file ----------*/
