/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : steadywin_motor.c
 * @Author       : lxf
 * @Date         : 2025-12-16 09:51:41
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-01-07 14:44:21
 * @Brief        : steadywin电机驱动文件
 *
 * 该电机经过本人测试有3个大坑
 * 注1：不要在CAN驱动中调用0X81重置配置，该电机重置配置后，需要重新校准相序和编码器。但是CAN协议中没有给出校准的接口。
 * 注2：实测位置控制指令的duration字节序和手册里的描述相反的。（仅位置控制相反，速度控制力矩控制是正常的）
 * 注3：配置的选项，电机回复应答大概需要50ms，如果期间发送多条重复指令，会导致该电机出BUG。
 * 注4: 位置指令的duration需要大于50ms，否则电机会忽略该指令。
 * 注5: 位置如果超过12会变为-12
 *
 */

/*---------- includes ----------*/
#include "steadywin_motor.h"
#include <string.h>

/*---------- macro ----------*/
#define sw_get_tick_ms(ms) get_ticks_from_isr(ms)

/* Steadywin电机数据收发 */
#define STATE_IDLE         0
#define STATE_BUSY         1

#define INIT_STEPS         (sizeof(g_init_table) / sizeof(sw_cmd_t))
#define MON_STEPS          (sizeof(g_monitor_table) / sizeof(sw_cmd_t))

// 用户初始化配置重试间隔
// GIM系列电机经测试对于设置类的命令，执行时间大概在50ms左右浮动。
// 本人测试如果按照2ms的间隔发送，电机设置大概需要50ms，期间超时20多次。这个操作会让电机出bug。
// bug的现象是：这个电机之后的所有命令，fifo都偏移了。所以这里选择100ms。
// 建议不要比50ms小。
#ifndef SW_USER_CONF_RETRY_MS
#define SW_USER_CONF_RETRY_MS 100
#endif

// 初始化重试间隔
// 本人测试对于读指令，1ms的时间电机完全够回复了（看时间戳大概是100us左右）
#ifndef SW_INIT_RETRY_MS
#define SW_INIT_RETRY_MS 1
#endif

// 运行时的监控轮询间隔 (即"间隙"的大小)
// 只要 IDLE 且距离上次查询超过这个时间，就插空查询
#ifndef SW_MONITOR_INTERVAL_MS
#define SW_MONITOR_INTERVAL_MS 1
#endif

// 默认超时时间
#ifndef SW_OUTTIME_MS
#define SW_OUTTIME_MS 100
#endif

// 浮点数计算是否使用浮点数加速，该选项开启后，内部的浮点数运算（主要在_parse_fb中）
// 将会采用外部库加速，而不是使用默认的C标准库
#ifndef SW_F_TYPE
#define SW_F_TYPE 0 // 默认的标准库
// #define SW_F_TYPE 1 //使用qflib-m0-full
// #define SW_F_TYPE 2 //使用用户自定义的库，同时_parse_fb变成弱定义
#endif
/*---------- type define ----------*/
/**
 * @brief steadywin协议命令列表
 * 该协议的下发的命令形式为cmd+sub_id(部分命令有sub_id)
 * @return {*}
 */
typedef struct {
    uint8_t cmd;
    uint8_t byte1; // ConfType
    uint8_t byte2; // ConfID 或 ParamID
} sw_cmd_t;

/**
 * @brief IEEE浮点数转化联合体
 * @return {*}
 */
typedef union {
    float f;
    int32_t i;
    uint32_t u;
    uint8_t b[4];
} sw_bytes_t;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/**
 * @brief 电机上电后，会根据初始化命令列表里面的命令初始化设备
 * @return {*}
 */
static const sw_cmd_t g_init_table[] = {
    { SW_CMD_CLEAR_ERROR, 0x00, 0X00 }, // 清除异常
    /* 0xB1 电机版本号 */
    { SW_CMD_GET_VER, 0x00, 0X00 }, // 获取固件版本
    /* 0x84 电机配置 参考PDF第7页 */
    { SW_CMD_READ_CONF, 0x00, 0X00 }, // 极对数
    { SW_CMD_READ_CONF, 0x00, 0X01 }, // 额定电流
    { SW_CMD_READ_CONF, 0x00, 0X02 }, // 最大转速
    { SW_CMD_READ_CONF, 0x00, 0X06 }, // 额定电压
    { SW_CMD_READ_CONF, 0x00, 0X07 }, // PWM频率
    { SW_CMD_READ_CONF, 0x00, 0X08 }, // 电流环默认Kp
    { SW_CMD_READ_CONF, 0x00, 0X09 }, // 电流环默认Ki
    { SW_CMD_READ_CONF, 0x00, 0X0C }, // 速度环默认Kp
    { SW_CMD_READ_CONF, 0x00, 0X0D }, // 速度环默认Ki
    { SW_CMD_READ_CONF, 0x00, 0X0E }, // 位置环默认Kp
    { SW_CMD_READ_CONF, 0x00, 0X0F }, // 位置环默认Ki
    { SW_CMD_READ_CONF, 0x00, 0X10 }, // 位置环默认Kd
    { SW_CMD_READ_CONF, 0x00, 0X11 }, // 减速比
    { SW_CMD_READ_CONF, 0x00, 0X12 }, // CANID
    { SW_CMD_READ_CONF, 0x00, 0X13 }, // 上位机CANID
    { SW_CMD_READ_CONF, 0x00, 0X14 }, // 零点位置输出轴
    { SW_CMD_READ_CONF, 0x00, 0X15 }, // 断电位置输出轴
    { SW_CMD_READ_CONF, 0x00, 0X16 }, // 过压门限值（V）
    { SW_CMD_READ_CONF, 0x00, 0X17 }, // 低压门限值（V）
    { SW_CMD_READ_CONF, 0x00, 0X18 }, // CAN波特率
    { SW_CMD_READ_CONF, 0x00, 0X19 }, // 弱磁默认Kp
    { SW_CMD_READ_CONF, 0x00, 0X1A }, // 弱磁默认Ki
    { SW_CMD_READ_CONF, 0x00, 0X20 }, // 过温告警阈值
    { SW_CMD_READ_CONF, 0x00, 0X1C }, // CAN协议类型
    { SW_CMD_READ_CONF, 0x01, 0X00 }, // 相间电阻（Ω）
    { SW_CMD_READ_CONF, 0x01, 0X01 }, // 相间电感（H）
    { SW_CMD_READ_CONF, 0x01, 0X02 }, // 反电动势常数（Vrms/kRPM）
    { SW_CMD_READ_CONF, 0x01, 0X03 }, // 转矩常数（N*m/A）
    { SW_CMD_READ_CONF, 0x01, 0X04 }, // 采样电阻阻值（Ω）
    { SW_CMD_READ_CONF, 0x01, 0X05 }, // 采样放大增益
    /* 0xA2 参数项 参考PDF第11页 ParaID */
    { SW_CMD_READ_PARAM, 0x00, 0X00 }, // 电流环Kp
    { SW_CMD_READ_PARAM, 0x01, 0X00 }, // 电流环Ki
    { SW_CMD_READ_PARAM, 0x02, 0X00 }, // 速度环Kp
    { SW_CMD_READ_PARAM, 0x03, 0X00 }, // 速度环Ki
    { SW_CMD_READ_PARAM, 0x04, 0X00 }, // 位置环Kp
    { SW_CMD_READ_PARAM, 0x05, 0X00 }, // 位置环Ki
    { SW_CMD_READ_PARAM, 0x06, 0X00 }, // 位置环Kd
    { SW_CMD_READ_PARAM, 0x07, 0X00 }, // 弱磁Kp
    { SW_CMD_READ_PARAM, 0x08, 0X00 }, // 弱磁Ki
    /* 0X91 启动电机 */
    { SW_CMD_START, 0X00, 0X00 }, // 启动电机
};

