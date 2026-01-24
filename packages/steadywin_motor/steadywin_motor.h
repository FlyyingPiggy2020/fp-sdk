/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : steadywin_motor.h
 * @Author       : lxf
 * @Date         : 2025-12-16 09:49:02
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-12-19 16:24:08
 * @Brief        :
 */
#ifndef _STEADYWIN_MOTOR_H_
#define _STEADYWIN_MOTOR_H_

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include "dev_can.h"
#include "options.h"

/*---------- macro ----------*/
#ifndef SW_MAX_INSTANCES
#define SW_MAX_INSTANCES 8
#endif

#ifndef SW_CMD_QUEUE_SIZE
#define SW_CMD_QUEUE_SIZE 8
#endif

// --- 辅助宏：方便用户定义表格 ---
#define SW_CFG_CONF_F32(_id, _val) \
    {                              \
        SW_OP_WRITE_CONF_F32, _id, \
        {                          \
            .f = _val              \
        }                          \
    }
#define SW_CFG_CONF_I32(_id, _val) \
    {                              \
        SW_OP_WRITE_CONF_I32, _id, \
        {                          \
            .i = _val              \
        }                          \
    }
#define SW_CFG_PARAM(_id, _val) \
    {                           \
        SW_OP_WRITE_PARAM, _id, \
        {                       \
            .u = _val           \
        }                       \
    }

/* 配置类指令 */
#define SW_CMD_RESET_CONF   0x81 // 重置配置 (恢复默认值)
#define SW_CMD_REFRESH_CONF 0x82 // 刷新配置 (使修改立即生效)
#define SW_CMD_WRITE_CONF   0x83 // 修改配置项
#define SW_CMD_READ_CONF    0x84 // 获取配置项

/* 控制类指令 */
#define SW_CMD_START        0x91 // 启动电机 (Enable)
#define SW_CMD_STOP         0x92 // 停止电机 (Disable)
#define SW_CMD_TORQUE_CTRL  0x93 // 力矩控制模式
#define SW_CMD_SPEED_CTRL   0x94 // 速度控制模式
#define SW_CMD_POS_CTRL     0x95 // 位置控制模式
#define SW_CMD_STOP_CTRL    0x97 // 中止当前控制 (保持Enable但停止动作)

/* 参数类指令 */
#define SW_CMD_WRITE_PARAM  0xA1 // 修改运行参数 (如PID实时调整)
#define SW_CMD_READ_PARAM   0xA2 // 获取运行参数

/* 状态与信息类指令 */
#define SW_CMD_GET_VER      0xB1 // 获取固件版本
#define SW_CMD_GET_ERROR    0xB2 // 获取异常状态码
#define SW_CMD_CLEAR_ERROR  0xB3 // 清除异常 (复位错误状态)
#define SW_CMD_GET_METRIC   0xB4 // 获取特定指标 (如母线电压、功率等)

// /* 系统类指令 */
// #define SW_CMD_ENTER_UPGRADE 0xC1 // 进入固件升级模式 (Bootloader)
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

/*---------- Enums ----------*/

// 业务状态
typedef enum {
    SW_STATE_RESET = 0,         // 复位
    SW_STATE_USER_INITIALIZING, // 用户参数设置
    SW_STATE_INITIALIZING,      // 初始化序列执行中 (同步静态参数)
    SW_STATE_RUNNING,           // 运行中 (控制 + 间隙轮询监控)
    SW_STATE_ERROR,             // 严重故障
} sw_logic_state_t;

// 异步事件
typedef enum {
    SW_EVT_USER_INIT_DONE = 0, // 用户设置完成
    SW_EVT_INIT_DONE,          // 初始化完成，静态参数已同步
    SW_EVT_UPDATE,             // 数据更新 (反馈/指标/异常)
    SW_EVT_TIMEOUT,            // 通信超时
    SW_EVT_ERROR,              // 协议或硬件错误
} sw_event_t;

