/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : dev_uart.h
 * @Author       : lxf
 * @Date         : 2025-12-24 09:08:24
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-01-22 11:09:29
 * @Brief        : UART设备驱动框架
 * @features     :
 *               - 非阻塞异步收发(基于HAL中断模式)
 *               - 乒乓缓冲区管理(支持连续收发)
 *               - IDLE中断接收(支持变长数据包)
 *               - RS485软件收发切换
 *               - 发送完成自动流控(支持队列发送)
 *
 * @architecture :
 *               ┌─────────────────────────────────────────────┐
 *               │              应用层                          │
 *               │  device_open/write/read/ioctl/close         │
 *               └─────────────────┬───────────────────────────┘
 *                                 │
 *               ┌─────────────────▼───────────────────────────┐
 *               │          dev_uart驱动层                      │
 *               │  - _dev_uart_write   拷贝到txpp              │
 *               │  - _dev_uart_read    从rxpp读取              │
 *               │  - _dev_uart_irq_handler 处理TX/RX事件      │
 *               │  - 接收完成后自动重启接收                    │
 *               └─────────────────┬───────────────────────────┘
 *                                 │
 *               ┌─────────────────▼───────────────────────────┐
 *               │         bsp_uart硬件抽象层                   │
 *               │  - uart_init            HAL初始化            │
 *               │  - uart_start_transmit  启动发送             │
 *               │  - uart_start_receive   启动接收             │
 *               │  - HAL_xxxCallback      中断回调             │
 *               └─────────────────┬───────────────────────────┘
 *                                 │
 *               ┌─────────────────▼───────────────────────────┐
 *               │            STM32 HAL库                      │
 *               └─────────────────────────────────────────────┘
 *
 * ============================================================================
 * @warning  接收模式冲突检测 - 三种接收方式互斥，只能选择一种！
 * ============================================================================
 *
 * UART驱动提供三种接收方式，共享同一个 rxpp 乒乓缓冲，混合使用会导致数据竞争：
 *
 * ┌────────────────┬─────────────────────────────────────────────────────────┐
 * │   接收模式      │                       使用方法                        │
 * ├────────────────┼─────────────────────────────────────────────────────────┤
 * │ STANDARD       │ device_read(uart, buf, 0, len)                         │
 * │ (标准模式)      │ 批量读取，自动释放缓冲区                                │
 * │                │ 适用: 一般应用，简单数据处理                            │
 * ├────────────────┼─────────────────────────────────────────────────────────┤
 * │ ZERO_COPY      │ device_ioctl(uart, IOCTL_UART_GET_RX_BUF, &rx_buf)      │
 * │ (零拷贝模式)    │ process_data(rx_buf.ptr, rx_buf.size)                 │
 * │                │ device_ioctl(uart, IOCTL_UART_RX_DONE, NULL)            │
 * │                │ 适用: 大数据量，回显，DMA转发                           │
 * ├────────────────┼─────────────────────────────────────────────────────────┤
 * │ STREAM         │ device_ioctl(uart, IOCTL_UART_GET_BYTE, &byte)         │
 * │ (流式模式)      │ 逐字节读取，驱动层维护游标，跨包读取                     │
 * │                │ 适用: Modbus/其他需要单字节接收的协议栈                 │
 * └────────────────┴─────────────────────────────────────────────────────────┘
 *
 * 冲突检测机制:
 *   - 首次调用任一接收接口时，自动锁定对应模式
 *   - 后续调用其他模式的接口时，返回 E_BUSY / -1 错误
 *   - 模式锁定状态保存在 config.rx_mode 中
 *
 * 示例:
 *   @code
 *   device_t *uart = device_open("usart2");
 *
 *   // 正确: 只使用 STREAM 模式
 *   uint8_t byte;
 *   device_ioctl(uart, IOCTL_UART_GET_BYTE, &byte);  // 锁定 STREAM 模式
 *
 *   // 错误: 混合使用模式
 *   device_read(uart, buf, 0, len);  // 返回 -1 (模式冲突)
 *   @endcode
 */
#ifndef __DEV_UART_H__
#define __DEV_UART_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "define.h"
#include <stdint.h>
#include <stdbool.h>
#include "device.h"
#include "pingpong.h"
/*---------- macro ----------*/

/* IOCTL 命令定义 */
#define IOCTL_UART_GET_RX_BUF  (IOCTL_USER_START + 0x00) /* 获取接收缓冲区(零拷贝) */
#define IOCTL_UART_GET_TX_BUF  (IOCTL_USER_START + 0x01) /* 获取发送缓冲区(零拷贝) */
#define IOCTL_UART_RX_DONE     (IOCTL_USER_START + 0x02) /* 接收完成(零拷贝模式-释放缓冲区) */
#define IOCTL_UART_TX_DONE     (IOCTL_USER_START + 0x03) /* 发送完成(零拷贝模式-触发发送) */
#define IOCTL_UART_GET_BYTE    (IOCTL_USER_START + 0x04) /* 获取单字节(流式模式-用于协议栈) */

/* UART 标志位 */
#define DEV_UART_FLAG_SWRS485  (1 << 0) /* 软件RS485模式，收发切换由软件控制 */
#define DEV_UART_FLAG_PKT_MODE (1 << 1) /* 使用整包收发模式(IDLE或者DMA)，内部采用乒乓缓冲 */

/*---------- type define ----------*/

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
struct dev_uart_describe; /* 前向声明 */

/**
 * @brief UART 接收模式 (用于 config.rx_mode，三种模式互斥，只能选择一种)
 * @note  三种接收方式共享同一个 rxpp 乒乓缓冲，混合使用会导致数据竞争
 *
 * @verbatim
 * 模式1: UART_RX_MODE_STANDARD - 标准模式(带拷贝，简单易用)
 *         device_read(uart, buf, 0, len)  → 批量读取，自动释放缓冲区
 *         适用场景: 一般应用，简单数据处理
 *
 * 模式2: UART_RX_MODE_ZERO_COPY - 零拷贝模式(高性能)
 *         device_ioctl(uart, IOCTL_UART_GET_RX_BUF, &rx_buf)  → 获取缓冲区
 *         process_data(rx_buf.ptr, rx_buf.size)              → 直接处理
 *         device_ioctl(uart, IOCTL_UART_RX_DONE, NULL)        → 释放缓冲区
 *         适用场景: 大数据量，回显，DMA转发
 *
 * 模式3: UART_RX_MODE_STREAM - 单字节流式模式(协议栈适配)
 *         device_ioctl(uart, IOCTL_UART_GET_BYTE, &byte)      → 逐字节读取
 *         驱动层维护游标，跨包读取
 *         适用场景: Modbus/其他需要单字节接收的协议栈
 * @endverbatim
 *
 * @warning 三种模式互斥，只能选择一种！混合使用会返回 E_BUSY 错误
 */
enum uart_rx_mode {
    UART_RX_MODE_NONE = 0,  /* 未使用接收功能 */
    UART_RX_MODE_STANDARD,  /* 标准模式: device_read() */
    UART_RX_MODE_ZERO_COPY, /* 零拷贝模式: IOCTL_GET_RX_BUF/DONE */
    UART_RX_MODE_STREAM,    /* 流式模式: IOCTL_GET_BYTE */
};

/**
 * @brief UART 发送模式 (用于 config.tx_mode)
 * @note  同步模式会阻塞等待发送完成，异步模式立即返回
 *
 * @verbatim
 * 模式0: UART_TX_MODE_ASYNC - 异步模式(默认，非阻塞)
 *         device_write(uart, buf, 0, len)  → 立即返回，发送在后台进行
 *         适用场景: 一般应用，高并发
 *
 * 模式1: UART_TX_MODE_SYNC - 同步模式(阻塞，等待发送完成)
 *         device_write(uart, buf, 0, len)  → 阻塞等待发送完成
 *         适用场景: RS485需要确保发送完成后再切换回接收模式
 * @endverbatim
 */
enum uart_tx_mode {
    UART_TX_MODE_ASYNC = 0, /* 异步模式: 非阻塞，发送完成后触发中断 */
    UART_TX_MODE_SYNC,      /* 同步模式: 阻塞等待发送完成 */
};

/**
 * @brief UART 硬件控制命令 (ops->control 使用)
 */
enum {
    UART_CTRL_SET_RS485_TX, /* RS485 切换到发送模式 */
    UART_CTRL_SET_RS485_RX, /* RS485 切换到接收模式 */
};

/**
 * @brief UART 中断事件类型 (IRQ Handler 入参)
 */
