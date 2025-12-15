/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : full_duplex_bus.h
 * @Author       : lxf
 * @Date         : 2025-07-22 11:33:21
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-08-02 14:56:12
 * @Brief        :
 */
#ifndef __FULL_DUPLEX_BUS_H__
#define __FULL_DUPLEX_BUS_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "clists.h"
#include "../serial_bus.h"
/*---------- macro ----------*/

/*---------- type define ----------*/

struct full_duplex_bus_handle {
    struct full_duplex_bus_ops *ops;                   // 操作函数
    struct list_head tx_list[SERIAL_BUS_PRIORITY_MAX]; // 发送队列
    uint8_t tx_delay;                                  // 发送延时
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif // !__FULL_DUPLEX_BUS_H__
