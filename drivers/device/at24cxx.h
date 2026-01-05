/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : at24cxx.h
 * @Author       : lxf
 * @Date         : 2024-12-10 08:43:34
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-05-12 08:37:22
 * @Brief        : AT24C系列EEPROM驱动头文件
 * @usage        :
 *               @code
 *               // 使用AT24C_CONFIG宏配置，只需指定型号和地址
 *               static at24cxx_describe_t at24c02 = {
 *                   .config = AT24C_CONFIG(AT24C_TYPE_02, 0xA0),
 *                   .ops = { ... },
 *                   .bus_name = "i2c1",
 *               };
 *               @endcode
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
/*
 * AT24CXX型号选择宏
 * 根据芯片型号选择对应宏，配合AT24C_CONFIG使用
 *
 * | 型号宏      | 容量  | 页大小 | 地址字节 |
 * |-------------|-------|--------|----------|
 * | AT24C_TYPE_01   | 128B  | 8      | 1        |
 * | AT24C_TYPE_02   | 256B  | 8      | 1        |
 * | AT24C_TYPE_04   | 512B  | 16     | 1        |
 * | AT24C_TYPE_08   | 1KB   | 16     | 1        |
 * | AT24C_TYPE_16   | 2KB   | 16     | 1        |
 * | AT24C_TYPE_32   | 4KB   | 32     | 2        |
 * | AT24C_TYPE_64   | 8KB   | 32     | 2        |
 * | AT24C_TYPE_128  | 16KB  | 64     | 2        |
 * | AT24C_TYPE_256  | 32KB  | 64     | 2        |
 * | AT24C_TYPE_512  | 64KB  | 128    | 2        |
 * | AT24C_TYPE_1024 | 128KB | 256    | 2        |
 */
#define AT24C_TYPE_01    1
#define AT24C_TYPE_02    2
#define AT24C_TYPE_04    4
#define AT24C_TYPE_08    8
#define AT24C_TYPE_16    16
#define AT24C_TYPE_32    32
#define AT24C_TYPE_64    64
#define AT24C_TYPE_128   128
#define AT24C_TYPE_256   256
#define AT24C_TYPE_512   512
#define AT24C_TYPE_1024  1024

/* 根据型号获取配置参数的宏 */
#define AT24C_SIZE(type)      ((type) * 128)
#define AT24C_PAGE_SIZE(type) \
    ((type) <= 2  ? 8  : \
     (type) <= 16 ? 16 : \
     (type) <= 64 ? 32 : \
     (type) <= 256 ? 64 : \
     (type) == 512 ? 128 : 256)
#define AT24C_ADDR_BYTES(type) ((type) <= 16 ? 1 : 2)

/*
 * 便捷配置宏 - 用户只需定义型号和设备地址
 * @param type: AT24C_TYPE_XX型号宏
 * @param addr: I2C设备地址(如0xA0)
 * @example: AT24C_CONFIG(AT24C_TYPE_02, 0xA0)
 */
#define AT24C_CONFIG(type, addr) { \
    .ee_size = AT24C_SIZE(type), \
    .ee_page_size = AT24C_PAGE_SIZE(type), \
    .ee_dev_addr = (addr), \
    .ee_addr_btyes = AT24C_ADDR_BYTES(type), \
}

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
