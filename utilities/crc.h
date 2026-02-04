/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : components/fp-sdk/utilities/crc.h
 * @Author       : lxf
 * @Date         : 2026-01-09 10:00:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-01-09 10:00:00
 * @Brief        : 通用 CRC 校验工具
 *
 * @usage        :
 *               @code
 *               // CRC16-Modbus
 *               uint16_t crc = crc16_modbus(data, len);
 *
 *               // 自定义参数 CRC16
 *               uint16_t crc = crc16_calc(data, len, 0x8005, 0xFFFF);
 *
 *               // 自定义参数 CRC8
 *               uint8_t crc = crc8_calc(data, len, 0x07, 0x00);
 *               @endcode
 */

#ifndef __CRC_H__
#define __CRC_H__

#ifdef __cplusplus
extern "C" {
#endif

/*---------- includes ----------*/
#include <stdint.h>

/*---------- macro ----------*/
// 常用 CRC 多项式
#define CRC8_POLY_MAXIM   0x31 // x^8 + x^5 + x^4 + 1
#define CRC8_POLY_DEFAULT 0x07 // x^8 + x^2 + x + 1

#define CRC16_POLY_IBM    0x8005 // x^16 + x^15 + x^2 + 1
#define CRC16_POLY_CCITT  0x1021 // x^16 + x^12 + x^5 + 1
#define CRC16_POLY_MODBUS 0x8005 // 同 IBM

// CRC16 初始值
#define CRC16_INIT_MODBUS 0xFFFF
#define CRC16_INIT_CCITT  0xFFFF

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

/**
 * @brief  CRC8 计算
 * @param  data: 数据指针
 * @param  len: 数据长度
 * @param  poly: 生成多项式
 * @param  init: 初始值
 * @return CRC8 校验值
 */
uint8_t crc8_calc(const uint8_t *data, uint32_t len, uint8_t poly, uint8_t init);

/**
 * @brief  CRC16 计算
 * @param  data: 数据指针
 * @param  len: 数据长度
 * @param  poly: 生成多项式
 * @param  init: 初始值
 * @return CRC16 校验值
 */
uint16_t crc16_calc(const uint8_t *data, uint32_t len, uint16_t poly, uint16_t init);

/**
 * @brief  CRC16-Modbus 计算 (常用)
 * @param  data: 数据指针
 * @param  len: 数据长度
 * @return CRC16-Modbus 校验值
 * @note   多项式 0x8005, 初始值 0xFFFF
 */
uint16_t crc16_modbus(const uint8_t *data, uint32_t len);

/**
 * @brief  CRC16-CCITT 计算
 * @param  data: 数据指针
 * @param  len: 数据长度
 * @return CRC16-CCITT 校验值
 * @note   多项式 0x1021, 初始值 0xFFFF
 */
uint16_t crc16_ccitt(const uint8_t *data, uint32_t len);

/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __CRC_H__ */
