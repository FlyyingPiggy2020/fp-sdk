/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : components/fp-sdk/utilities/crc.c
 * @Author       : lxf
 * @Date         : 2026-01-09 10:00:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-01-09 10:00:00
 * @Brief        : 通用 CRC 校验工具实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "crc.h"

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

/**
 * @brief  CRC8 计算
 * @param  data: 数据指针
 * @param  len: 数据长度
 * @param  poly: 生成多项式
 * @param  init: 初始值
 * @return CRC8 校验值
 */
uint8_t crc8_calc(const uint8_t *data, uint32_t len, uint8_t poly, uint8_t init)
{
    uint8_t crc = init;
    uint8_t i;

    if (data == NULL) {
        return 0;
    }

    while (len--) {
        crc ^= *data++;
        for (i = 0; i < 8; i++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ poly;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

/**
 * @brief  CRC16 计算
 * @param  data: 数据指针
 * @param  len: 数据长度
 * @param  poly: 生成多项式
 * @param  init: 初始值
 * @return CRC16 校验值
 */
uint16_t crc16_calc(const uint8_t *data, uint32_t len, uint16_t poly, uint16_t init)
{
    uint16_t crc = init;
    uint8_t i;

    if (data == NULL) {
        return 0;
    }

    while (len--) {
        crc ^= (uint16_t)(*data++) << 8;
        for (i = 0; i < 8; i++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ poly;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

/**
 * @brief  CRC16-Modbus 计算 (常用)
 * @param  data: 数据指针
 * @param  len: 数据长度
 * @return CRC16-Modbus 校验值
 * @note   多项式 0x8005, 初始值 0xFFFF
 */
uint16_t crc16_modbus(const uint8_t *data, uint32_t len)
{
    return crc16_calc(data, len, CRC16_POLY_MODBUS, CRC16_INIT_MODBUS);
}

/**
 * @brief  CRC16-CCITT 计算
 * @param  data: 数据指针
 * @param  len: 数据长度
 * @return CRC16-CCITT 校验值
 * @note   多项式 0x1021, 初始值 0xFFFF
 */
uint16_t crc16_ccitt(const uint8_t *data, uint32_t len)
{
    return crc16_calc(data, len, CRC16_POLY_CCITT, CRC16_INIT_CCITT);
}

/*---------- end of file ----------*/