/**
 * @brief 电机运行后，会监控monitor列表里面的数据
 * @return {*}
 */
static const sw_cmd_t g_monitor_table[] = {
    { SW_CMD_GET_ERROR, 0x00, 0X00 },  // 获取异常
    { SW_CMD_GET_METRIC, 0x09, 0X00 }, // iq电流(A)
};

static sw_motor_t *g_instances[SW_MAX_INSTANCES] = { 0 };

/*---------- function ----------*/

static void _trigger_cb(sw_motor_t *m, sw_event_t evt, uint32_t val)
{
    if (m->cb) {
        m->cb(m, evt, val);
    }
}

/** @brief 指令入队 */
static int _queue_push(sw_motor_t *m, uint8_t *payload)
{
    if (m->q_count >= SW_CMD_QUEUE_SIZE)
        return E_FULL; // 队列溢出

    sw_job_t *job = &m->queue[m->q_tail];
    if (payload)
        memcpy(job->data, payload, 8);
    else
        memset(job->data, 0, 8);

    m->q_tail = (m->q_tail + 1) % SW_CMD_QUEUE_SIZE;
    m->q_count++;
    return E_OK;
}

/** @brief 指令出队并实际发送 */
static int _queue_pop_and_send(sw_motor_t *m)
{
    if (m->q_count == 0)
        return E_EMPTY;

    sw_job_t *job = &m->queue[m->q_head];

    if (m->driver_state == STATE_BUSY) {
        // 超时释放锁，防止死锁
        if ((sw_get_tick_ms() - m->tx_timestamp) >= m->timeout_setting) {
            m->driver_state = STATE_IDLE;
            _trigger_cb(m, SW_EVT_TIMEOUT, m->pending_cmd);
        } else {
            return E_BUSY; // Busy
        }
    }

    struct can_msg tx = { 0 };
    tx.id = m->send_id;
    tx.dlc = 8;
    // 控制指令的 payload 填充
    memcpy(tx.data, job->data, 8);
    if (device_write(m->can_dev, &tx, NULL, NULL) != E_OK) {
        return E_ERROR;
    }

    m->pending_cmd = tx.data[0];
    m->pending_byte1 = tx.data[1];
    m->pending_byte2 = tx.data[2];
    m->tx_timestamp = sw_get_tick_ms();
    m->timeout_setting = SW_OUTTIME_MS;
    m->driver_state = STATE_BUSY;
    m->q_head = (m->q_head + 1) % SW_CMD_QUEUE_SIZE;
    m->q_count--;
    return E_OK;
}

/*---------- Data Parsing & Cache Update ----------*/
#if (0 == SW_F_TYPE)
// 解析 0x9x 反馈 (更新 Feedback 动态数据)
static void _parse_fb(sw_motor_t *m, uint8_t *d)
{
    m->cache.state.temp = (float)d[2];
    uint16_t p = (uint16_t)d[3] | ((uint16_t)d[4] << 8);
    m->cache.state.pos = (float)p * 25.0f / 65535.0f - 12.5f;
    uint16_t s = ((uint16_t)d[5] << 4) | ((d[6] & 0xF0) >> 4);
    uint16_t t = ((uint16_t)(d[6] & 0x0F) << 8) | d[7];
    m->cache.state.spd = (float)s * 130.0f / 4095.0f - 65.0f;
    float k = m->cache.config.kt * m->cache.config.gear;
    m->cache.state.tor = (float)t * (450.0f * k) / 4095.0f - (225.0f * k);
}
#elif (1 == SW_F_TYPE)
#include "qfplib-m0-full.h" // 确保包含了 qfplib 的头文件

// 预定义常量 (编译器会自动计算好这些值，运行时直接作为浮点立即数使用)
// 避免在运行时调用 qfp_fdiv，极大地提高速度
static const float K_POS_FACTOR = 25.0f / 65535.0f; // 0.0003814...
static const float K_POS_OFFSET = 12.5f;
static const float K_SPD_FACTOR = 130.0f / 4095.0f; // 0.031746...
static const float K_SPD_OFFSET = 65.0f;
static const float K_TOR_FACTOR_BASE = 450.0f / 4095.0f; // 0.10989...
static const float K_TOR_OFFSET_BASE = 225.0f;

