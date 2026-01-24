/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : host_protocol.h
 * @Author       : lxf
 * @Date         : 2025-01-22 10:00:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-01-22 10:00:00
 * @Brief        : 通用上位机通讯协议层
 * @features     :
 *               - 设备无关的通用帧格式编解码
 *               - 支持命令、波形数据、主动上报等类型
 *               - CRC16校验确保数据完整性
 *               - 支持多实例，无静态全局变量
 *
 * @usage        :
 *               @code
 *               // 打包并发送波形数据
 *               uint8_t buf[128];
 *               int len = host_pack_frame(HOST_TYPE_WAVE1, seq, payload, payload_len,
 *                                        buf, sizeof(buf));
 *               device_write(uart, buf, 0, len);
 *
 *               // 解析接收数据
 *               struct host_frame frame;
 *               if (host_parse_frame(rx_buf, rx_len, &frame) == 0) {
 *                   // 处理帧...
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

/*---------- macro ----------*/

/* 帧格式定义 */
#define HOST_FRAME_HEADER      0xAA55  /* 帧头 (小端存储为0x55AA) */
#define HOST_PROTOCOL_VERSION  0x00   /* 当前协议版本 */

/* 类型字段定义 */
#define HOST_TYPE_WAVE1        0x01   /* 波形数据1 MCU→上位机 */
#define HOST_TYPE_WAVE2        0x02   /* 波形数据2 上位机→MCU */
#define HOST_TYPE_CMD          0x03   /* 命令数据 */
#define HOST_TYPE_REPORT       0x04   /* 主动上报 */
#define HOST_TYPE_RESPONSE     0x80   /* 应答标志位 (D7位) */

/* 帧头固定长度 */
#define HOST_FRAME_HEADER_SIZE 10     /* 帧头(2) + 版本(1) + 类型(1) + 序列号(2) + 长度(2) + CRC(2) */

/* CRC16参数 */
#define HOST_CRC16_INIT        0xFFFF
#define HOST_CRC16_POLY        0x8005

/* 错误码 */
#define HOST_ERR_NONE          0      /* 无错误 */
#define HOST_ERR_HEADER        -1     /* 帧头错误 */
#define HOST_ERR_VERSION       -2     /* 版本不匹配 */
#define HOST_ERR_LENGTH        -3     /* 长度错误 */
#define HOST_ERR_CRC           -4     /* CRC校验失败 */
#define HOST_ERR_BUFFER        -5     /* 缓冲区不足 */

/*---------- type define ----------*/

/**
 * @brief 通用帧结构
 * @note   payload指向外部缓冲区，不持有内存
 */
struct host_frame {
    uint16_t header;           /* 帧头 0x55AA */
    uint8_t  ver;              /* 协议版本 */
    uint8_t  type;             /* 类型 */
    uint16_t seq;              /* 序列号 */
    uint16_t len;              /* 载荷长度 */
    uint8_t  *payload;         /* 载荷指针 */
    uint16_t crc;              /* CRC16校验 */
};

/**
 * @brief 命令处理回调函数类型
 * @param  cmd_id: 命令ID
 * @param  param: 命令参数指针
 * @param  len: 参数长度
 * @param  userdata: 用户数据
 * @return 0=成功, <0=失败
 */
typedef int (*host_cmd_handler_t)(uint16_t cmd_id, const uint8_t *param, uint16_t len, void *userdata);

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/

/**
 * @brief  打包数据帧
 * @param  type: 类型字段
 * @param  seq: 序列号
 * @param  payload: 载荷数据
 * @param  len: 载荷长度
 * @param  buf: 输出缓冲区
 * @param  buf_size: 缓冲区大小
 * @return 打包后的帧长度, <0表示失败
 * @note   帧格式: [帧头2][版本1][类型1][序列号2][长度2][载荷N][CRC16_2]
 */
int host_pack_frame(uint8_t type, uint16_t seq, const uint8_t *payload, uint16_t len,
                    uint8_t *buf, uint16_t buf_size);

/**
 * @brief  解析接收数据
 * @param  data: 接收数据指针
 * @param  len: 数据长度
 * @param  frame: 输出帧结构
 * @return 0=解析成功, <0=失败
 * @note   frame.payload指向data内部的载荷区域
 */
int host_parse_frame(const uint8_t *data, uint16_t len, struct host_frame *frame);

/**
 * @brief  处理命令类型帧
 * @param  frame: 已解析的帧 (type应为HOST_TYPE_CMD)
 * @param  handler: 命令处理回调
 * @param  userdata: 传递给handler的用户数据
 * @return 0=成功, <0=失败
 * @note   从载荷中解析命令ID并调用回调函数
 */
int host_handle_command(const struct host_frame *frame,
                        host_cmd_handler_t handler, void *userdata);

/**
 * @brief  生成应答帧
 * @param  cmd_type: 原命令类型 (自动设置响应位)
 * @param  seq: 序列号
 * @param  result: 0x00=成功, 0x01=失败
 * @param  buf: 输出缓冲区
 * @param  buf_size: 缓冲区大小
 * @return 应答帧长度, <0表示失败
 */
int host_pack_response(uint8_t cmd_type, uint16_t seq, uint8_t result,
                       uint8_t *buf, uint16_t buf_size);

/**
 * @brief  CRC16计算 (多项式0x8005, 初始值0xFFFF)
 * @param  data: 数据指针
 * @param  len: 数据长度
 * @return CRC16校验值
 */
uint16_t host_crc16(const uint8_t *data, uint16_t len);

/**
 * @brief  判断是否为应答类型
 * @param  type: 类型字段
 * @return 1=是应答, 0=非应答
 */
static inline int host_is_response(uint8_t type)
{
    return (type & HOST_TYPE_RESPONSE) != 0;
}

/**
 * @brief  获取命令对应的应答类型
 * @param  cmd_type: 命令类型
 * @return 应答类型
 */
static inline uint8_t host_get_response_type(uint8_t cmd_type)
{
    return cmd_type | HOST_TYPE_RESPONSE;
}

/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __HOST_PROTOCOL_H__ */
