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
 * @FilePath     : si446x.h
 * @Author       : lxf
 * @Date         : 2024-03-27 11:24:37
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-03-27 11:24:39
 * @Brief        :
 */

#ifndef __SI446X_H__
#define __SI446X_H__
/*---------- includes ----------*/

#include "stdint.h"
#include "fp_sdk.h"
/*---------- macro ----------*/

/**
 * @brief 该bit决定TX数据流是以同步还是异步方式处理。
 * 该bit仅适用于TX模式，且仅当sMOD_SOURCE字段也设置为TX Direct mode时才适用。
 * 
 * 在TX直接同步模式(TX Direct Synchronous mode)下，芯片通过输出 TX 位时钟（GPIO_PIN_CFG=0x10）来控制发送数据速率。主机 MCU 接收 TX 时钟的上升沿，
 * 并在每个时钟周期输出一个比特作为响应,芯片在随后的 TX 时钟下降沿将这个新数据位编入时钟。支持所有 2 级调制模式（OOK、2FSK、2GFSK）。
 * 
 * 在TX直接异步模式(TX Direct Asynchronous mode)下，主机 MCU 控制发送数据速率；RFIC 不知道传入 TX 数据流的速率，只是尽可能快地对数据进行过采样，
 * 以确定位边缘转换。该模式支持 OOK 和 2FSK，但不支持 2GFSK。
 * 
 * 在TX直接同步或 TX 直接异步模式下，都不支持 4(G)FSK
 */
enum {
    TX_DIRECT_MODE_TYPE_SNYC = 0,
    TX_DIRECT_MODE_TYPE_ASYNC = 1,
};

/**
 * @brief 选择哪个GPIO引脚作为tx数据源。仅在tx直接模式适用。
 */
enum {
    TX_DIRECT_MODE_GPIO0 = 0,
    TX_DIRECT_MODE_GPIO1 = 1,
    TX_DIRECT_MODE_GPIO2 = 2,
    TX_DIRECT_MODE_GPIO3 = 3,
};

enum {
    MOD_SOURCE_PACKET = 0,
    MOD_SOURCE_DIRECT = 1,
    MOD_SOURCE_PSEUDO = 2,
};
/*---------- type define ----------*/

typedef struct si446x_xfer {
    /**
     * @brief 发送完成的同时，也接收完成
     * @param {void} *send_buff
     * @param {void} *recv_buff
     * @param {unsigned short} data_size
     * @return {*}
     */
    int (*spi_read_recv)(const void *send_buff, void *recv_buff, unsigned short data_size);

    /**
     * @brief cs操作
     * @param {uint8_t} state
     * @return {*}
     */
    void (*cs)(uint8_t state);
    /**
     * @brief nirq回调
     * @return {*}
     */
    void (*nirq_callback)(void);
    /**
     * @brief 如果配置成直接模式，则使用direct io 进行输出
     * @param {uint8_t} level 0低电平;1高电平
     * @return {*}
     */
    void (*direct_tx_io)(uint8_t level);
    /**
     * @brief sdn引脚，拉高进入关机状态，用该引脚进入POR
     * @param {uint8_t} level 0低电平;1高电平
     * @return {*}
     */
    void (*sdn)(uint8_t level);

    /**
     * @brief 相关GPIO硬件引脚初始化
     * @return {*}
     */
    void (*hw_gpio_init)(void);

    /**
     * @brief 微妙级延时
     * @param {uint32_t} us
     * @return {*}
     */
    void (*delay_us)(uint32_t us);
} si446x_xfer;

/**
 * @brief si4460初始化设置
 */
typedef struct si446x_property {
    struct
    {
        uint8_t tx_direct_mode_type:1;
        uint8_t tx_direct_mode_gpio:2;
        uint8_t mode_source:2;
        uint8_t mode_type:3;
    }modem_mod_type;
    
} si446x_property;

#pragma pack(1)
typedef struct part_info {
    uint8_t chip_rev;
    uint8_t part[2];
    uint8_t pbuild;
    uint8_t id[2];
    uint8_t costomer;
    uint8_t romid;
} part_info;

typedef struct int_status {
    uint8_t int_pend;
    uint8_t int_status;
    uint8_t ph_pend;
    uint8_t ph_statsus;
    uint8_t modem_pend;
    uint8_t modem_status;
    uint8_t chip_pend;
    uint8_t chip_status;
} int_status;

typedef struct si446x_reg {
    part_info part_info;
    int_status int_status;
} si446x_reg;
#pragma pack()

typedef struct si446x_device {
    si446x_xfer *xfer;
    si446x_property property;
    struct device parent;
    si446x_reg reg;
} si446x_device;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

void si446x_register(struct si446x_device *si446x, const char *name, si446x_property config, si446x_xfer *xfer);
/*---------- end of file ----------*/
#endif