static void _parse_fb(sw_motor_t *m, uint8_t *d)
{
    // --- 1. 温度 (Temp) ---
    m->cache.state.temp = qfp_uint2float(d[2]);

    // --- 2. 位置 (Position) ---
    uint16_t p = (uint16_t)d[3] | ((uint16_t)d[4] << 8);
    float p_f = qfp_uint2float(p);
    float p_scaled = qfp_fmul(p_f, K_POS_FACTOR);
    m->cache.state.pos = qfp_fsub(p_scaled, K_POS_OFFSET);

    // --- 3. 速度 (Speed) ---
    uint16_t s = ((uint16_t)d[5] << 4) | ((d[6] & 0xF0) >> 4);
    float s_f = qfp_uint2float(s);
    float s_scaled = qfp_fmul(s_f, K_SPD_FACTOR);
    m->cache.state.spd = qfp_fsub(s_scaled, K_SPD_OFFSET);

    // --- 4. 转矩 (Torque) ---
    uint16_t t = ((uint16_t)(d[6] & 0x0F) << 8) | d[7];
    // 步骤 A: 计算总系数 k
    float k = qfp_fmul(m->cache.config.kt, m->cache.config.gear);
    float slope = qfp_fmul(K_TOR_FACTOR_BASE, k);
    float offset = qfp_fmul(K_TOR_OFFSET_BASE, k);

    // 计算最终 Torque = t_float * slope - offset
    float t_f = qfp_uint2float(t);
    float t_val = qfp_fmul(t_f, slope);
    m->cache.state.tor = qfp_fsub(t_val, offset);
}
#elif (2 == SW_F_TYPE)
#endif
// 解析 0x84 配置读取 (更新 Config 静态数据)
static void _parse_conf(sw_motor_t *m, uint8_t *d)
{
    // [84][Type][ID][RES][D0]...
    uint8_t type = d[1];
    uint8_t id = d[2];

    sw_bytes_t v;
    v.b[0] = d[4];
    v.b[1] = d[5];
    v.b[2] = d[6];
    v.b[3] = d[7];

    // 根据 ID 映射到结构体成员
    if (type == 0x00) {
        switch (id) {
            case 0x00:
                m->cache.config.pole_pair = v.i;
                break;
            case 0x01:
                m->cache.config.rated_curr = v.i;
                break;
            case 0x02:
                m->cache.config.max_speed = v.i;
                break;
            case 0x06:
                m->cache.config.rated_volt = v.i;
                break;
            case 0x07:
                m->cache.config.pwm_freq = v.i;
                break;
            case 0x08:
                m->cache.config.cur_deafult_kp = v.i;
                break;
            case 0x09:
                m->cache.config.cur_deafult_ki = v.i;
                break;
            case 0x0C:
                m->cache.config.spd_deafult_kp = v.i;
                break;
            case 0x0D:
                m->cache.config.spd_deafult_ki = v.i;
                break;
            case 0x0E:
                m->cache.config.pos_deafult_kp = v.i;
                break;
            case 0x0F:
                m->cache.config.pos_deafult_ki = v.i;
                break;
            case 0x10:
                m->cache.config.pos_deafult_kd = v.i;
                break;
            case 0x11:
                m->cache.config.gear = v.i;
                break;
            case 0x12:
                m->cache.config.motor_canid = v.i;
                break;
            case 0x13:
                m->cache.config.pc_canid = v.i;
                break;
            case 0x14:
                m->cache.config.zero_pos = v.i;
                break;
            case 0x15:
                m->cache.config.pd_pos = v.i;
                break;
            case 0x16:
                m->cache.config.over_voltage = v.i;
                break;
            case 0x17:
                m->cache.config.low_voltage = v.i;
                break;
            case 0x18:
                m->cache.config.can_baud = v.i;
                break;
            case 0x19:
                m->cache.config.fw_default_kp = v.i;
                break;
            case 0x1A:
                m->cache.config.fw_default_ki = v.i;
                break;
            case 0x20:
                m->cache.config.over_temp = v.i;
                break;
            case 0x1C:
                m->cache.config.protocol_type = v.i;
                break;
        }
    } else {
        switch (id) {
            case 0x00:
                m->cache.config.p2p_resistance = v.f;
                break;
            case 0x01:
                m->cache.config.p2p_inductance = v.f;
                break;
            case 0x02:
                m->cache.config.emf_constant = v.f;
                break;
            case 0x03:
                m->cache.config.kt = v.f;
                break;
            case 0x04:
                m->cache.config.samp_resistance = v.f;
                break;
            case 0x05:
                m->cache.config.samp_gain = v.f;
                break;
            default:
                break;
        }
    }
}

