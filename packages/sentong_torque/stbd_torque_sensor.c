/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : stbd_torque_sensor.c
 * @Author       : Lu Xianfan
 * @Date         : 2025-12-26 15:00:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-12-29 10:56:15
 * @Brief        : STBD系列扭矩传感器驱动层实现 (Modbus RTU协议)
 *
 * @hardware     :
 *               - 通信接口: RS485 (Modbus RTU协议)
 *               - 默认波特率: 115200
 *               - 从机ID范围: 1-247
 *               - 电源: 根据传感器规格供电
 *
 * @usage        :
 *               @code
 *               // ========== 1. 初始化流程 ==========
 *               // 步骤1: 打开UART设备 (RS485接口)
 *               device_t *uart_dev = device_open("usart2");
 *
 *               // 步骤2: 定义Modbus收发缓冲区
 *               static uint8_t rx_buffer[256];
 *               static uint8_t tx_buffer[256];
 *
 *               // 步骤3: 实现Modbus收发回调函数
 *               static uint32_t serial_send(const void *data, size_t len) {
 *                   return device_write(uart_dev, (void *)data, 0, len);
 *               }
 *               static uint32_t serial_getc(uint8_t *byte) {
 *                   // 使用UART流式模式读取单字节
 *                   return device_ioctl(uart_dev, IOCTL_UART_GET_BYTE, byte);
 *               }
 *
 *               // 步骤4: 初始化Modbus RTU主机
 *               _MBX_MASTER mbx_master;
 *               MBx_Master_RTU_Init(&mbx_master,
 *                                   (MBX_SEND_PTR)serial_send,
 *                                   (MBX_GTEC_PTR)serial_getc,
 *                                   115200,
 *                                   rx_buffer, sizeof(rx_buffer),
 *                                   tx_buffer, sizeof(tx_buffer));
 *
 *               // 步骤5: 创建传感器实例 (从机ID=1)
 *               struct stbd_torque_sensor *sensor;
 *               sensor = stbd_torque_sensor_create(1, &mbx_master);
 *               if (sensor == NULL) {
 *                   return -1;
 *               }
 *
 *               // 步骤6: (可选) 配置传感器参数
 *               stbd_torque_sensor_set_measure_mode(sensor, 1);           // 实时更新模式
 *               stbd_torque_sensor_set_peak_valley_enable(sensor, true);  // 使能峰谷检测
 *
 *               // 步骤7: 执行扭矩归零 (传感器空载时)
 *               stbd_torque_sensor_zero(sensor);
 *
 *               // ========== 2. 数据读取 (周期调用) ==========
 *               // 在主循环中周期调用,建议50-100ms调用一次
 *               void main_loop(void) {
 *                   static uint64_t tick_last = 0;
 *
 *                   // 周期性读取数据
 *                   if (get_ticks() - tick_last > 100) {
 *                       if (stbd_torque_sensor_poll_default(sensor) == 0) {
 *                           // 读取成功,数据已更新到sensor结构体
 *                           float torque = sensor->torque_value;      // 实时扭矩 (Nm)
 *                           float speed  = sensor->speed_value;       // 实时转速 (RPM)
 *                           float peak   = sensor->torque_peak;       // 扭矩峰值 (Nm)
 *                           float valley = sensor->torque_valley;     // 扭矩谷值 (Nm)
 *                       }
 *                       tick_last = get_ticks();
 *                   }
 *               }
 *
 *               // ========== 3. 在线状态检测 ==========
 *               if (stbd_torque_sensor_is_online(sensor)) {
 *                   // 传感器在线
 *               } else {
 *                   // 传感器离线或通信失败
 *               }
 *
 *               // ========== 4. 功能操作 ==========
 *               // 扭矩归零 (传感器空载时调用)
 *               stbd_torque_sensor_zero(sensor);
 *
 *               // 使能/禁用峰谷检测
 *               stbd_torque_sensor_set_peak_valley_enable(sensor, true);   // 使能
 *               stbd_torque_sensor_set_peak_valley_enable(sensor, false);  // 禁用
 *
 *               // 清零峰谷值记录
 *               stbd_torque_sensor_clear_peak_valley(sensor);
 *
 *               // 设置测量模式 (0=保持最大值, 1=实时更新)
 *               stbd_torque_sensor_set_measure_mode(sensor, 0);
 *
 *               // ========== 5. 资源释放 ==========
 *               stbd_torque_sensor_destroy(sensor);
 *               device_close(uart_dev);
 *               @endcode
 *
 * @register_map :
 *               ┌─────────┬──────────┬────────────────────────────────┐
 *               │  地址   │   类型   │            说明                 │
 *               ├─────────┼──────────┼────────────────────────────────┤
 *               │ 0x0000  │ Float32  │ 实时扭矩值 (Nm, 小端模式)       │
 *               │ 0x0002  │ Float32  │ 实时转速值 (RPM, 小端模式)      │
 *               │ 0x0018  │ Float32  │ 扭矩峰值 (Nm)                   │
 *               │ 0x001C  │ Float32  │ 扭矩谷值 (Nm)                   │
 *               │ 0x0054  │ Uint16   │ 峰谷检测使能 (0x0008=使能)      │
 *               │ 0x0073  │ Uint16   │ 峰谷值清零 (写入0x4000触发)     │
 *               │ 0x0103  │ Uint16   │ 测量模式 (0=保持最大,1=实时)   │
 *               │ 0x0104  │ Float32  │ 峰值检测阈值 (Nm)               │
 *               │ 0xF004  │ -        │ 扭矩归零 (写入任意值触发)       │
 *               └─────────┴──────────┴────────────────────────────────┘
 *
 * @note         :
 *               - 扭矩归零必须在传感器空载状态下进行
 *               - 峰谷检测需要先使能 (set_peak_valley_enable)
 *               - 通信失败时is_online会被置为false
 *               - 建议在周期性任务(50-100ms)中调用轮询函数
 *               - Float32数据采用小端模式存储
 *               - UART接收需使用流式模式 (IOCTL_UART_GET_BYTE)
 */

