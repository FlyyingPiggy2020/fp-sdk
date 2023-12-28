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
 * Copyright (c) 2023 by Moorgen Tech. Co, Ltd.
 * @FilePath     : a7159.c
 * @Author       : lxf
 * @Date         : 2023-12-28 08:34:47
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2023-12-28 08:35:09
 * @Brief        : a7159
 */

/*---------- includes ----------*/
#include "../../device/core/inc/device_manager.h"
#include "../../utilities/log/inc/log.h"

/*---------- macro ----------*/

#undef LOG_TAG
#define LOG_TAG "a7159"
/*---------- type define ----------*/

typedef enum
{
    REG_SYSTEM_CLOCK = 0X00,
    REG_PLL1 = 0X01,
    REG_PLL2 = 0X02,
    REG_PLL3 = 0X03,
    REG_PLL4 = 0X04,
    REG_PLL5 = 0X05,
    REG_PLL6 = 0X06,
    REG_CRYSTAL = 0X07,
    REG_PAGEA = 0X08,
    REG_PAGEB = 0X09,
    REG_RX1 = 0X0A,
    REG_RX2 = 0X0B,
    REG_ADC = 0X0C,
    REG_PIN_CONTROL = 0X0D,
    REG_CALIBRATION = 0X0E,
    REG_MODE_CONTROL = 0X0F,
} a7159_reg_t;

typedef enum
{
    CMD_WRITE_REG = 0x00,
    CMD_READ_REG = 0x80,
    CMD_WRITE_ID = 0x20,
    CMD_READ_ID = 0xA0,
    CMD_WRITE_TX_FIFO = 0x40,
    CMD_READ_RX_FIFO = 0xC0,
    CMD_RESET_TX_FIFO = 0x60,
    CMD_RESET_RX_FIFO = 0xE0,
    CMD_RESET = 0x70,
    CMD_SLEEP_MODE = 0x10,
    CMD_IDLE_MODE = 0x12,
    CMD_STANDBY_MODE = 0x14,
    CMD_PLL_MODE = 0x16,
    CMD_RX_MODE = 0x18,
    CMD_TX_MODE = 0x1A,
    CMD_DEEP_SLEEP_MODE_TRI_STATE = 0x1C,
    CMD_DEEP_SLEEP_MODE_PULL_HIGH = 0x1F
} a7139_cmd_t;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

/*---------- end of file ----------*/
