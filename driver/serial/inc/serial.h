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
 * @FilePath     : serial.h
 * @Author       : lxf
 * @Date         : 2024-03-16 13:24:50
 * @LastEditors  : flyyingpiggy2020 154562451@qq.com
 * @LastEditTime : 2024-03-16 13:24:51
 * @Brief        : serial is a full-dup communication method
 */

#ifndef __SERIAL_H_
#define __SERIAL_H_
/*---------- includes ----------*/

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "fp_sdk.h"
#include "utilities/common/inc/fp_def.h"
/*---------- macro ----------*/

#define SERIAL_DATA_5_BITS    0x0 /*!< word length: 5bits*/
#define SERIAL_DATA_6_BITS    0x1 /*!< word length: 6bits*/
#define SERIAL_DATA_7_BITS    0x2 /*!< word length: 7bits*/
#define SERIAL_DATA_8_BITS    0x3 /*!< word length: 8bits*/

#define SERIAL_STOP_BITS_1    0x1 /*!< stop bit: 1bit*/
#define SERIAL_STOP_BITS_1_5  0x2 /*!< stop bit: 1.5bits*/
#define SERIAL_STOP_BITS_2    0x3 /*!< stop bit: 2bits*/

#define SERIAL_PARITY_DISABLE 0x0 /*!< Disable UART parity*/
#define SERIAL_PARITY_EVEN    0x1 /*!< Enable UART even parity*/
#define SERIAL_PARITY_ODD     0x2 /*!< Enable UART odd parity*/
/*---------- type define ----------*/
struct device_serial {
    struct device parent;
    uint8_t usart_port;
    struct serial_ops *ops;
    const struct serial_config *config;
};

struct serial_config {
    int32_t tx_io_num;
    int32_t rx_io_num;
    int32_t tx_buffer_size;
    int32_t rx_buffer_size;
    uint32_t baud_rate;
    uint8_t data_bits;
    uint8_t parity;
    uint8_t stop_bits;
};

struct serial_ops {
    /**
     * @brief serial read data form fifo
     * @param {device} *device :serial device
     * @param {uint8_t} *buff : output buffer
     * @param {fp_size_t} wanted_size : wanted size
     * @return {*} real size
     */
    fp_size_t (*serial_read)(struct device *device, uint8_t *buff, fp_size_t wanted_size);

    /**
     * @brief serial write data to fifo
     * @param {device} *device : serial device
     * @param {uint8_t} *buff : ouput buffer
     * @param {fp_size_t} size : buff size
     * @return {*} NULL
     */
    void (*serial_write)(struct device *device, uint8_t *buff, fp_size_t size);

    /**
     * @brief serial config
     * @param {device} *device : serial device
     * @param {serial_config} config : config,such as baud rates, data bits ....
     * @return {*}
     */
    fp_size_t (*serial_config)(struct device *device, struct serial_config *config, void (*rxidle_event)(fp_size_t event_size));

    /**
     * @brief rx idle,notify task to read rxbuff.
     * @return {*}
     */
    void (*idle_callback)(fp_size_t event_size);
};

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

int device_serial_register(struct device_serial *serial, const char *name, struct serial_ops *ops);

fp_size_t serial_read(struct device *device, uint8_t *buff, fp_size_t wanted_size);

void serial_write(struct device *device, uint8_t *buff, fp_size_t size);

fp_size_t serial_config(struct device *device, struct serial_config *config, void (*idle_callback)(fp_size_t event_size));
/*---------- end of file ----------*/
#endif