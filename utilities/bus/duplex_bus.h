/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : duplex_bus.h
 * @Author       : lxf
 * @Date         : 2025-07-22 11:54:17
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-07-22 11:55:19
 * @Brief        :
 */

#ifndef __DUPLEX_BUS_H__
#define __DUPLEX_BUS_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
/*---------- macro ----------*/
enum {
    DUPLEX_BUS_PRIORITY_IDLE = 0,
    DUPLEX_BUS_PRIORITY_NORMAL,
    DUPLEX_BUS_PRIORITY_REGISTER,
    DUPLEX_BUS_PRIORITY_ACK,
    DUPLEX_BUS_PRIORITY_MAX
};
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif // !__DUPLEX_BUS_H__