// 缓存的所有电机数据 (Rich Model)
typedef struct {
    // 来源: 监控轮询 (0xB4)
    uint8_t fault_flags; // 异常掩码

    // --- 状态 (代表电机当前运行的情况) ---
    struct {
        // 来源: 控制指令反馈 (0x9x)
        float temp; // 当前温度 (C)
        float pos;  // 当前位置 (Rad)
        float spd;  // 当前速度 (Rad/s)
        float tor;  // 当前力矩 (Nm)

        // 来源: 监控轮询 (0xB2)
        float v_bus;       // 母线电压 (V)
        float iq;          // iq电流（A）
        float id;          // id电流（A）
        float shaft_angle; // 输出轴角度
    } state;

    // --- 配置 (仅初始化读取一次) ---
    struct {
        // 来源: 0xB1
        uint32_t fw_version; // 硬件版本号
        // 来源: 0x84 (Config)
        int32_t pole_pair;      // 极对数
        int32_t rated_curr;     // 额定电流
        int32_t max_speed;      // 最大转速(RPM)
        int32_t rated_volt;     // 额定电压
        int32_t pwm_freq;       // PWM频率(HZ)
        int32_t cur_deafult_kp; // 电流环默认kp
        int32_t cur_deafult_ki; // 电流环默认ki
        int32_t spd_deafult_kp; // 速度环默认kp
        int32_t spd_deafult_ki; // 速度环默认ki
        int32_t pos_deafult_kp; // 位置环默认kp
        int32_t pos_deafult_ki; // 位置环默认ki
        int32_t pos_deafult_kd; // 位置环默认kd
        int32_t gear;           // 减速比
        int32_t motor_canid;    // 电机CANID
        int32_t pc_canid;       // 上位机CANID
        int32_t zero_pos;       // 零点位置（输出轴）
        int32_t pd_pos;         // 断电位置（输出轴）
        int32_t over_voltage;   // 过压值
        int32_t low_voltage;    // 欠压值
        int32_t can_baud;       // can波特率
        int32_t fw_default_kp;  // 弱磁kp Flux weak
        int32_t fw_default_ki;  // 弱磁ki
        int32_t over_temp;      // 过温告警阈值
        int32_t protocol_type;  // 0 SteadyWin；协议; 1 MIT协议
        float p2p_resistance;   // 相间电阻 ohm
        float p2p_inductance;   // 相间电感 H
        float emf_constant;     // 反电势常数(vrms/krpm)
        float kt;               // 转矩常数（N*M*A）
        float samp_resistance;  // 采样电阻 ohm
        float samp_gain;        // 采样运放增益
    } config;

    // --- 参数 (仅初始化读取一次) ---
    struct {
        // 来源: 0xA2 (Param)
        uint32_t cur_kp; // 电流环kp
        uint32_t cur_ki; // 电流环ki
        uint32_t spd_kp; // 速度环kp
        uint32_t spd_ki; // 速度环ki
        uint32_t pos_kp; // 位置环kp
        uint32_t pos_ki; // 位置环ki
        uint32_t pos_kd; // 位置环kd
        uint32_t fw_kp;  // 弱磁kp
        uint32_t fw_ki;  // 弱磁ki
    } param;
} sw_cache_t;

// 初始化操作类型
typedef enum {
    SW_OP_WRITE_CONF_F32, // 写入浮点配置 (CMD 0x83 Type=1)
    SW_OP_WRITE_CONF_I32, // 写入整型配置 (CMD 0x83 Type=0)
    SW_OP_WRITE_PARAM,    // 写入运行参数 (CMD 0xA1)
} sw_init_op_t;

// 单个配置项描述
typedef struct {
    sw_init_op_t op; // 操作类型
    uint8_t id;      // ConfID 或 ParamID
    union {
        float f;
        int32_t i;
        uint32_t u;
        uint8_t b[4];
    } val;
} sw_init_config_t;

/**
 * @brief 内部发送指令结构
 * @return {*}
 */
typedef struct {
    uint8_t data[8];
} sw_job_t;

// 前置声明
typedef struct sw_motor_s sw_motor_t;
typedef void (*sw_callback_t)(sw_motor_t *motor, sw_event_t evt, uint32_t payload);

// 电机句柄
struct sw_motor_s {
    // [电机配置]
    device_t *can_dev;
    uint32_t motor_id;
    uint32_t send_id;
    const sw_init_config_t *user_init_table;
    uint8_t user_init_len;

    // [驱动业务逻辑]
    sw_logic_state_t logic_state;
    uint8_t user_init_step;  // 用户设置进度
    uint8_t init_step;       // 初始化序列进度
    uint64_t last_poll_time; // 上次发送监控查询的时间
    uint8_t monitor_idx;     // 监控轮询索引

    // [底层CAN数据同步发送]
    volatile uint8_t driver_state; // 0:IDLE, 1:BUSY
    uint64_t tx_timestamp;
    uint32_t timeout_setting;
    uint8_t pending_cmd;
    uint8_t pending_byte1; // 记录子指令ID (ConfID/ParamID/IndID)
    uint8_t pending_byte2;

    // [发送FIFO]
    sw_job_t queue[SW_CMD_QUEUE_SIZE];
    uint8_t q_head;
    uint8_t q_tail;
    uint8_t q_count;

    // [全量数据缓存]
    sw_cache_t cache;

    // [用户私有回调函数]
    sw_callback_t cb;
    void *user_data;

    // [System]
    bool is_registered;
};

/*---------- API ----------*/

void sw_motor_init(sw_motor_t *motor, device_t *can, const sw_init_config_t *cfg_table, uint8_t cfg_count);
void sw_motor_set_cb(sw_motor_t *motor, sw_callback_t cb, void *user_data);

// 主循环调用
void sw_bus_poll(device_t *can_dev, uint8_t max_packets);
void sw_motor_process(sw_motor_t *motor);

// 控制接口（只有在SW_STATE_RUNNING时才能够被调用）
/**
 * @brief 从当前力矩变化到目标力矩
 * @param {sw_motor_t} *motor
 * @param {float} torque 目标力矩
 * @param {uint32_t} ms 变化的时间
 * @return {*}
 */
int sw_motor_set_torque(sw_motor_t *motor, float torque, uint32_t ms);
/**
 * @brief 从当前速度变化到目标速度
 * @param {sw_motor_t} *motor
 * @param {float} speed 目标速度
 * @param {uint32_t} ms 变化的时间
 * @return {*}
 */
int sw_motor_set_speed(sw_motor_t *motor, float speed, uint32_t ms);
/**
 * @brief 从当前位置变化到目标位置
 * @param {sw_motor_t} *motor
 * @param {float} pos 目标位置
 * @param {uint32_t} ms 变化的时间
 * @return {*}
 */
int sw_motor_set_pos(sw_motor_t *motor, float pos, uint32_t ms);
int sw_motor_start(sw_motor_t *motor);
int sw_motor_stop(sw_motor_t *motor);
int sw_motor_clear_err(sw_motor_t *motor);

/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif