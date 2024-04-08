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
 * @FilePath     : simulator_spi.h
 * @Author       : lxf
 * @Date         : 2024-03-27 11:43:14
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-03-27 11:43:50
 * @Brief        :
 */

#ifndef __SIMULATOR_SPI_H__
#define __SIMULATOR_SPI_H__
/*---------- includes ----------*/

#include "stdint.h"
/*---------- macro ----------*/
/*---------- type define ----------*/

/**
 * @brief simulator spi gpio control
 * @return {*}
 */
struct spi_ops {
    /**
     * @brief mosi output
     * @param {uint8_t} state : 0 is low level, 1 is high level
     * @return {*} null
     */
    void (*sdo)(uint8_t state);
    /**
     * @brief miso input
     * @return {int8_t} 0 is low level,1 is high level.
     */
    int8_t (*sdi)(void);
    /**
     * @brief spi clk
     * @param {uint8_t} state
     * @return {*}
     */
    void (*clk)(uint8_t state);
    /**
     * @brief spi delay
     * @param {uint32_t} us
     * @return {*}
     */
    void (*delayus)(uint32_t us);

    uint8_t sckpl; /* 时钟极性 */
    uint8_t sckph; /* 时钟相位 */
};

struct spi_dev_device {
    /**
     * @brief spi cs pin
     * @param {unsigned char} state
     * @return {*}
     */
    void (*cs)(unsigned char state);
    struct spi_ops *ops;
    unsigned char data_width; // 8bit or 16bit or other.
};

typedef struct simulator_spi_message {
    const void *send_buf;      /* message send */
    void *recv_buf;            /* message receive */
    int length;                /* message length */
    unsigned char cs_take : 1; /*  */
    unsigned char cs_release : 1;
} simulator_spi_message;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
int spi_send_recv_without_cs(struct spi_dev_device *spi_dev, const void *send_buff, void *recv_buff, unsigned short data_size);
int spi_send(struct spi_dev_device *spi_dev, const void *send_buff, unsigned short send_size);
int spi_send_recv(struct spi_dev_device *spi_dev, const void *send_buff, void *recv_buff, unsigned short data_size);
void spi_gpio_level_init(struct spi_dev_device *spi);
/*---------- end of file ----------*/
#endif
