/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : serial_bus.h
 * @Author       : lxf
 * @Date         : 2025-08-02 15:03:39
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-01-28 16:00:00
 * @Brief        : 串行总线统一接口
 *
 * @usage       :
 * 本模块提供统一的串行总线接口, 通过 write_buf 的不同实现支持全双工和半双工模式。
 * 框架层完全统一, 无需区分全双工/半双工。
 *
 * **全双工模式** (如 UART、SPI 等硬件支持全双工的接口):
 *   - write_buf 直接发送数据
 *   - 收发独立, 可同时进行
 *   - 示例: USB CDC、SPI Flash
 *
 * **半双工模式** (如 RS485、单线 UART 等需要方向控制的接口):
 *   - write_buf 内部处理:
 *     - RS485 方向控制 (DE/RE引脚)
 *     - 帧间静默期检测 (Inter-frame Gap, 如 3.5字节时间)
 *     - 总线仲裁 (与接收字节的时间戳比较)
 *   - 返回 <0 表示总线忙, 框架层会等待下次 poll 再尝试
 *   - 示例: RS485、单线半双工 UART
 *
 * **接收方式**:
 *   - 统一使用中断接收
 *   - 在接收中断中调用 serial_bus_receive() 将数据交给框架处理
 *
 * @features    :
 *   - 支持优先级队列 (数值越大优先级越高, 支持抢占)
 *   - 支持重发机制
 *   - 支持 ACK 停止重发
 *   - 支持总线忙避让 (write_buf 返回 <0 时不推进发送状态)
 *
 * @example     :
 * @code
 * // 全双工模式示例 (UART)
 * int uart_write(uint8_t *buf, uint16_t len) {
 *     return UART_Send(buf, len);  // 直接发送
 * }
 *
 * // 半双工模式示例 (RS485)
 * static uint32_t last_rx_tick = 0;
 *
 * void uart_rx_isr(uint8_t data) {
 *     last_rx_tick = get_tick();
 *     serial_bus_receive(g_bus, &data, 1);
 * }
 *
 * int rs485_write(uint8_t *buf, uint16_t len) {
 *     // 1. 检查帧间静默期 (3.5字节时间)
 *     if (get_tick() - last_rx_tick < FRAME_GAP_MS) {
 *         return -EBUSY;  // 总线忙
 *     }
 *     // 2. 方向控制
 *     RS485_DE_TX();
 *     // 3. 发送
 *     UART_Send(buf, len);
 *     // 4. 发送完成后在 TC 中断中记录 last_rx_tick
 *     return 0;
 * }
 *
 * // 创建总线
 * struct serial_bus_ops ops = {
 *     .write_buf = uart_write,  // 唯一必需函数
 * };
 * bus = serial_bus_new(&ops, &cb, 10);  // default_delay=10ms
 * @endcode
 */

