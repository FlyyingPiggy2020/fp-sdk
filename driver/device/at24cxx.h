/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : at24cxx.h
 * @Author       : lxf
 * @Date         : 2024-12-10 08:43:34
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-05-12 08:37:22
 * @Brief        :
 */

#ifndef __AT24CXX__
#define __AT24CXX__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device.h"
/*---------- macro ----------*/

/*---------- type define ----------*/
typedef struct {
    struct {
        uint32_t ee_size;      /* 总容量(字节) */
        uint32_t ee_page_size; /* 页面大小(字节) */
        uint8_t ee_dev_addr;   /* 设备地址 */
        uint8_t ee_addr_btyes; /* 地址字节个数 */
    } config;
    struct {
        /**
         * @brief 用于初始化IO引脚
         * @return {*}
         */
        bool (*init)(void);
        /**
         * @brief 用于反初始化
         * @return {*}
         */
        void (*deinit)(void);
        /**
         * @brief eeprom如果支持写保护，这里操作写保护引脚
         * @param {bool} on :1代表写保护关闭，0代表写保护打开
         * @return {*}
         */
        void (*wp_enable)(bool on); // 如果有wp引脚，这个API操作保护引脚
        /**
         * @brief 如果eeprom的电源可控
         * @param {bool} on 1:代表打开电源，0:代表关闭电源
         * @return {*}
         */
        void (*vcc_enable)(bool on); // 如果设备电源能够控制
        void (*lock)(void *data);
        void (*unlock)(void *data);
    } ops;
    char *bus_name;

    // priv
    void *bus;
} at24cxx_describe_t;

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
