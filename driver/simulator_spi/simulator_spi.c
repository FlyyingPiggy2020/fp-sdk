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
#include "simulator_spi.h"
#include "../../utilities/log/log.h"
/*---------- macro ----------*/

#undef LOG_TAG
#define LOG_TAG "simulator_spi"
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

static char ops_spi_send_byte(fp_spi_dev_t *spi_dev, unsigned char data)
{
    assert(spi_dev->spi_mode < 4);
    simulator_spi_ops_t *ops = spi_dev->spi_bus->spi_phy;

    /* frequnece init or change frequence */
    if (spi_dev->spi_bus->ticks == 0 || spi_dev->spi_clock_frequence != spi_dev->spi_bus->spi_clock_frequence) {
        double frequency = spi_dev->spi_bus->spi_clock_frequence;
        double ticks = (1.0 / frequency * 1000000);
        spi_dev->spi_bus->spi_clock_frequence = spi_dev->spi_clock_frequence;
        spi_dev->spi_bus->ticks = (unsigned char)ticks;
        if (spi_dev->spi_bus->ticks == 0) {
            spi_dev->spi_bus->ticks = 1;
        }
    }

    unsigned char tick = spi_dev->spi_bus->ticks;
    unsigned char out_data = 0;

    if (spi_dev->spi_mode == 0) { /* SCKPL = 0; SCKPH = 0 */
		for (unsigned char i = 0; i < 8; i++) {
			if (data & 0x80) {
				ops->mosi(1);
			} else {
				ops->mosi(0);
			}
			data <<= 1;
			out_data <<= 1;
			if (ops->miso()) {
				out_data |= 0x01;
			}
			ops->sck(1);
			ops->delay_us(tick);
			ops->sck(0);
		}
    } else if (spi_dev->spi_mode == 1) { /* SCKPL = 0; SCKPH = 1*/
        for (unsigned char i = 0; i < 8; i++) {
            ops->delay_us(tick);
            ops->sck(1);
            if (data & 0x80) {
                ops->mosi(1);
            } else {
                ops->mosi(0);
            }
            ops->delay_us(tick);
            ops->sck(0);
            if (ops->miso()) {
                out_data |= 0x01;
            }
            data <<= 1;
            if (i < 7) {
                out_data <<= 1;
            }
        }
    } else if (spi_dev->spi_mode == 2) { /* SCKPL = 1; SCKPH = 0*/
        for (unsigned char i = 0; i < 8; i++) {
            ops->delay_us(tick);
            ops->sck(1);
            if (data & 0x80) {
                ops->mosi(1);
            } else {
                ops->mosi(0);
            }
            ops->delay_us(tick);
            ops->sck(0);
            if (ops->miso()) {
                out_data |= 0x01;
            }
            data <<= 1;
            if (i < 7) {
                out_data <<= 1;
            }
        }
    } else if (spi_dev->spi_mode == 3) { /* SCKPL = 1; SCKPH = 1*/
        for (unsigned char i = 0; i < 8; i++) {
            ops->delay_us(tick);
            ops->sck(1);
            if (data & 0x80) {
                ops->mosi(1);
            } else {
                ops->mosi(0);
            }
            ops->delay_us(tick);
            ops->sck(0);
            if (ops->miso()) {
                out_data |= 0x01;
            }
            data <<= 1;
            if (i < 7) {
                out_data <<= 1;
            }
        }
    }
    return out_data;
}

int simulator_spi_bus_xfer(fp_spi_dev_t *spi_dev, fp_spi_message_t *message)
{
    assert(spi_dev->cs);
    assert(spi_dev->spi_bus);
    assert(message);
    int size = message->length;

    if (message->cs_take) {
        spi_dev->cs(0);
    }
    if (spi_dev->spi_width <= 8) {
        const unsigned char *send_ptr = message->send_buf;
        unsigned char *recv_ptr = message->recv_buf;
        while (size--) {
            unsigned char data = 0xff;
            if (send_ptr != NULL) {
                data = *send_ptr++;
            }
            data = ops_spi_send_byte(spi_dev, data);
            if (recv_ptr != NULL) {
                *recv_ptr++ = data;
            }
        }
    }
    return 0;
}
/*---------- end of file ----------*/
