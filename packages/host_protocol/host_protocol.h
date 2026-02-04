/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : host_protocol.h
 * @Author       : lxf
 * @Date         : 2025-01-22 10:00:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-02-04 13:13:04
 * @Brief        : 通用上位机通讯协议层
 * @features     :
 *               - 设备无关的通用帧格式编解码
 *               - 支持命令、波形数据、主动上报等类型
 *               - CRC16校验确保数据完整性
 *               - 支持多实例，无静态全局变量
 *
 * @usage        :
 *               @code
 *               // 定义发送操作
 *               static int my_send(const uint8_t *buf, uint16_t len)
 *               {
 *                   return serial_bus_send(&g_bus, buf, len);
 *               }
 *
 *               static void my_stop_retrans(void)
 *               {
 *                   serial_bus_stop_retrans(&g_bus);
 *               }
 *
 *               // 初始化会话
 *               struct host_ops ops = {
 *                   .send = my_send,
 *                   .stop_retrans = my_stop_retrans,
 *               };
 *               host_session_init(&sess, &ops);
 *
 *               // 发送数据
 *               host_send(&sess, HOST_TYPE_WAVE1, payload, len, NULL);
 *
 *               // 发送应答
 *               host_send_response(&sess, HOST_TYPE_CMD, seq, HOST_RESULT_SUCCESS, NULL, 0);
 *
 *               // 解析接收数据
 *               struct host_frame frame;
 *               if (host_parse_frame(rx_buf, rx_len, &frame) == 0) {
 *                   if (host_handle_response(&frame, &sess)) {
 *                       // 是响应帧，已自动停止重发
 *                   }
 *               }
 *               @endcode
 */

#ifndef __HOST_PROTOCOL_H__
#define __HOST_PROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*---------- macro ----------*/

/* 帧格式定义 */
#define HOST_FRAME_HEADER      0x55AA /* 帧头 */
#define HOST_PROTOCOL_VERSION  0x00   /* 当前协议版本 */

/* 类型字段定义 */
#define HOST_TYPE_WAVE1        0x01 /* 波形数据1 MCU→上位机 */
#define HOST_TYPE_WAVE2        0x02 /* 波形数据2 上位机→MCU */
#define HOST_TYPE_CMD          0x03 /* 命令数据 */
#define HOST_TYPE_REPORT       0x04 /* 主动上报 */
#define HOST_TYPE_PARAM_SET    0x05 /* 参数设置 */
#define HOST_TYPE_PARAM_GET    0x06 /* 参数获取 */
#define HOST_TYPE_RESPONSE     0x80 /* 应答标志位 (D7位) */

/* 帧头固定长度 */
#define HOST_FRAME_HEADER_SIZE 10 /* 帧头(2) + 版本(1) + 类型(1) + 序列号(2) + 长度(2) + CRC(2) */

/* CRC16参数 */
#define HOST_CRC16_INIT        0xFFFF
#define HOST_CRC16_POLY        0x8005

/* 错误码 */
#define HOST_ERR_NONE          0  /* 无错误 */
#define HOST_ERR_HEADER        -1 /* 帧头错误 */
#define HOST_ERR_LENGTH        -2 /* 长度错误 */
#define HOST_ERR_CRC           -3 /* CRC校验失败 */
#define HOST_ERR_BUFFER        -4 /* 缓冲区不足 */
#define HOST_ERR_NSUPPORT      -5 /* 不支持的功能 */

/* 通用应答 */
#define HOST_RESULT_SUCCESS    0x00 /* 成功 */
#define HOST_RESULT_FAIL       0x01 /* 失败 */
#define HOST_RESULT_DUPLICATE  0x02 /* 重复命令 */
#define HOST_RESULT_ARG_ERR    0x03 /* 参数错误 */
/*---------- type define ----------*/

/**
 * @brief 协议层操作接口
 * @note  参考 serial_bus_ops 设计风格
 */
struct host_ops {
    /**
     * @brief 发送数据
     * @param buf: 待发送数据
     * @param len: 数据长度
     * @return 0=成功, <0=失败
     */
    int (*send)(const uint8_t *buf, uint16_t len);

    /**
     * @brief 停止重发
     */
    void (*stop_retrans)(void);