/*---------- includes ----------*/
#include "stbd_torque_sensor.h"
#include "MBx_api.h"
#include "options.h"
#include "device.h"
#include <string.h>

/*---------- macro ----------*/
/* (无宏定义) */

/*---------- type define ----------*/
/* (无私有类型定义) */

/*---------- variable prototype ----------*/
// 无私有变量声明

/*---------- function prototype ----------*/
// 无私有函数

/*---------- variable ----------*/
// 无全局变量

/*---------- function ----------*/

/**
 * @Brief  创建STBD扭矩传感器从机成员
 * @param  slave_id: 从机ID (1-247)
 * @return 传感器句柄, NULL表示失败
 */
struct stbd_torque_sensor *stbd_torque_sensor_create(uint8_t slave_id, _MBX_MASTER *mbx_master)
{

    struct stbd_torque_sensor *sensor;
    uint32_t ret;
    if (slave_id < 1 || slave_id > 247) {
        return NULL;
    }

    // 分配传感器结构体内存
    sensor = (struct stbd_torque_sensor *)malloc(sizeof(struct stbd_torque_sensor));
    if (sensor == NULL) {
        return NULL;
    }

    // 清零结构体
    memset(sensor, 0, sizeof(struct stbd_torque_sensor));

    // 保存从机ID和主机指针
    sensor->slave_id = slave_id;
    sensor->mbx_master = mbx_master;

    // 初始化数据缓存
    sensor->is_online = false;

    /* 初始化寄存器映射表(必须按地址升序排列) */
    /* Modbus浮点数字节序: 2EC3 AD3F -> 使用 MBX_REG_TYPE_U32_CD */

    /* 实时扭矩 Float32 (0x0000-0x0001) */
    sensor->map_list[0].Addr = STBD_REG_ADDR_TORQUE_FLOAT;
    sensor->map_list[0].Memory = &sensor->torque_value;
    sensor->map_list[0].Type = MBX_REG_TYPE_U32_DC;
    sensor->map_list[0].Handle = NULL;

    sensor->map_list[1].Addr = STBD_REG_ADDR_TORQUE_FLOAT + 1;
    sensor->map_list[1].Memory = &sensor->torque_value;
    sensor->map_list[1].Type = MBX_REG_TYPE_U32_BA;
    sensor->map_list[1].Handle = NULL;

    /* 映射表结束标记 */
    sensor->map_list[2].Addr = 0;
    sensor->map_list[2].Memory = NULL;
    sensor->map_list[2].Type = 0;
    sensor->map_list[2].Handle = NULL;

    // 添加传感器从机成员到主机
    ret = MBx_Master_Member_Add(mbx_master, &sensor->MBxMember, sensor->slave_id, sensor->map_list);
    if (ret != MBX_API_RETURN_DEFAULT) {
        free(sensor);
        sensor = NULL;
    }
    return sensor;
}

/**
 * @Brief  销毁传感器从机成员
 * @param  sensor: 传感器句柄
 */
