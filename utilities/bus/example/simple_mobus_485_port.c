/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : simple_mobus_485_port.c
 * @Author       : lxf
 * @Date         : 2024-12-16 17:20:09
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-12-18 10:17:11
 * @Brief        :
 */

/*---------- includes ----------*/
#undef LOG_TAG
#define LOG_TAG "RS485"
#include "fplog_port.h"
#include "simple_mobus_485_port.h"
#include "bsp.h"
/*---------- macro ----------*/

#define HALFDUPLEX_485_BAUDRATE  115200                                         // 波特率
#define HALFDUPLEX_ONE_TIME      (1000 * 1000 * 11.0 / HALFDUPLEX_485_BAUDRATE) // 单个字节的长度
#define HALFDUPLEX_RECV_OUT_TIME (3.5 * HALFDUPLEX_ONE_TIME)                    // 3.5个字节间隔判断为超时
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
// 半双工总线移植接口
static bool _is_allowed_phase(void);
static bool _is_allowed_sending(void);
static void _send_delay(unsigned short ms);
static int _write_buf(unsigned char *buf, unsigned short len);
static int _read_buf(unsigned char *buf, unsigned short len);
static void _phase(unsigned char *recvbuf, unsigned short recvlen, unsigned char *transbuf, unsigned short translen);

// simple mobus协议移植接口
static uint32_t sys_rand(void);
static int32_t _mobus_485_SetReportTimeout(unsigned int timout, unsigned int compare_timout, int compare);

/*---------- variable ----------*/
bool sending_flag = false; // 发送标志
bool allowed_sending_flag = true;
bool allowed_phase_flag = false;

static half_duplex_bus_t *protocol_bus = NULL;
static half_duplex_bus_ops_t ops = {
    .is_allowed_phase = _is_allowed_phase,
    .is_allowed_sending = _is_allowed_sending,
    .phase = _phase,
    .send_delay = _send_delay,
    .write_buf = _write_buf,
    .read_buf = _read_buf,
};

static simple_mobus_describe_t simple_mobus_485 = {
    .cb = {
        .simple_mobus_cmd_exelist = NULL,
        .rand = sys_rand,
        .set_report_timeout = _mobus_485_SetReportTimeout,
    },
    .time_out = 40, // 默认随机延时时基为40ms
};
/*---------- function ----------*/

/**
 * @brief 是否允许解析
 * @return {*}
 */
static bool _is_allowed_phase(void)
{
    if (allowed_phase_flag == true) {
        return true;
    } else {
        return false;
    }
}

/**
 * @brief 发送延时
 * @param {unsigned short} ms
 * @return {*}
 */
static void _send_delay(unsigned short ms)
{
    bsp_StartTimer(TMR_ID_RS485_TRANS_THREAD, ms);
    return;
}

/**
 * @brief 是否允许发送。1.空闲。2.延时
 * @return {*}
 */
static bool _is_allowed_sending(void)
{
    if (allowed_sending_flag == false) {
        return false;
    }
    if (bsp_CountTimer(TMR_ID_RS485_TRANS_THREAD) == 0) {
        return false;
    }
    return true;
}

/**
 * @brief 写数据
 * @param {unsigned char} *buf
 * @param {unsigned short} len
 * @return {*}
 */
static int _write_buf(unsigned char *buf, unsigned short len)
{
    sending_flag = true;
    return comSendBuf(COM1, buf, len);
}

/**
 * @brief 读取数据
 * @param {unsigned char} *buf
 * @param {unsigned short} len
 * @return {*}
 */
static int _read_buf(unsigned char *buf, unsigned short len)
{
    return comGetBuf(COM1, buf, len);
}

/** 3.5字节超时
 * @brief
 * @return {*}
 */
static void tim_callback(void)
{
    // 因为485TX的时候同时也会RX，所以要做屏蔽处理
    if (sending_flag == true) {
        sending_flag = false;
    } else {
        allowed_phase_flag = true;
        allowed_sending_flag = true;
    }
}

/**
 * @brief 485接收到数据后调用这个函数。
 * @return {*
 */
static void uart_recive_new_call_back(COM_PORT_E com, uint8_t _byte)
{
    allowed_phase_flag = false;
    allowed_sending_flag = false;
    bsp_StartHardTimer(1, HALFDUPLEX_RECV_OUT_TIME, tim_callback);
}
half_duplex_bus_t *get_half_duplex_bus(void)
{
    return protocol_bus;
}

static void uart_recive_error(uint8_t ch, uint8_t err)
{
    log_i("%02x,%02x", ch, err);
    half_duplex_bus_t *bus = get_half_duplex_bus();
    half_duplex_bus_transmit_cache_complete(bus);
}
/**
 * @brief 解析回调函数
 * @param {unsigned char} *recvbuf
 * @param {unsigned short} recvlen
 * @param {unsigned char} *transbuf
 * @param {unsigned short} translen
 * @return {*}
 */
static void _phase(unsigned char *recvbuf, unsigned short recvlen, unsigned char *transbuf, unsigned short translen)
{
    log_hex_dump("[RS485_R]", 32, recvbuf, recvlen);
    simple_mobus_phase(&simple_mobus_485, recvbuf, recvlen, transbuf, translen);
    allowed_phase_flag = false;
}

static uint32_t sys_rand(void)
{
    static uint8_t count = 0; // 防止第一次随机数相同
    if (count == 0) {
        srand((unsigned)bsp_GetRunTime());
        rand();
        count++;
    }
    return rand();
}

/*
*********************************************************************************************************
*   函 数 名: mobus_SetReportTimeout
*   功能说明: 上报时间设置
*   形    参: timout：超时时间
*             compare_timout：比较值
*             compare：>0：剩余时间大于比较值 <0：剩余时间小于比较值 ==0：当前值
*   返 回 值: 无
*********************************************************************************************************
*/
static int32_t _mobus_485_SetReportTimeout(unsigned int timout, unsigned int compare_timout, int compare)
{
    if (compare > 0) {
        if (bsp_RemainTimer(TMR_ID_RS485_TRANS_THREAD) > compare_timout) {
            bsp_StartTimer(TMR_ID_RS485_TRANS_THREAD, timout);
        }
    } else if (compare < 0) {
        if (bsp_RemainTimer(TMR_ID_RS485_TRANS_THREAD) < compare_timout) {
            bsp_StartTimer(TMR_ID_RS485_TRANS_THREAD, timout);
        }
    } else {
        bsp_StartTimer(TMR_ID_RS485_TRANS_THREAD, timout);
    }

    return bsp_RemainTimer(TMR_ID_RS485_TRANS_THREAD);
}
/**
 * @brief 注册cmd解析函数
 * @param {void} *args
 * @return {*}
 */
simple_mobus_describe_t *simple_mobnus_485_port_init(uint8_t addr, void *args)
{
    bsp_InitHardTimer();
    /* 1.注册 半双工总线 */
    if (protocol_bus == NULL) {
        protocol_bus = half_duplex_bus_new(&ops, 256);
    }
    extern UART_T g_tUart1;
    g_tUart1.ReciveNew = uart_recive_new_call_back;
    g_tUart1.half_bus_err = uart_recive_error;
    if (protocol_bus == NULL) {
        return NULL;
    }

    /* 把simple mobus协议挂载到半双工总线上 */
    simple_mobus_485.info.mod_addr = addr;
    simple_mobus_485.cb.simple_mobus_cmd_exelist = args;
    simple_mobus_new(&simple_mobus_485, protocol_bus);
    return &simple_mobus_485;
}

/*---------- end of file ----------*/
