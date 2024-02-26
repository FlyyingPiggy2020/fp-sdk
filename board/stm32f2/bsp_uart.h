/*
 * Copyright (c) 2024 by Moorgen Tech. Co, Ltd.
 * @FilePath     : bsp_uart.h
 * @Author       : lxf
 * @Date         : 2024-02-26 09:32:53
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-02-26 17:01:52
 * @Brief        : usart driver
 */

#ifndef __BSP_UART_H__
#define __BSP_UART_H__
/*---------- includes ----------*/

#include "bsp.h"
/*---------- macro ----------*/

#define USE_USART1 1
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

void bsp_uart_init(void);

void bsp_uart_loop(void);
/*---------- end of file ----------*/
#endif
