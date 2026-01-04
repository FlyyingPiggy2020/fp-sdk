/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : vofa.h
 * @Author       : lxf
 * @Date         : 2025-12-29 15:49:33
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-12-29 15:49:33
 * @Brief        : VOFA+调试输出模块
 * @features     :
 *               - 支持JustFloat协议(原始浮点数据流 + NaN帧尾)
 *               - 最多支持8通道浮点数据输出
 *               - 非阻塞发送,基于UART设备
 *
 * @usage        :
 *               @code
 *               // 初始化
 *               device_t *uart = device_open("usart3");
 *               vofa_init(uart);
 *
 *               // 发送多通道数据
 *               float data[4] = {1.0f, 2.0f, 3.0f, 4.0f};
 *               vofa_send(data, 4);
 *               @endcode
 *
 * @warning      VOFA+需配置:
 *               - 协议: JustFloat
 *               - 波特率: 与UART配置一致(默认115200)
 */

#ifndef __VOFA_H__
#define __VOFA_H__

#ifdef __cplusplus
extern "C" {
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include "device.h"
/*---------- macro ----------*/

/* 最大通道数 */
#ifndef VOFA_MAX_CHANNELS
#define VOFA_MAX_CHANNELS 8
#endif

/* JustFloat协议帧尾(NaN值: 0x00,0x00,0x80,0x7F) */
#define VOFA_JUSTFLOAT_TAIL 0x7F800000U /* NaN作为帧尾标记 */

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

/**
 * @Brief  初始化VOFA+模块
 * @param  uart_dev: UART设备句柄
 * @return 0=成功, <0=失败
 */
int vofa_init(device_t *uart_dev);

/**
 * @Brief  发送多通道浮点数据
 * @param  values: 浮点数组指针
 * @param  count: 通道数量(1~8)
 * @return 0=成功, <0=失败
 */
int vofa_send(const float *values, uint8_t count);

/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __VOFA_H__ */
