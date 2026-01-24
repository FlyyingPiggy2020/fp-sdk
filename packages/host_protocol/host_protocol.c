/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : host_protocol.c
 * @Author       : lxf
 * @Date         : 2025-01-22 10:00:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-01-22 10:00:00
 * @Brief        : 通用上位机通讯协议层实现
 */

/*---------- includes ----------*/
#include "host_protocol.h"
#include <string.h>

/*---------- macro ----------*/

/* 小端模式读写宏 */
#define read_u16_le(ptr)       ((uint16_t)((ptr)[0] | ((ptr)[1] << 8)))
#define read_u32_le(ptr)       ((uint32_t)((ptr)[0] | ((ptr)[1] << 8) | ((ptr)[2] << 16) | ((ptr)[3] << 24)))
#define write_u16_le(ptr, val) ((ptr)[0] = (uint8_t)(val), (ptr)[1] = (uint8_t)((val) >> 8))

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

/**
 * @brief  CRC16计算 (多项式0x8005, 初始值0xFFFF)
 * @param  data: 数据指针
 * @param  len: 数据长度
 * @return CRC16校验值
 * @note   采用查表法优化性能
 */
uint16_t host_crc16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = HOST_CRC16_INIT;
    static const uint16_t crc_table[16] = { 0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
                                            0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022 };

    while (len--) {
        uint8_t byte = *data++;
        uint8_t idx = crc ^ byte;
        idx &= 0x0F;
        crc = (crc >> 4) ^ crc_table[idx];

        idx = crc ^ (byte >> 4);
        idx &= 0x0F;
        crc = (crc >> 4) ^ crc_table[idx];
    }

    return crc;
}

/**
 * @brief  打包数据帧
 * @param  type: 类型字段
 * @param  seq: 序列号
 * @param  payload: 载荷数据
 * @param  len: 载荷长度
 * @param  buf: 输出缓冲区
 * @param  buf_size: 缓冲区大小
 * @return 打包后的帧长度, <0表示失败
 */
int host_pack_frame(uint8_t type, uint16_t seq, const uint8_t *payload, uint16_t len, uint8_t *buf, uint16_t buf_size)
{
    uint16_t crc;
    uint16_t frame_len;

    /* 计算帧总长度 */
    frame_len = HOST_FRAME_HEADER_SIZE + len;

    /* 检查缓冲区大小 */
    if (buf_size < frame_len) {
        return HOST_ERR_BUFFER;
    }

    /* 填充帧头 */
    write_u16_le(&buf[0], HOST_FRAME_HEADER);
    buf[2] = HOST_PROTOCOL_VERSION;
    buf[3] = type;
    write_u16_le(&buf[4], seq);
    write_u16_le(&buf[6], len);

    /* 拷贝载荷 */
    if (len > 0 && payload != NULL) {
        memcpy(&buf[8], payload, len);
    }

    /* 计算并填充CRC16 (对帧头到载荷进行校验) */
    crc = host_crc16(buf, frame_len - 2);
    write_u16_le(&buf[frame_len - 2], crc);

    return frame_len;
}

/**
 * @brief  解析接收数据
 * @param  data: 接收数据指针
 * @param  len: 数据长度
 * @param  frame: 输出帧结构
 * @return 0=解析成功, <0=失败
 */
int host_parse_frame(const uint8_t *data, uint16_t len, struct host_frame *frame)
{
    uint16_t calc_crc;
    uint16_t recv_crc;

    /* 检查最小长度 */
    if (len < HOST_FRAME_HEADER_SIZE) {
        return HOST_ERR_LENGTH;
    }

    /* 解析帧头 */
    frame->header = read_u16_le(&data[0]);
    frame->ver = data[2];
    frame->type = data[3];
    frame->seq = read_u16_le(&data[4]);
    frame->len = read_u16_le(&data[6]);

    /* 检查帧头 */
    if (frame->header != HOST_FRAME_HEADER) {
        return HOST_ERR_HEADER;
    }

    /* 检查版本 (可选，用于未来兼容) */
    if (frame->ver != HOST_PROTOCOL_VERSION) {
        return HOST_ERR_VERSION;
    }

    /* 检查长度 */
    if (len != HOST_FRAME_HEADER_SIZE + frame->len) {
        return HOST_ERR_LENGTH;
    }

    /* 设置载荷指针 */
    if (frame->len > 0) {
        frame->payload = (uint8_t *)&data[8];
    } else {
        frame->payload = NULL;
    }

    /* 校验CRC16 */
    recv_crc = read_u16_le(&data[HOST_FRAME_HEADER_SIZE + frame->len - 2]);
    calc_crc = host_crc16(data, HOST_FRAME_HEADER_SIZE + frame->len - 2);

    if (recv_crc != calc_crc) {
        return HOST_ERR_CRC;
    }

    frame->crc = recv_crc;

    return HOST_ERR_NONE;
}

/**
 * @brief  处理命令类型帧
 * @param  frame: 已解析的帧
 * @param  handler: 命令处理回调
 * @param  userdata: 传递给handler的用户数据
 * @return 0=成功, <0=失败
 */
int host_handle_command(const struct host_frame *frame, host_cmd_handler_t handler, void *userdata)
{
    uint16_t cmd_id;

    /* 检查类型 */
    if (frame->type != HOST_TYPE_CMD) {
        return HOST_ERR_LENGTH;
    }

    /* 检查回调函数 */
    if (handler == NULL) {
        return HOST_ERR_LENGTH;
    }

    /* 检查载荷长度 (至少包含命令ID) */
    if (frame->len < 2) {
        return HOST_ERR_LENGTH;
    }

    /* 解析命令ID (小端) */
    cmd_id = read_u16_le(&frame->payload[0]);

    /* 调用回调函数 */
    return handler(cmd_id, &frame->payload[2], frame->len - 2, userdata);
}

/**
 * @brief  生成应答帧
 * @param  cmd_type: 原命令类型 (自动设置响应位)
 * @param  seq: 序列号
 * @param  result: 0x00=成功, 0x01=失败
 * @param  buf: 输出缓冲区
 * @param  buf_size: 缓冲区大小
 * @return 应答帧长度, <0表示失败
 */
int host_pack_response(uint8_t cmd_type, uint16_t seq, uint8_t result, uint8_t *buf, uint16_t buf_size)
{
    uint8_t response_type;
    uint8_t payload[1];

    /* 构造应答类型 */
    response_type = host_get_response_type(cmd_type);
    payload[0] = result;

    /* 打包帧 */
    return host_pack_frame(response_type, seq, payload, 1, buf, buf_size);
}

/*---------- end of file ----------*/