// 解析 0xA2 参数读取 (更新 Param 静态数据)
static void _parse_param(sw_motor_t *m, uint8_t *d)
{
    uint8_t id = d[1];
    sw_bytes_t v;
    v.b[0] = d[4];
    v.b[1] = d[5];
    v.b[2] = d[6];
    v.b[3] = d[7];
    switch (id) {
        case 0x00:
            m->cache.param.cur_kp = v.u;
            break;
        case 0x01:
            m->cache.param.cur_ki = v.u;
            break;
        case 0x02:
            m->cache.param.spd_kp = v.u;
            break;
        case 0x03:
            m->cache.param.spd_ki = v.u;
            break;
        case 0x04:
            m->cache.param.pos_kp = v.u;
            break;
        case 0x05:
            m->cache.param.pos_ki = v.u;
            break;
        case 0x06:
            m->cache.param.pos_kd = v.u;
            break;
        case 0x07:
            m->cache.param.fw_kp = v.u;
            break;
        case 0x08:
            m->cache.param.fw_ki = v.u;
            break;
        default:
            break;
    }
}

// 解析0XB4 指标项
static void _parse_mmetric(sw_motor_t *m, uint8_t *d)
{
    uint8_t id = d[1];
    sw_bytes_t v;
    v.b[0] = d[4];
    v.b[1] = d[5];
    v.b[2] = d[6];
    v.b[3] = d[7];
    switch (id) {
        case 0x00:
            m->cache.state.v_bus = v.f;
            break;
        case 0x09:
            m->cache.state.iq = v.f;
            break;
        case 0x0A:
            m->cache.state.id = v.f;
            break;
        case 0x13:
            m->cache.state.shaft_angle = v.f;
            break;
        default:
            break;
    }
}
// 核心接收解析
static bool _feed_packet(sw_motor_t *m, struct can_msg *msg)
{
    if (m->driver_state != STATE_BUSY)
        return false;
    if (msg->id != m->motor_id)
        return false;
    if (msg->data[0] != m->pending_cmd)
        return false;

    // 协议错误检查

    uint8_t res_idx;
    bool is_return = false;

    // 不同命令的RES字节不一样,匹配的逻辑不一样
    switch (m->pending_cmd) {
        case SW_CMD_WRITE_CONF:
        case SW_CMD_READ_CONF:
            // 这几类指令的RES在byte3
            res_idx = 3;
            if (msg->data[1] != m->pending_byte1 && msg->data[2] != m->pending_byte2) {
                is_return = true;
            }
            break;

        case SW_CMD_WRITE_PARAM:
        case SW_CMD_READ_PARAM:
        case SW_CMD_GET_METRIC:
            // 这几类指令的RES在byte2
            if (msg->data[1] != m->pending_byte1) {
                is_return = true;
            }
            res_idx = 2;
            break;

        default:
            // 其余的都在byte1
            res_idx = 1;
            break;
    }

    if (is_return) {
        return false;
    }

    // 立即上报异常
    if (m->pending_cmd != SW_CMD_GET_ERROR && msg->data[res_idx] != 0x00) {
        m->driver_state = STATE_IDLE;
        _trigger_cb(m, SW_EVT_ERROR, msg->data[res_idx]);
        return true;
    }

    // 数据归档
    switch (m->pending_cmd) {
        case SW_CMD_START:
            if (m->logic_state == SW_STATE_INITIALIZING) {
                m->init_step++;
            }
            break;
        case SW_CMD_WRITE_CONF:
        case SW_CMD_WRITE_PARAM:
            if (m->logic_state == SW_STATE_USER_INITIALIZING) {
                m->user_init_step++;
            }
            break;
        case SW_CMD_TORQUE_CTRL:
        case SW_CMD_SPEED_CTRL:
        case SW_CMD_POS_CTRL:
            _parse_fb(m, msg->data);
            break;

        case SW_CMD_GET_VER: {
            sw_bytes_t v;
            memcpy(v.b, &msg->data[4], 4);
            m->cache.config.fw_version = v.u;
            if (m->logic_state == SW_STATE_INITIALIZING) {
                m->init_step++;
            }
        } break;

        case SW_CMD_READ_CONF:
            _parse_conf(m, msg->data);
            // 如果处于初始化阶段，每收到一个回复，步数+1
            if (m->logic_state == SW_STATE_INITIALIZING) {
                m->init_step++;
            }
            break;

        case SW_CMD_READ_PARAM:
            _parse_param(m, msg->data);
            if (m->logic_state == SW_STATE_INITIALIZING) {
                m->init_step++;
            }
            break;

        case SW_CMD_CLEAR_ERROR:
            if (m->logic_state == SW_STATE_INITIALIZING) {
                m->init_step++;
            }
            break;

        case SW_CMD_GET_ERROR:
            m->cache.fault_flags = msg->data[2];
            if (m->logic_state == SW_STATE_INITIALIZING) {
                m->init_step++;
            }
            break;

        case SW_CMD_GET_METRIC:
            _parse_mmetric(m, msg->data);
            if (m->logic_state == SW_STATE_INITIALIZING) {
                m->init_step++;
            }
            break;
    }
    m->driver_state = STATE_IDLE;
    _trigger_cb(m, SW_EVT_UPDATE, 0); // 通知用户数据已更新
    return true;
}

