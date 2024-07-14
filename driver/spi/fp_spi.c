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
 * @FilePath     : fp_spi.c
 * @Author       : lxf
 * @Date         : 2024-07-09 10:38:25
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-07-09 11:09:08
 * @Brief        : This file contains the implementation of the SPI driver.
 * spi_bus_xfer() is the function that user needs to be implemented.
 */

/*---------- includes ----------*/

#include "fp_spi.h"
#include "stddef.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

int spi_send_then_recv(fp_spi_dev_t *spi_dev, const void *send_buff, unsigned short send_size, void *recv_buff, unsigned short recv_size)
{
    fp_spi_message_t message;

    /* Send data */
    message.send_buf = send_buff;
    message.recv_buf = NULL;
    message.cs_take = 1;
    message.cs_release = 0;
    message.length = send_size;
    spi_dev->spi_bus->spi_bus_xfer(spi_dev, &message);

    /* Receive data */
    message.send_buf = NULL;
    message.recv_buf = recv_buff;
    message.cs_take = 0;
    message.cs_release = 1;
    message.length = recv_size;
    spi_dev->spi_bus->spi_bus_xfer(spi_dev, &message);

    return 0;
}
int spi_send_then_send(fp_spi_dev_t *spi_dev, const void *send_buff1, unsigned short send_size1, const void *send_buff2, unsigned short send_size2)
{
    fp_spi_message_t message;

    /* Send data */
    message.send_buf = send_buff1;
    message.recv_buf = NULL;
    message.cs_take = 1;
    message.cs_release = 0;
    message.length = send_size1;
    spi_dev->spi_bus->spi_bus_xfer(spi_dev, &message);

    /* Send data */
    message.send_buf = send_buff2;
    message.recv_buf = NULL;
    message.cs_take = 0;
    message.cs_release = 1;
    message.length = send_size2;
    spi_dev->spi_bus->spi_bus_xfer(spi_dev, &message);

    return 0;
}
int spi_send_recv(fp_spi_dev_t *spi_dev, const void *send_buff, void *recv_buff, unsigned short data_size)
{
    fp_spi_message_t message;

    /* Send and receive data */
    message.send_buf = send_buff;
    message.recv_buf = recv_buff;
    message.cs_take = 1;
    message.cs_release = 1;
    message.length = data_size;
    spi_dev->spi_bus->spi_bus_xfer(spi_dev, &message);

    return 0;
}

int spi_send(fp_spi_dev_t *spi_dev, const void *send_buff, unsigned short send_size)
{
    fp_spi_message_t message;

    /* Send data */
    message.send_buf = send_buff;
    message.recv_buf = NULL;
    message.cs_take = 1;
    message.cs_release = 1;
    message.length = send_size;
    spi_dev->spi_bus->spi_bus_xfer(spi_dev, &message);

    return 0;
}

int spi_send_recv_without_cs(fp_spi_dev_t *spi_dev, const void *send_buff, void *recv_buff, unsigned short data_size)
{
    fp_spi_message_t message;

    message.length = data_size;
    message.send_buf = send_buff;
    message.recv_buf = recv_buff;
    message.cs_take = 0;
    message.cs_release = 0;
    spi_dev->spi_bus->spi_bus_xfer(spi_dev, &message);
    return 0;
}
/*---------- end of file ----------*/