enum {
    DEV_UART_EVENT_TX_COMPLETE, /* 发送物理完成 (TC) - 所有位发完，用于RS485切向 */
    DEV_UART_EVENT_RX_COMPLETE, /* 接收完成 - IDLE中断或DMA完成 */
    DEV_UART_EVENT_RX_ERROR,    /* 接收错误 - ORE/FE/NE/PE等错误,需要重启接收 */
};

/**
 * @brief UART 缓冲区描述(用于零拷贝模式)
 */
struct dev_uart_buf {
    uint8_t *ptr;  /* 缓冲区指针 */
    uint32_t size; /* 缥冲区大小或数据长度 */
};

/**
 * @brief UART 配置结构体
 */
struct dev_uart_config {
    uint32_t baudrate;         /* 波特率: 9600, 19200, 38400, 57600, 115200等 */
    uint32_t databits;         /* 数据位: 8, 9 */
    uint32_t stopbits;         /* 停止位: 1, 2 */
    uint32_t parity;           /* 校验位: 0=None, 1=Odd, 2=Even */
    uint32_t bufsz_rx;         /* 软件接收FIFO大小 */
    uint32_t bufsz_tx;         /* 软件发送FIFO大小 */
    uint32_t flags;            /* UART标志位: DEV_UART_FLAG_XXX */
    enum uart_rx_mode rx_mode; /* 接收模式: UART_RX_MODE_XXX (默认 STANDARD) */
    enum uart_tx_mode tx_mode; /* 发送模式: UART_TX_MODE_XXX (默认 ASYNC) */
};

/**
 * @brief UART 底层硬件操作接口
 */
struct dev_uart_ops {
    /**
     * @brief 硬件初始化
     * @param self UART设备描述符指针
     * @param cfg 配置参数
     * @return E_OK=成功,其他=失败
     */
    fp_err_t (*init)(struct dev_uart_describe *self, struct dev_uart_config *cfg);

    /**
     * @brief 硬件反初始化
     * @param {dev_uart_describe} *self UART设备描述符指针
     * @return {*}
     */
    void (*deinit)(struct dev_uart_describe *self);

    /**
     * @brief 控制操作(开关中断、RS485方向控制等)
     * @param self UART设备描述符指针
     * @param cmd 控制命令
     * @param arg 命令参数
     * @return E_OK=成功,其他=失败
     */
    fp_err_t (*control)(struct dev_uart_describe *self, uint32_t cmd, void *arg);

    /**
     * @brief 启动同步/异步发送
     * @param self UART设备描述符指针
     * @param buf 要发送的数据缓冲区
     * @param len 发送长度
     * @note 发送完成后会触发DEV_UART_EVENT_TX_COMPLETE事件
     */
    void (*start_transmit)(struct dev_uart_describe *self, uint8_t *buf, uint32_t len);

    /**
     * @brief 启动异步接收
     * @param self UART设备描述符指针
     * @param buf 接收缓冲区
     * @param len 缓冲区大小
     * @return E_OK=成功,其他=失败
     * @note 接收完成后会触发DEV_UART_EVENT_RX_COMPLETE事件
     */
    fp_err_t (*start_receive)(struct dev_uart_describe *self, uint8_t *buf, uint32_t len);
};

struct dev_uart_msg {
    uint32_t pos;
    uint8_t pbuf[];
};

/**
 * @brief UART 设备描述符
 */
struct dev_uart_describe {
    struct dev_uart_config config; /* 配置参数 */
    struct dev_uart_ops *ops;      /* 底层硬件操作接口指针 */
    void *hw_handle;               /* 硬件接口 */
    /* 私有变量不需要用户初始化 */
    struct dev_uart_msg *preveiver;    /* 指向当前接收的 dev_uart_msg */
    struct dev_uart_msg *ptransmitter; /* 指向当前发送的 dev_uart_msg */
    struct pingpong_buffer txpp;       /* 发送乒乓缓冲 */
    struct pingpong_buffer rxpp;       /* 接收乒乓缓冲 */

    /* 流式单字节读取状态 (UART_RX_MODE_STREAM 模式使用) */
    struct {
        struct dev_uart_msg *current_msg; /* 当前正在读取的消息包 */
        uint32_t read_pos;                /* 当前读取位置(游标) */
    } stream;
};

/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
