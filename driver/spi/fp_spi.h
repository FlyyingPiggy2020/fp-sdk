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
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : fp_spi.h
 * @Author       : lxf
 * @Date         : 2024-07-09 10:38:30
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-07-09 10:38:53
 * @Brief        : This file contains the declarations and documentation for the SPI driver.
 * The SPI driver provides functions for initializing and controlling the SPI peripheral.
 */

#ifndef __FP_SPI_H__
#define __FP_SPI_H__
/*---------- includes ----------*/
/*---------- macro ----------*/
/*---------- type define ----------*/

typedef struct fp_spi_message {
    const void *send_buf;         /**< Pointer to the data buffer to send. */
    void *recv_buf;               /**< Pointer to the data buffer to receive. */
    unsigned short length;        /**< Length of the data buffer in bytes. */
    unsigned char cs_take : 1;    /**< Flag to indicate if the chip select (CS) line should be taked before the transfer. */
    unsigned char cs_release : 1; /**< Flag to indicate if the chip select (CS) line should be released after the transfer. */
} fp_spi_message_t;

typedef struct fp_spi_dev fp_spi_dev_t;

typedef struct fp_spi_bus {
    int (*spi_bus_xfer)(fp_spi_dev_t *spi_dev, fp_spi_message_t *message);
    void *spi_phy;
    unsigned int spi_clock_frequence; /**< if clock frequence of spi dev is different to spi bus need to reconfig spi.*/
    unsigned char ticks;              /**< if ticks of spi dev is different to spi bus need to reconfig spi.*/
    /**
     * @brief The SPI mode for communication.
     *
     * This variable represents the SPI mode for communication. There are four possible SPI modes:
     * 1. Mode 0: CPOL = 0, CPHA = 0
     * 2. Mode 1: CPOL = 0, CPHA = 1
     * 3. Mode 2: CPOL = 1, CPHA = 0
     * 4. Mode 3: CPOL = 1, CPHA = 1
     *
     * The SPI mode determines the clock polarity (CPOL) and clock phase (CPHA) for data transfer.
     *
     * @see https://en.wikipedia.org/wiki/Serial_Peripheral_Interface#Clock_polarity_and_phase
     */
    unsigned int spi_mode; /**< if spi mode of spi dev is different to spi bus need to reconfig spi. */
} fp_spi_bus_t;

/**
 * @brief Structure representing an SPI device.
 *
 * This structure defines the properties of an SPI device that can be connected to an SPI bus.
 * An SPI bus can have multiple SPI devices attached to it.
 */
typedef struct fp_spi_dev {
    void (*cs)(unsigned char state);  /**< Function pointer to control the chip select (CS) pin of the device. */
    fp_spi_bus_t *spi_bus;            /**< Pointer to the SPI bus driver associated with the device. */
    unsigned int spi_clock_frequence; /**< unit:HZ.The clock frequency for SPI communication. */
    /**
     * @brief The SPI mode for communication.
     *
     * This variable represents the SPI mode for communication. There are four possible SPI modes:
     * 1. Mode 0: CPOL = 0, CPHA = 0
     * 2. Mode 1: CPOL = 0, CPHA = 1
     * 3. Mode 2: CPOL = 1, CPHA = 0
     * 4. Mode 3: CPOL = 1, CPHA = 1
     *
     * The SPI mode determines the clock polarity (CPOL) and clock phase (CPHA) for data transfer.
     *
     * @see https://en.wikipedia.org/wiki/Serial_Peripheral_Interface#Clock_polarity_and_phase
     */
    unsigned int spi_mode;
    unsigned int spi_width; /**< The data width for SPI communication. */
} fp_spi_dev_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

int spi_send_then_recv(fp_spi_dev_t *spi_dev, const void *send_buff, unsigned short send_size, void *recv_buff, unsigned short recv_size);
int spi_send_then_send(fp_spi_dev_t *spi_dev, const void *send_buff1, unsigned short send_size1, const void *send_buff2, unsigned short send_size2);
int spi_send_recv(fp_spi_dev_t *spi_dev, const void *send_buff, void *recv_buff, unsigned short data_size);
int spi_send(fp_spi_dev_t *spi_dev, const void *send_buff, unsigned short send_size);

int spi_send_recv_without_cs(fp_spi_dev_t *spi_dev, const void *send_buff, void *recv_buff, unsigned short data_size);
/*---------- end of file ----------*/
#endif
