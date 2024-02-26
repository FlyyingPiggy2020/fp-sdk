/*
 * Copyright (c) 2024 by Moorgen Tech. Co, Ltd.
 * @FilePath     : bsp_uart.c
 * @Author       : lxf
 * @Date         : 2024-02-26 09:32:44
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-02-26 17:31:45
 * @Brief        : 1.如果处理不过来,覆盖原先的数据
 *                 2.单次接收不能超过默认的buff大小
 */

/*---------- includes ----------*/

#include "bsp.h"
#include "fp_sdk.h"
/*---------- macro ----------*/

#if (1 == USE_USART1)
#define USART1_RX_BUF_SIZE 10
#endif
/*---------- type define ----------*/

/*---------- variable prototype ----------*/

#if (1 == USE_USART1)
uint8_t g_uart1_rx_buf0[USART1_RX_BUF_SIZE];
uint8_t g_uart1_rx_buf1[USART1_RX_BUF_SIZE];

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef  hdma_usart1_rx;
struct pingpong_buffer    uart1_pingpong_handler;
#endif
/*---------- function prototype ----------*/
/*---------- variable ----------*/

/*---------- function ----------*/

/**
 * @brief 需要配合CubeMX使用，在CubeMX自动生成的串口初始化函数调用之后，再调用该函数。
 * @return {*}
 */
void bsp_uart_init(void)
{
    void *pingpong[1] = { 0 };
    /**
     * @brief 初始化乒乓buffer
     * @return {*}
     */
    pingpong_buffer_init(&uart1_pingpong_handler, g_uart1_rx_buf0, g_uart1_rx_buf1);

    /**
     * @brief 获取写入地址的指针
     * @return {*}
     */
    pingpong_buffer_get_write_buf(&uart1_pingpong_handler, pingpong);
    /**
     * @brief 开启DMA接收和IDIE中断
     * @return {*}
     */
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, pingpong[0], USART1_RX_BUF_SIZE);
}

/**
 * @brief 在主循环中调用
 * @return {*}
 */
void bsp_uart_loop(void)
{
    void    *tempbuf[1];
    uint32_t size;
    if (pingpong_buffer_get_read_buf(&uart1_pingpong_handler, tempbuf, &size)) {

        //        HAL_Delay(3000); // 延时，用于模拟耗时的操作
        // 这里要用阻塞发送，否则的话需要在发送完成中断里面调用pingpong_buffer_set_read_done,不然的话可能这个待发送的buf会被DMA修改。
        HAL_UART_Transmit(&huart1, tempbuf[0], size, 0XFFFF);
        pingpong_buffer_set_read_done(&uart1_pingpong_handler);
    }
}
/**
 * @brief 空闲中断回调函数
 * @param {UART_HandleTypeDef} *huart
 * @param {uint16_t} Size
 * @return {*}
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    void *pingpong[1] = { 0 };
    if (huart->Instance == USART1) {
        // 当Size和USART1_RX_BUF_SIZE一样的时候，我测试下来不会触发TC，反而是触发HT。
        if (huart->RxEventType == HAL_UART_RXEVENT_IDLE || huart->ReceptionType == HAL_UART_RXEVENT_TC || Size == USART1_RX_BUF_SIZE) {
            pingpong_buffer_set_write_done(&uart1_pingpong_handler, Size);
            pingpong_buffer_get_write_buf(&uart1_pingpong_handler, pingpong);
            HAL_UARTEx_ReceiveToIdle_DMA(&huart1, pingpong[0], USART1_RX_BUF_SIZE);
        }
    }
}
/*---------- end of file ----------*/