#ifndef __SERIAL_BUS_H__
#define __SERIAL_BUS_H__
#ifdef __cplusplus
extern "C" {
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>

/*---------- macro ----------*/

/* 优先级定义 (数值越大优先级越高) */
enum {
    SERIAL_BUS_PRIORITY_IDLE = 0,
    SERIAL_BUS_PRIORITY_NORMAL,
    SERIAL_BUS_PRIORITY_REGISTER,
    SERIAL_BUS_PRIORITY_ACK,
    SERIAL_BUS_PRIORITY_MAX
};

#define SERIAL_BUS_TRANS_FOREVER 0xFF /* 永久重发 */

/*---------- type define ----------*/

/**
 * @brief 串行总线操作接口
 * @note  write_buf 是唯一必需的函数
 *        - 半双工模式: write_buf 内部处理忙检测、方向控制、帧间静默期
 *        - 返回 0 表示发送成功, <0 表示总线忙(框架会重试)
 */
struct serial_bus_ops {
    /**
     * @brief 发送数据
     * @param buf: 待发送数据
     * @param len: 数据长度
     * @return 0=成功, <0=失败(如总线忙)
     * @note  半双工模式下需要在此函数内部处理:
     *        - RS485 方向控制 (DE/RE引脚)
     *        - 帧间静默期检测 (Inter-frame Gap)
     *        - 总线仲裁
     */
    int (*write_buf)(uint8_t *buf, uint16_t len);

    /**
     * @brief 可选: 发送延时函数 (如需要软件延时)
     * @param ms: 延时时间(毫秒)
     * @note  如果为 NULL, 框架使用内部计数器实现延时
     */
    void (*send_delay)(uint16_t ms);

    /**
     * @brief 可选: 原子操作锁 (多线程环境)
     * @note  如果为 NULL, 框架不进行加锁保护
     */
    void (*lock)(void);

    /**
     * @brief 可选: 原子操作解锁
     */
    void (*unlock)(void);
};

/**
 * @brief 用户回调函数
 */
struct serial_bus_cb {
    /**
     * @brief 数据接收回调 (在中断上下文调用)
     * @param recv: 接收到的数据
     * @param recv_len: 接收数据长度
     * @param trans: 当前正在发送的数据 (统一为NULL, 保留用于兼容)
     * @param trans_len: 当前发送数据长度 (统一为0, 保留用于兼容)
     * @return: 0=继续处理, >0=停止当前重发(用于ACK场景)
     */
    int (*recv_callback)(uint8_t *recv, uint16_t recv_len, uint8_t *trans, uint16_t trans_len);

    /**
     * @brief 获取延时时间(可选)
     * @param trans_cnt: 当前已发送次数
     * @param trans_max: 最大发送次数
     * @return: 延时时间(ms), 如果为NULL则使用默认延时
     */
    uint16_t (*get_delay_ms)(uint8_t trans_cnt, uint8_t trans_max);
};

/**
 * @brief 串行总线句柄 (不透明指针)
 */
typedef struct serial_bus serial_bus_t;

/*---------- variable prototype ----------*/

/*---------- function prototype ----------*/

/**
 * @brief 创建串行总线实例
 * @param ops: 操作接口 (write_buf 是唯一必需的函数)
 * @param cb: 用户回调函数
 * @param default_delay: 默认发送延时(ms)
 * @return: 总线句柄, 失败返回NULL
 * @note  全双工和半双工通过 write_buf 的不同实现区分, 无需指定模式
 */
serial_bus_t *serial_bus_new(struct serial_bus_ops *ops, struct serial_bus_cb *cb, uint16_t default_delay);

/**
 * @brief 销毁总线实例
 * @param self: 总线句柄
 */
void serial_bus_delete(serial_bus_t *self);

/**
 * @brief 发送数据到缓存
 * @param self: 总线句柄
 * @param buf: 待发送数据
 * @param len: 数据长度
 * @param priority: 优先级 (0~SERIAL_BUS_PRIORITY_MAX-1)
 * @param trans_cnt: 重发次数 (0=不发送, 1=发送1次, SERIAL_BUS_TRANS_FOREVER=永久重发)
 * @return: 0=成功, <0=失败
 */
int serial_bus_send(serial_bus_t *self, const uint8_t *buf, uint16_t len, uint8_t priority, uint8_t trans_cnt);

/**
 * @brief 停止当前重发(收到ACK后调用)
 * @param self: 总线句柄
 */
void serial_bus_stop_retrans(serial_bus_t *self);

/**
 * @brief 接收数据(在中断中调用)
 * @param self: 总线句柄
 * @param buf: 接收到的数据
 * @param len: 数据长度
 * @note  全双工和半双工都使用中断接收模式
 */
void serial_bus_receive(serial_bus_t *self, const uint8_t *buf, uint16_t len);

/**
 * @brief 周期性处理函数(需在定时任务中调用)
 * @param self: 总线句柄
 * @note  处理发送队列和重发逻辑
 */
void serial_bus_poll(serial_bus_t *self);

/**
 * @brief 清空发送队列
 * @param self: 总线句柄
 */
void serial_bus_clear_txlist(serial_bus_t *self);

/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __SERIAL_BUS_H__ */
