/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : stbd_torque_sensor.h
 * @Author       : Lu Xianfan
 * @Date         : 2025-12-26 15:00:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-12-26 16:34:48
 * @Brief        : STBD系列扭矩传感器驱动层 (Modbus RTU协议)
 */

#ifndef __STBD_TORQUE_SENSOR_H__
#define __STBD_TORQUE_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include "MBx_api.h" // ModbusX API

/*---------- macro ----------*/
// STBD扭矩传感器默认配置
#define STBD_TORQUE_DEFAULT_SLAVE_ID  1      // 默认从机ID
#define STBD_TORQUE_DEFAULT_BAUDRATE  115200 // 默认波特率

// STBD扭矩传感器寄存器地址定义 (根据Modbus协议规范)
#define STBD_REG_ADDR_TORQUE_FLOAT    0x0000 // 实时扭矩 (Float32, 小端, 2个寄存器)
#define STBD_REG_ADDR_SPEED_FLOAT     0x0002 // 实时转速 (Float32, 小端, 2个寄存器)
#define STBD_REG_ADDR_PEAK_TORQUE     0x0018 // 扭矩峰值 (Float32)
#define STBD_REG_ADDR_VALLEY_TORQUE   0x001C // 扭矩谷值 (Float32)
#define STBD_REG_ADDR_PEAK_VALLEY_EN  0x0054 // 峰谷检测使能 (Uint16)
#define STBD_REG_ADDR_PEAK_VALLEY_CLR 0x0073 // 清零峰谷值 (写入0x4000)
#define STBD_REG_ADDR_MEASURE_MODE    0x0103 // 测量模式 (0:保持最大, 1:实时更新)
#define STBD_REG_ADDR_PEAK_THRESHOLD  0x0104 // 峰值检测阈值 (Float32)
#define STBD_REG_ADDR_ZERO            0xF004 // 扭矩归零 (写入任意值触发)

/*---------- type define ----------*/

/**
 * @Brief  STBD扭矩传感器设备结构 (协议层 - 包含从机成员和主机引用)
 */
struct stbd_torque_sensor {
    _MBX_MASTER_TEAM_MEMBER MBxMember;
    _MBX_MAP_LIST_ENTRY map_list[8];
    _MBX_MASTER *mbx_master; /* 主机对象指针 */

    /* 浮点数值 (直接使用float, modbusX自动处理字节序转换) */
    float torque_value; /* 实时扭矩值 (单位: Nm) */
    // float speed_value;        /* 实时转速值 (单位: RPM) */
    // float torque_peak;        /* 扭矩峰值 (单位: Nm) */
    // float torque_valley;      /* 扭矩谷值 (单位: Nm) */
    // float peak_threshold;     /* 峰值阈值(单位：Nm) */

    // uint16_t peak_valley_en; /* 峰谷检测使能状态 */
    // uint16_t measure_mode;   /* 测量模式 (0:保持最大, 1:实时更新) */

    /* 从机ID */
    uint8_t slave_id;

    /* 在线状态 */
    uint64_t update_time; /* 上次更新时间戳(ms) */
    bool is_online;
};

/*---------- variable prototype ----------*/
// 无外部变量

/*---------- function prototype ----------*/
/**
 * @Brief  创建STBD扭矩传感器从机成员
 * @param  slave_id: 从机ID (1-247)
 * @return 传感器句柄, NULL表示失败
 */
struct stbd_torque_sensor *stbd_torque_sensor_create(uint8_t slave_id, _MBX_MASTER *mbx_master);

/**
 * @Brief  销毁传感器从机成员
 * @param  sensor: 传感器句柄
 */
void stbd_torque_sensor_destroy(struct stbd_torque_sensor *sensor);

/**
 * @Brief  检查传感器在线状态
 * @param  sensor: 传感器句柄
 * @return true=在线, false=离线
 */
bool stbd_torque_sensor_is_online(struct stbd_torque_sensor *sensor);

/**
 * @Brief  扭矩归零
 * @param  sensor: 传感器句柄
 * @return 0=成功, <0=失败
 */
int stbd_torque_sensor_zero(struct stbd_torque_sensor *sensor);

/**
 * @Brief  设置峰谷检测使能
 * @param  sensor: 传感器句柄
 * @param  enable: true=使能, false=禁用
 * @return 0=成功, <0=失败
 */
int stbd_torque_sensor_set_peak_valley_enable(struct stbd_torque_sensor *sensor, bool enable);

/**
 * @Brief  清零峰谷值
 * @param  sensor: 传感器句柄
 * @return 0=成功, <0=失败
 */
int stbd_torque_sensor_clear_peak_valley(struct stbd_torque_sensor *sensor);

/**
 * @Brief  设置测量模式
 * @param  sensor: 传感器句柄
 * @param  mode: 0=保持最大值, 1=实时更新
 * @return 0=成功, <0=失败
 */
int stbd_torque_sensor_set_measure_mode(struct stbd_torque_sensor *sensor, uint8_t mode);

/**
 * @Brief  默认轮询函数 - 读取扭矩、转速、峰值、谷值
 * @param  sensor: 传感器句柄
 * @return 0=成功, <0=失败
 * @note   内部读取以下寄存器:
 *         - STBD_REG_ADDR_TORQUE_FLOAT (0x0000): 实时扭矩
 *         - STBD_REG_ADDR_SPEED_FLOAT (0x0002): 实时转速
 *         - STBD_REG_ADDR_PEAK_TORQUE (0x0018): 扭矩峰值
 *         - STBD_REG_ADDR_VALLEY_TORQUE (0x001C): 扭矩谷值
 * @note   读取浮点数值示例:
 *         float torque = sensor->torque_value;  // 直接访问, 无需转换
 *         float speed  = sensor->speed_value;
 */
int stbd_torque_sensor_poll_default(struct stbd_torque_sensor *sensor);

/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __STBD_TORQUE_SENSOR_H__ */
