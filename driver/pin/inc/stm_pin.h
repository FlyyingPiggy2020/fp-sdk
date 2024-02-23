/*
 * Copyright (c) 2024 by Moorgen Tech. Co, Ltd.
 * @FilePath     : stm_pin.h
 * @Author       : lxf
 * @Date         : 2024-02-22 17:18:04
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-02-23 08:47:56
 * @Brief        : stm32 gpio drvier
 */

#pragma once
/*---------- includes ----------*/

/* in order to add hal gpio drvier to your chip */
#include "fp_def.h"
#include "main.h"

/*---------- macro ----------*/

#define __STM32_PORT(port)  GPIO##port##_BASE
/** it's not applicable to SOC_SERIES_STM32MP1.it is a useage :
 *  #define  led0_pin GET_PIN(B,14)
 */
#define GET_PIN(PORTx, PIN) (uint32_t)((16 * (((uint32_t)__STM32_PORT(PORTx) - (uint32_t)GPIOA_BASE) / (0x0400UL))) + PIN)
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
