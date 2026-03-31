/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : mgt_abs_sensor.h
 * @Author       : lxf
 * @Date         : 2026-01-09 10:00:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-02-27 08:58:30
 * @Brief        : MGT-ABS 位置传感器驱动层 (UART协议)
 *
 * @hardware     :
 *               - 通信接口: UART
 *               - 波特率: 根据传感器规格配置
 *               - 数据格式: 8N1
 *
 * @usage        :
 *               @code
 *               // 初始化
 *               device_t *uart = device_open("usart1");
 *               struct mgt_abs_sensor sensor;
 *               mgt_abs_sensor_init(&sensor, uart);
 *
 *               // 周期轮询读取位置
 *               mgt_abs_sensor_poll(&sensor);
 *               uint16_t pos = mgt_abs_sensor_get_position(&sensor);
 *               bool is_running = mgt_abs_sensor_is_running(&sensor);
 *               @endcode
 *
 * @transmit_flow:
 *               主机发送: [0x02]
 *               传感器返回: [CF][SF][DF0][DF1][CRC]
 *
 * @note         :
 *               - CF: 命令帧 (0x02=读取位置, 0xBA=进入校准, 0xC2=归零)
 *               - SF: 状态帧 (bit3=1运行, bit3=0校准)
 *               - DF0/DF1: 数据帧 (位置值，小端)
 *               - CRC: CRC8(x^8+1) 校验 CF SF DF0 DF1
 */

#ifndef __MGT_ABS_SENSOR_H__
#define __MGT_ABS_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include "device.h"

/*---------- macro ----------*/
// MGT-ABS 命令定义
#define MGT_ABS_CMD_READ_POSITION   0x02 // 读取单圈绝对位置
#define MGT_ABS_CMD_ENTER_CALIB     0xBA // 进入校准状态 (连续发送10次)
#define MGT_ABS_CMD_ZERO_POSITION   0xC2 // 当前位置归零 (连续发送10次)

// MGT-ABS 响应格式
#define MGT_ABS_RESP_FRAME_SIZE     5 // 响应帧大小: CF SF DF0 DF1 CRC

// MGT-ABS 状态帧位定义
#define MGT_ABS_STATUS_RUNNING_MASK 0x08 // bit3: 运行状态 (1=正常运行, 0=校准)

// MGT-ABS 接收缓冲区大小
#define MGT_ABS_RX_BUF_SIZE         32

// MGT-ABS 离线检测配置
#define MGT_ABS_OFFLINE_THRESHOLD   20 // 离线判定超时次数
#define MGT_ABS_TIMEOUT_INTERVAL_MS 50 // timeout_count增加的时间间隔(ms)

/*---------- type define ----------*/

/**
 * @brief  MGT-ABS 位置传感器结构体
 */
struct mgt_abs_sensor {
    device_t *uart_dev;                  // UART设备句柄
    uint16_t position;                   // 当前位置值
    uint8_t status;                      // 传感器状态
    bool data_valid;                     // 数据是否有效 (CRC校验通过)
    uint8_t rx_buf[MGT_ABS_RX_BUF_SIZE]; // 接收缓冲区
    uint64_t tick_last;                  // 上次发送命令时间
    uint64_t tick_timeout;               // 上次timeout_count增加时间
    uint16_t timeout_count;              // 连续超时计数
    bool is_online;                      // 在线状态
    uint8_t calib_cmd_count;             // 校准命令计数
    uint8_t zero_cmd_count;              // 归零命令计数
};

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

/**
 * @brief  初始化MGT-ABS传感器
 * @param  sensor: 传感器结构体指针
 * @param  uart_dev: UART设备句柄
 * @return 0=成功, <0=失败
 */
int mgt_abs_sensor_init(struct mgt_abs_sensor *sensor, device_t *uart_dev);

/**
 * @brief  轮询读取位置(周期调用)
 * @param  sensor: 传感器结构体指针
 * @return 0=成功, <0=失败
 * @note   内部自动发送命令、接收数据、解析位置
 */
int mgt_abs_sensor_poll(struct mgt_abs_sensor *sensor);

/**
 * @brief  检查传感器在线状态
 * @param  sensor: 传感器结构体指针
 * @return true=在线, false=离线
 */
bool mgt_abs_sensor_is_online(struct mgt_abs_sensor *sensor);

/**
 * @brief  获取当前位置值
 * @param  sensor: 传感器结构体指针
 * @return 当前位置值
 */
uint16_t mgt_abs_sensor_get_position(struct mgt_abs_sensor *sensor);

/**
 * @brief  发送进入校准状态命令
 * @param  sensor: 传感器结构体指针
 * @return 0=成功, <0=失败
 * @note   需要连续发送10次才能进入校准状态
 */
int mgt_abs_sensor_enter_calib(struct mgt_abs_sensor *sensor);

/**
 * @brief  发送位置归零命令
 * @param  sensor: 传感器结构体指针
 * @return 0=成功, <0=失败
 * @note   需要连续发送10次才能归零
 */
int mgt_abs_sensor_zero_position(struct mgt_abs_sensor *sensor);

/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __MGT_ABS_SENSOR_H__ */
