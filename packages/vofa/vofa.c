/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : vofa.c
 * @Author       : lxf
 * @Date         : 2025-12-29 15:49:33
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-12-29 16:00:00
 * @Brief        : VOFA+调试输出模块实现
 */

/*---------- includes ----------*/
#include "vofa.h"
#include "device.h"
#include <string.h>

/*---------- macro ----------*/

/*---------- type define ----------*/

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

/*---------- variable ----------*/

/* UART设备句柄 */
static device_t *g_vofa_uart = NULL;

/* 发送缓冲区(最大8通道float + NaN帧尾) */
static uint8_t g_tx_buf[VOFA_MAX_CHANNELS * sizeof(float) + sizeof(float)];

/*---------- function ----------*/

/**
 * @Brief  初始化VOFA+模块
 * @param  uart_dev: UART设备句柄
 * @return 0=成功, <0=失败
 */
int vofa_init(device_t *uart_dev)
{
    if (!uart_dev) {
        return -1;
    }
    g_vofa_uart = uart_dev;
    return 0;
}

/**
 * @Brief  发送多通道浮点数据(JustFloat协议)
 * @param  values: 浮点数组指针
 * @param  count: 通道数量(1~8)
 * @return 0=成功, <0=失败
 * @note   数据格式: [float0][float1]...[floatN][NaN帧尾]
 *         NaN帧尾为0x00,0x00,0x80,0x7F
 */
int vofa_send(const float *values, uint8_t count)
{
    uint32_t tail_value;
    uint32_t offset = 0;

    if (!g_vofa_uart || !values || count > VOFA_MAX_CHANNELS) {
        return -1;
    }

    /* 拷贝float数据到缓冲区 */
    memcpy(g_tx_buf, values, count * sizeof(float));
    offset += count * sizeof(float);

    /* 添加NaN帧尾(0x00,0x00,0x80,0x7F) */
    tail_value = VOFA_JUSTFLOAT_TAIL;
    memcpy(&g_tx_buf[offset], &tail_value, sizeof(float));
    offset += sizeof(float);

    device_write(g_vofa_uart, g_tx_buf, 0, offset);

    return 0;
}

/*---------- end of file ----------*/