/*---------- Public API ----------*/

/**
 * @brief 电机控制层状态机
 * 1.电机上电后，读取所有参数。
 * 2.空闲时不停的读取监控数据。
 * @param {sw_motor_t} *m
 * @return {*}
 */
void sw_motor_process(sw_motor_t *m)
{
    // 1. 驱动层维护 (检查超时)
    if (m->driver_state == STATE_BUSY) {
        if ((sw_get_tick_ms() - m->tx_timestamp) >= m->timeout_setting) {
            m->driver_state = STATE_IDLE;
            _trigger_cb(m, SW_EVT_TIMEOUT, m->pending_cmd);
        }
        return; // 正忙，无法发送新指令
    }

    // --- 以下逻辑仅在驱动 IDLE 时执行 ---
    // 2. 优先级最高：处理用户队列 (解决连续调用 API 问题)
    if (m->q_count > 0) {
        _queue_pop_and_send(m);
        return; // 发送完立即退出，等待回包
    }

    // 3. 优先级中等：GIM系列电机默认执行逻辑
    uint64_t now = sw_get_tick_ms();
    if (m->logic_state == SW_STATE_USER_INITIALIZING) {
        if (now - m->last_poll_time < SW_USER_CONF_RETRY_MS) {
            return;
        }
        if (m->user_init_table == NULL || !m->user_init_len) {
            m->logic_state = SW_STATE_INITIALIZING;
            _trigger_cb(m, SW_EVT_USER_INIT_DONE, 0);
        } else {
            if (m->user_init_step < m->user_init_len) {
                const sw_init_config_t *cfg = &m->user_init_table[m->user_init_step];
                uint8_t d[8] = { 0 };
                if (cfg->op == SW_OP_WRITE_CONF_F32) {
                    // 0X83 0X01
                    d[0] = SW_CMD_WRITE_CONF;
                    d[1] = 0x01;
                    d[2] = cfg->id;
                } else if (cfg->op == SW_OP_WRITE_CONF_I32) {
                    // 0x83 0x00
                    d[0] = SW_CMD_WRITE_CONF;
                    d[1] = 0x00;
                    d[2] = cfg->id;
                } else if (cfg->op == SW_OP_WRITE_PARAM) {
                    // 0xA1
                    d[0] = SW_CMD_WRITE_PARAM;
                    d[1] = cfg->id;
                }
                d[4] = cfg->val.b[0];
                d[5] = cfg->val.b[1];
                d[6] = cfg->val.b[2];
                d[7] = cfg->val.b[3];
                if (_queue_push(m, d) == E_OK) {
                    m->last_poll_time = now;
                }
            } else {
                m->logic_state = SW_STATE_INITIALIZING;
                _trigger_cb(m, SW_EVT_USER_INIT_DONE, 0);
            }
        }

    } else if (m->logic_state == SW_STATE_INITIALIZING) {
        // 电机初始化，读取所有参数
        if (now - m->last_poll_time < SW_INIT_RETRY_MS) {
            return;
        }

        if (m->init_step < INIT_STEPS) {
            sw_cmd_t t = g_init_table[m->init_step];
            uint8_t d[8] = { 0 };
            d[0] = t.cmd;
            d[1] = t.byte1;
            d[2] = t.byte2;
            if (_queue_push(m, d) == E_OK) {
                m->last_poll_time = now;
            }
        } else {
            // 序列跑完了
            m->logic_state = SW_STATE_RUNNING;
            _trigger_cb(m, SW_EVT_INIT_DONE, 0);
        }
    } else if (m->logic_state == SW_STATE_RUNNING) {
        // 间隙轮询：利用空闲时间更新动态数据
        // 规则：只要空闲时间超过 MONITOR_INTERVAL_MS，就插一条查询
        // 发送数据，不会影响 last_poll_time (监控计时)
        // 但因为发送后状态变 BUSY，sw_motor_process 会自动暂停监控发送
        // 等到回复回来变 IDLE，且时间满足间隔后，监控继续
        if (now - m->last_poll_time > SW_MONITOR_INTERVAL_MS) {
            sw_cmd_t t = g_monitor_table[m->monitor_idx];
            uint8_t d[8] = { 0 };
            d[0] = t.cmd;
            d[1] = t.byte1;
            d[2] = t.byte2;
            if (_queue_push(m, d) == E_OK) {
                m->last_poll_time = now;
                m->monitor_idx = (m->monitor_idx + 1) % MON_STEPS;
            }
        }
    }
}
void sw_motor_set_cb(sw_motor_t *m, sw_callback_t cb, void *user_data)
{
    m->cb = cb;
    m->user_data = user_data;
}

