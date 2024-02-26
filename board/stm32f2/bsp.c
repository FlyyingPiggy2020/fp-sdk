/*
 * Copyright (c) 2024 by Moorgen Tech. Co, Ltd.
 * @FilePath     : bsp.c
 * @Author       : lxf
 * @Date         : 2024-02-26 09:43:37
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-02-26 09:47:50
 * @Brief        : bsp板级驱动
 */

/*---------- includes ----------*/

#include "bsp.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

/**
 * @brief 需要放在CubeMX自动生成的函数后面
 * @return {*}
 */
void bsp_init(void)
{
    bsp_uart_init();
}
/*---------- end of file ----------*/