    /**
     * @brief 命令处理
     * @param cmd_id: 命令ID
     * @param param: 命令参数
     * @param len: 参数长度
     * @param extra_data: 输出额外数据指针
     * @param extra_len: 输出额外数据长度
     * @return HOST_RESULT_SUCCESS=成功, HOST_RESULT_FAIL=失败
     */
    int (*cmd_handler)(uint16_t cmd_id, const uint8_t *param, uint16_t len, uint8_t **extra_data, uint16_t *extra_len);
};

/**
 * @brief 协议层会话上下文
 * @note  应用层创建并维护此结构体，支持多实例
 */
struct host_session {
    uint16_t last_cmd_seq; /* 上一次处理的命令序列号 */
    uint16_t tx_seq;       /* 发送序列号 */

    const struct host_ops *ops; /* 操作接口 */
};

/**
 * @brief 通用帧结构
 * @note   payload指向外部缓冲区，不持有内存
 */
struct host_frame {
    uint16_t header;  /* 帧头 0x55AA */
    uint8_t ver;      /* 协议版本 */
    uint8_t type;     /* 类型 */
    uint16_t seq;     /* 序列号 */
    uint16_t len;     /* 载荷长度 */
    uint8_t *payload; /* 载荷指针 */
    uint16_t crc;     /* CRC16校验 */
};

/**
 * @brief 命令处理回调
 * @param cmd_id: 命令ID
 * @param param: 命令参数指针
 * @param len: 参数长度
 * @param extra_data: 输出额外数据指针 (由回调填充)
 * @param extra_len: 输出额外数据长度
 * @return HOST_RESULT_SUCCESS=成功, HOST_RESULT_FAIL=失败
 */
typedef int (*host_cmd_handler_t)(
    uint16_t cmd_id, const uint8_t *param, uint16_t len, uint8_t **extra_data, uint16_t *extra_len);

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

/**
 * @brief  初始化会话
 * @param  sess: 会话结构体指针
 * @param  ops: 操作接口
 */
void host_session_init(struct host_session *sess, const struct host_ops *ops);

/*---------- 发送相关 ----------*/

/**
 * @brief  发送应答帧
 * @param  sess: 会话上下文
 * @param  cmd_type: 原命令类型 (自动设置响应位)
 * @param  seq: 序列号
 * @param  cmd_id:   原ID
 * @param  result: 0x00=成功, 0x01=失败
 * @param  ext_result: 扩展结果数据 (可选)
 * @param  ext_len: 扩展结果长度
 * @return 0=成功, <0=失败
 */
int host_send_response(struct host_session *sess,
                       uint8_t cmd_type,
                       uint16_t seq,
                       uint16_t cmd_id,
                       uint8_t result,
                       const uint8_t *ext_result,
                       uint16_t ext_len);

/**
 * @brief  发送波形数据1 (不重发)
 * @param  sess: 会话上下文
 * @param  payload: 波形数据载荷
 * @param  len: 载荷长度
 * @return 0=成功, <0=失败
 */
int host_send_wave1(struct host_session *sess, const uint8_t *payload, uint16_t len);

/**
 * @brief  发送主动上报 (不重发)
 * @param  sess: 会话上下文
 * @param  report_id: 上报ID
 * @param  param: 上报参数
 * @param  len: 参数长度
 * @return 0=成功, <0=失败
 */
int host_send_report(struct host_session *sess, uint16_t report_id, const uint8_t *param, uint16_t len);

/*---------- 接收相关 ----------*/

/**
 * @brief  处理接收数据 (一站式接口)
 * @param  recv: 接收数据指针
 * @param  recv_len: 接收数据长度
 * @param  trans: 透传数据指针 (未使用，保留接口兼容性)
 * @param  trans_len: 透传数据长度
 * @param  sess: 会话上下文
 * @param  cmd_handler: 命令处理回调
 * @return 0=成功处理, <0=解析失败, 1=响应帧已停止重发
 *
 * @note  此函数封装了完整的接收处理流程:
 *        1. 解析帧数据
 *        2. 处理响应帧(自动停止重发)
 *        3. 处理命令帧:
 *           - 检查重复命令
 *           - 执行命令处理(获取额外数据)
 *           - 发送响应
 */
int host_process(uint8_t *recv, uint16_t recv_len, uint8_t *trans, uint16_t trans_len, struct host_session *sess);

/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __HOST_PROTOCOL_H__ */