/**
 * @brief 电机初始化
 * @param {sw_motor_t} *m 控制句柄
 * @param {device_t} *can can总线句柄
 * @param {uint32_t} motor_id 电机的can id
 * @param {uint32_t} send_id  本机发送的can id
 * @param {float} kt 转矩常数
 * @param {float} gear 减速比
 * @return {*}
 */
void sw_motor_init(sw_motor_t *motor, device_t *can, const sw_init_config_t *cfg_table, uint8_t cfg_count)
{
    memset(motor, 0, sizeof(sw_motor_t));
    motor->can_dev = can;
    motor->motor_id = 0;
    motor->send_id = 1;
    motor->user_init_table = cfg_table;
    motor->user_init_len = cfg_count;

    // 启动即进入初始化序列
    motor->logic_state = SW_STATE_USER_INITIALIZING;
    motor->init_step = 0;
    motor->last_poll_time = sw_get_tick_ms() + SW_INIT_RETRY_MS; // 立即发送

    // 注册
    if (!motor->is_registered) {
        for (int i = 0; i < SW_MAX_INSTANCES; i++) {
            if (!g_instances[i]) {
                g_instances[i] = motor;
                motor->is_registered = true;
                break;
            }
        }
    }

    // 停止电机
    uint8_t d[8] = { 0 };
    d[0] = SW_CMD_STOP;
    _queue_push(motor, d);
}

