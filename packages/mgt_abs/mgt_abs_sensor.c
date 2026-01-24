/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : mgt_abs_sensor.c
 * @Author       : lxf
 * @Date         : 2026-01-09 10:00:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-01-09 09:37:48
 * @Brief        : MGT-ABS 位置传感器驱动层实现
 */

/*---------- includes ----------*/
#include "mgt_abs_sensor.h"
#include "options.h"
#include "crc.h"
#include <string.h>

/*---------- macro ----------*/
#define MGT_ABS_CALIB_CMD_COUNT 10 // 校准命令发送次数
#define MGT_ABS_ZERO_CMD_COUNT  10 // 归零命令发送次数
#define MGT_ABS_CMD_DELAY_MS    1  // 命令发送间隔(要求>600us)

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int mgt_abs_parse_frame(struct mgt_abs_sensor *sensor, const uint8_t *frame);
/*---------- variable ----------*/
/*---------- function ----------*/

/**
 * @brief  解析响应帧
 * @param  sensor: 传感器结构体指针
 * @param  frame: 帧数据指针 (CF SF DF0 DF1 CRC)
 * @return 0=成功, <0=失败
 */
static int mgt_abs_parse_frame(struct mgt_abs_sensor *sensor, const uint8_t *frame)
{
    uint8_t crc_calc;

    // CRC校验: 对 CF SF DF0 DF1 进行校验
    crc_calc = frame[0] ^ frame[1] ^ frame[2] ^ frame[3];

    if (crc_calc != frame[4]) {
        sensor->data_valid = false;
        return -1; // CRC校验失败
    }

    // 保存状态
    sensor->status = frame[1];

    // 解析位置值 (小端模式: DF0 + DF1 << 8)
    sensor->position = frame[2] + (frame[3] << 8);
    sensor->data_valid = true;

    return 0;
}

/**
 * @brief  初始化MGT-ABS传感器
 * @param  sensor: 传感器结构体指针
 * @param  uart_dev: UART设备句柄
 * @return 0=成功, <0=失败
 */
int mgt_abs_sensor_init(struct mgt_abs_sensor *sensor, device_t *uart_dev)
{
    if (sensor == NULL || uart_dev == NULL) {
        return -1;
    }

    memset(sensor, 0, sizeof(struct mgt_abs_sensor));
    sensor->uart_dev = uart_dev;
    sensor->data_valid = false;

    return 0;
}

/**
 * @brief  轮询读取位置(周期调用)
 * @param  sensor: 传感器结构体指针
 * @return 0=成功, <0=失败
 * @note   内部自动发送命令、接收数据、解析位置
 */
int mgt_abs_sensor_poll(struct mgt_abs_sensor *sensor)
{
    uint8_t cmd = MGT_ABS_CMD_READ_POSITION;
    int32_t len;

    if (sensor == NULL || sensor->uart_dev == NULL) {
        return -1;
    }

    // 周期发送读取命令
    if (get_ticks() - sensor->tick_last >= 1) {
        sensor->tick_last = get_ticks();
        device_write(sensor->uart_dev, &cmd, 0, 1);
    }

    // 接收并解析响应
    len = device_read(sensor->uart_dev, sensor->rx_buf, 0, sizeof(sensor->rx_buf));
    if (len >= MGT_ABS_RESP_FRAME_SIZE) {
        // 查找完整帧 (CF SF DF0 DF1 CRC)
        for (int32_t i = 0; i <= len - MGT_ABS_RESP_FRAME_SIZE; i++) {
            if (sensor->rx_buf[i] == MGT_ABS_CMD_READ_POSITION) {
                mgt_abs_parse_frame(sensor, &sensor->rx_buf[i]);
                break;
            }
        }
    }

    return 0;
}

/**
 * @brief  获取当前位置值
 * @param  sensor: 传感器结构体指针
 * @return 当前位置值
 */
uint16_t mgt_abs_sensor_get_position(struct mgt_abs_sensor *sensor)
{
    if (sensor == NULL) {
        return 0;
    }

    return sensor->position;
}

/**
 * @brief  发送进入校准状态命令
 * @param  sensor: 传感器结构体指针
 * @return 0=成功, <0=失败
 * @note   需要连续发送10次才能进入校准状态
 */
int mgt_abs_sensor_enter_calib(struct mgt_abs_sensor *sensor)
{
    uint8_t cmd = MGT_ABS_CMD_ENTER_CALIB;

    if (sensor == NULL || sensor->uart_dev == NULL) {
        return -1;
    }

    // 连续发送10次校准命令，每次间隔>600us
    for (uint8_t i = 0; i < MGT_ABS_CALIB_CMD_COUNT; i++) {
        if (device_write(sensor->uart_dev, &cmd, 0, 1) != 1) {
            return -2;
        }
        delay_ms(MGT_ABS_CMD_DELAY_MS);
    }

    sensor->calib_cmd_count = MGT_ABS_CALIB_CMD_COUNT;

    return 0;
}

/**
 * @brief  发送位置归零命令
 * @param  sensor: 传感器结构体指针
 * @return 0=成功, <0=失败
 * @note   需要连续发送10次才能归零
 */
int mgt_abs_sensor_zero_position(struct mgt_abs_sensor *sensor)
{
    uint8_t cmd = MGT_ABS_CMD_ZERO_POSITION;

    if (sensor == NULL || sensor->uart_dev == NULL) {
        return -1;
    }

    // 连续发送10次归零命令，每次间隔>600us
    for (uint8_t i = 0; i < MGT_ABS_ZERO_CMD_COUNT; i++) {
        if (device_write(sensor->uart_dev, &cmd, 0, 1) != E_OK) {
            return -2;
        }
        delay_ms(MGT_ABS_CMD_DELAY_MS);
    }

    sensor->zero_cmd_count = MGT_ABS_ZERO_CMD_COUNT;

    return 0;
}

/*---------- end of file ----------*/
