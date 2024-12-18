/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : simple_mobus_485_port.h
 * @Author       : lxf
 * @Date         : 2024-12-16 17:21:19
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-12-18 10:18:28
 * @Brief        :
 */

#ifndef __SIMPLE_MOBUS_485_PORT_H__
#define __SIMPLE_MOBUS_485_PORT_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "simple_mobus.h"
#include "half_duplex_bus.h"
/*---------- macro ----------*/

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
simple_mobus_describe_t *simple_mobnus_485_port_init(uint8_t addr, void *args);

/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif // !__HALF_DUPLEX_BUS_PORT_H__
