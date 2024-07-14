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
#include "../spi/fp_spi.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
typedef struct simulator_spi_ops {
    void (*mosi)(unsigned char state);
    unsigned char (*miso)(void);
    void (*sck)(unsigned char state);
    void (*delay_us)(unsigned int us);
} simulator_spi_ops_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

int simulator_spi_bus_xfer(fp_spi_dev_t *spi_dev, fp_spi_message_t *message);
/*---------- end of file ----------*/
#endif