void stbd_torque_sensor_destroy(struct stbd_torque_sensor *sensor)
{
    if (sensor != NULL) {
        free(sensor);
    }
}
/**
 * @Brief  检查传感器在线状态
 * @param  sensor: 传感器句柄
 * @return true=在线, false=离线
 */
bool stbd_torque_sensor_is_online(struct stbd_torque_sensor *sensor)
{
    if (sensor == NULL) {
        return false;
    }

    return sensor->is_online;
}

/**
 * @Brief  扭矩归零
 * @param  sensor: 传感器句柄
 * @return 0=成功, <0=失败
 */
int stbd_torque_sensor_zero(struct stbd_torque_sensor *sensor)
{
    uint32_t ret;
    uint16_t zero_data[2] = { 0x0000, 0x0000 };

    if (sensor == NULL || sensor->mbx_master == NULL) {
        return -1;
    }

    ret = MBx_Master_Write_Reg_Mul_Request(
        sensor->mbx_master, sensor->slave_id, STBD_REG_ADDR_ZERO, 2, (uint8_t *)zero_data, 4);

    if (ret != MBX_API_RETURN_DEFAULT) {
        return -1;
    }

    return 0;
}

/**
 * @Brief  设置峰谷检测使能
 * @param  sensor: 传感器句柄
 * @param  enable: true=使能, false=禁用
 * @return 0=成功, <0=失败
 */
int stbd_torque_sensor_set_peak_valley_enable(struct stbd_torque_sensor *sensor, bool enable)
{
    uint32_t ret;
    uint16_t enable_value = enable ? 0x0008 : 0x0000;

    if (sensor == NULL || sensor->mbx_master == NULL) {
        return -1;
    }

    ret =
        MBx_Master_Write_Reg_Request(sensor->mbx_master, sensor->slave_id, STBD_REG_ADDR_PEAK_VALLEY_EN, enable_value);

    if (ret != MBX_API_RETURN_DEFAULT) {
        return -1;
    }

    return 0;
}

/**
 * @Brief  清零峰谷值
 * @param  sensor: 传感器句柄
 * @return 0=成功, <0=失败
 */
int stbd_torque_sensor_clear_peak_valley(struct stbd_torque_sensor *sensor)
{
    uint32_t ret;

    if (sensor == NULL || sensor->mbx_master == NULL) {
        return -1;
    }

    ret = MBx_Master_Write_Reg_Request(sensor->mbx_master, sensor->slave_id, STBD_REG_ADDR_PEAK_VALLEY_CLR, 0x4000);

    if (ret != MBX_API_RETURN_DEFAULT) {
        return -1;
    }

    return 0;
}

/**
 * @Brief  设置测量模式
 * @param  sensor: 传感器句柄
 * @param  mode: 0=保持最大值, 1=实时更新
 * @return 0=成功, <0=失败
 */
int stbd_torque_sensor_set_measure_mode(struct stbd_torque_sensor *sensor, uint8_t mode)
{
    uint32_t ret;

    if (sensor == NULL || sensor->mbx_master == NULL || mode > 1) {
        return -1;
    }

    ret =
        MBx_Master_Write_Reg_Request(sensor->mbx_master, sensor->slave_id, STBD_REG_ADDR_MEASURE_MODE, (uint16_t)mode);

    if (ret != MBX_API_RETURN_DEFAULT) {
        return -1;
    }

    return 0;
}

/**
 * @Brief  默认轮询函数 - 读取扭矩、转速、峰值、谷值
 * @param  sensor: 传感器句柄
 * @return 0=成功, <0=失败
 * @note   内部读取以下寄存器:
 *         - STBD_REG_ADDR_TORQUE_FLOAT (0x0000): 实时扭矩
 *         - STBD_REG_ADDR_SPEED_FLOAT (0x0002): 实时转速
 *         - STBD_REG_ADDR_PEAK_TORQUE (0x0018): 扭矩峰值
 *         - STBD_REG_ADDR_VALLEY_TORQUE (0x001C): 扭矩谷值
 */
int stbd_torque_sensor_poll_default(struct stbd_torque_sensor *sensor)
{
    uint32_t ret;

    if (sensor == NULL || sensor->mbx_master == NULL) {
        return -1;
    }

    // 读取实时扭矩和转速 (0x0000-0x0003, 连续4个寄存器)
    ret = MBx_Master_Read_Reg_Request(sensor->mbx_master, sensor->slave_id, STBD_REG_ADDR_TORQUE_FLOAT, 2);
    if (ret != MBX_API_RETURN_DEFAULT) {
        sensor->is_online = false;
        return -1;
    }

    sensor->is_online = true;
    return 0;
}

/*---------- end of file ----------*/