/**
 * @brief 以总线为轮询主体，防止单条总线上挂在多个电机，数据被偷吃的现象。
 * 设置单次轮询最大的read次数，防止can负载过大时，CPU完全被CAN接收逻辑占用。
 * @param {device_t} *can can总线
 * @param {uint8_t} max_packets
 * @return {*}
 */
void sw_bus_poll(device_t *can, uint8_t max_packets)
{
    struct can_msg msg;
    uint8_t cnt = 0;
    while (cnt < max_packets && device_read(can, &msg, NULL, NULL) == E_OK) {
        for (int i = 0; i < SW_MAX_INSTANCES; i++) {
            if (g_instances[i] && g_instances[i]->can_dev == can) {
                if (_feed_packet(g_instances[i], &msg))
                    break;
            }
        }
        cnt++;
    }
}

// 示例控制函数 (简写)
static int _ctrl(sw_motor_t *motor, uint8_t cmd, float val, uint32_t ms)
{
    sw_bytes_t v;
    v.f = val;
    uint8_t d[8] = { 0 };

    d[0] = cmd;
    d[1] = v.b[0];
    d[2] = v.b[1];
    d[3] = v.b[2];
    d[4] = v.b[3];
    if (cmd == SW_CMD_POS_CTRL) {
        d[5] = (uint8_t)(ms & 0xFF);         // Low
        d[6] = (uint8_t)((ms >> 8) & 0xFF);  // Mid
        d[7] = (uint8_t)((ms >> 16) & 0xFF); // High
    } else {
        d[7] = (uint8_t)(ms & 0xFF);         // Low
        d[6] = (uint8_t)((ms >> 8) & 0xFF);  // Mid
        d[5] = (uint8_t)((ms >> 16) & 0xFF); // High
    }
    return _queue_push(motor, d);
}

int sw_motor_set_speed(sw_motor_t *motor, float speed, uint32_t ms)
{
    if (motor->logic_state != SW_STATE_RUNNING) {
        return E_ERROR;
    }
    return _ctrl(motor, SW_CMD_SPEED_CTRL, speed, ms);
}

int sw_motor_set_torque(sw_motor_t *motor, float torque, uint32_t ms)
{
    if (motor->logic_state != SW_STATE_RUNNING) {
        return E_ERROR;
    }
    return _ctrl(motor, SW_CMD_TORQUE_CTRL, torque, ms);
}
int sw_motor_set_pos(sw_motor_t *motor, float pos, uint32_t ms)
{
    if (motor->logic_state != SW_STATE_RUNNING) {
        return E_ERROR;
    }
    return _ctrl(motor, SW_CMD_POS_CTRL, pos, ms);
}

int sw_motor_start(sw_motor_t *motor)
{
    if (motor->logic_state != SW_STATE_RUNNING) {
        return E_ERROR;
    }
    uint8_t d[8] = { 0 };
    d[0] = SW_CMD_START;
    return _queue_push(motor, d);
}
int sw_motor_stop(sw_motor_t *motor)
{
    if (motor->logic_state != SW_STATE_RUNNING) {
        return E_ERROR;
    }
    uint8_t d[8] = { 0 };
    d[0] = SW_CMD_STOP;
    return _queue_push(motor, d);
}
int sw_motor_clear_err(sw_motor_t *motor)
{
    if (motor->logic_state != SW_STATE_RUNNING) {
        return E_ERROR;
    }
    uint8_t d[8] = { 0 };
    d[0] = SW_CMD_CLEAR_ERROR;
    return _queue_push(motor, d);
}

/*---------- end of file ----------*/