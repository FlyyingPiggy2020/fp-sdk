/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : host_protocol.c
 * @Author       : lxf
 * @Date         : 2025-01-22 10:00:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-01-28 16:30:00
 * @Brief        : 通用上位机通讯协议层实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "host_protocol.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "crc.h"

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
static uint16_t host_crc16(const uint8_t *data, uint16_t len)
{
    return crc16_calc(data, len, 0x8005, 0xffff);
}

/*---------- 发送相关 ----------*/

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
 * @brief  初始化会话
 * @param  sess: 会话结构体指针
 * @param  ops: 操作接口
 */
void host_session_init(struct host_session *sess, const struct host_ops *ops)
{
    sess->last_cmd_seq = 0xFFFF;
    sess->tx_seq = 0;
    sess->ops = ops;
}

/**
 * @brief  发送数据帧 (组包 + 调用 ops->send)
 * @param  sess: 会话上下文
 * @param  type: 类型字段
 * @param  payload: 载荷数据
 * @param  len: 载荷长度
 * @return 0=成功, <0=失败
 */
static int host_send(struct host_session *sess, uint8_t type, const uint8_t *payload, uint16_t len)
{
    uint8_t buf[256];
    int frame_len;

    if (!sess->ops || !sess->ops->send) {
        return HOST_ERR_NSUPPORT;
    }

    /* 打包帧 */
    frame_len = host_pack_frame(type, sess->tx_seq, payload, len, buf, sizeof(buf));
    sess->tx_seq = (sess->tx_seq + 1) & 0xFFFF;
    if (frame_len < 0) {
        return frame_len;
    }

    /* 调用发送接口 */
    return sess->ops->send(buf, frame_len);
}

/**
 * @brief  发送应答帧
 * @param  sess: 会话上下文
 * @param  cmd_type: 原命令类型 (自动设置响应位)
 * @param  seq: 序列号
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
                       uint16_t ext_len)
{
    uint8_t payload[256];
    uint16_t payload_len;

    if (!sess->ops || !sess->ops->send) {
        return HOST_ERR_NSUPPORT;
    }

    /* 构造载荷: [result(1)][ext_result(N)] */
    write_u16_le(&payload[0], cmd_id);
    payload[2] = result;
    payload_len = 3;

    if (ext_result != NULL && ext_len > 0) {
        if (payload_len + ext_len > sizeof(payload)) {
            return HOST_ERR_BUFFER;
        }
        memcpy(payload + payload_len, ext_result, ext_len);
        payload_len += ext_len;
    }

    /* 发送应答帧 */
    return host_send(sess, cmd_type | HOST_TYPE_RESPONSE, payload, payload_len);
}

/**
 * @brief  发送波形数据1 (不重发)
 * @param  sess: 会话上下文
 * @param  payload: 波形数据载荷
 * @param  len: 载荷长度
 * @return 0=成功, <0=失败
 */
int host_send_wave1(struct host_session *sess, const uint8_t *payload, uint16_t len)
{
    return host_send(sess, HOST_TYPE_WAVE1, payload, len);
}

/**
 * @brief  发送主动上报 (不重发)
 * @param  sess: 会话上下文
 * @param  report_id: 上报ID
 * @param  param: 上报参数
 * @param  len: 参数长度
 * @return 0=成功, <0=失败
 * @note   载荷格式: [report_id(2)][param(N)]
 */
int host_send_report(struct host_session *sess, uint16_t report_id, const uint8_t *param, uint16_t len)
{
    uint8_t payload[256];
    uint16_t payload_len = 2 + len;

    if (payload_len > sizeof(payload)) {
        return HOST_ERR_BUFFER;
    }

    write_u16_le(&payload[0], report_id);
    if (len > 0 && param != NULL) {
        memcpy(&payload[2], param, len);
    }

    return host_send(sess, HOST_TYPE_REPORT, payload, payload_len);
}

/*---------- 接收相关 ----------*/

/**
 * @brief  解析接收数据
 * @param  data: 接收数据指针
 * @param  len: 数据长度
 * @param  frame: 输出帧结构
 * @return 0=解析成功, <0=失败
 */
static int host_parse_frame(const uint8_t *data, uint16_t len, struct host_frame *frame)
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
 * @brief  检查是否为重复命令
 * @param  frame: 已解析的帧
 * @param  sess: 会话上下文
 * @return true=重复, false=新命令
 */
static bool host_is_duplicate_cmd(const struct host_frame *frame, struct host_session *sess)
{
    if (frame->type != HOST_TYPE_CMD) {
        return false;
    }
    return (frame->seq == sess->last_cmd_seq);
}

/**
 * @brief  处理命令类型帧
 * @param  frame: 已解析的帧
 * @param  handler: 命令处理回调
 * @param  extra_data: 输出额外数据指针 (由handler填充)
 * @param  extra_len: 输出额外数据长度
 * @return 0=成功, <0=失败
 */
static int host_handle_command(const struct host_frame *frame,
                               host_cmd_handler_t handler,
                               uint8_t **extra_data,
                               uint16_t *extra_len)
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

    /* 初始化额外数据输出 */
    if (extra_data) {
        *extra_data = NULL;
    }
    if (extra_len) {
        *extra_len = 0;
    }

    /* 调用回调函数，传入额外数据输出参数 */
    return handler(cmd_id, &frame->payload[2], frame->len - 2, extra_data, extra_len);
}

/**
 * @brief  处理响应帧（收到ACK时停止重发）
 * @param  frame: 已解析的帧
 * @param  sess: 会话上下文
 * @return true=是响应帧(已调用停止重发), false=不是响应帧
 */
static bool host_handle_response(const struct host_frame *frame, struct host_session *sess)
{
    if (!(frame->type & HOST_TYPE_RESPONSE)) {
        return false;
    }

    /* 如果设置了停止重发回调，调用它 */
    if (sess->ops && sess->ops->stop_retrans) {
        sess->ops->stop_retrans();
    }

    return true;
}

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
int host_process(uint8_t *recv, uint16_t recv_len, uint8_t *trans, uint16_t trans_len, struct host_session *sess)
{
    static struct host_frame frame;
    uint16_t cmd_id;
    int8_t result;
    uint8_t *extra_data = NULL;
    uint16_t extra_len = 0;

    (void)trans;
    (void)trans_len;

    /* 第1步: 解析帧 */
    if (host_parse_frame(recv, recv_len, &frame) != HOST_ERR_NONE) {
        return -1;
    }

    /* 第2步: 处理响应帧 */
    if (host_handle_response(&frame, sess)) {
        return 1;
    }

    /* 第3步: 只处理命令帧 */
    if (frame.type != HOST_TYPE_CMD) {
        return 0;
    }

    cmd_id = read_u16_le(&frame.payload[0]);

    /* 第4步: 检查重复命令 */
    if (host_is_duplicate_cmd(&frame, sess)) {
        /* 重复命令，发送DUPLICATE响应 */
        host_send_response(sess, HOST_TYPE_CMD, frame.seq, cmd_id, HOST_RESULT_DUPLICATE, NULL, 0);
        return 0;
    }

    /* 第5步: 执行命令处理，获取额外数据 */
    result = host_handle_command(&frame, sess->ops->cmd_handler, &extra_data, &extra_len);
    sess->last_cmd_seq = frame.seq;

    /* 第6步: 发送响应 */
    host_send_response(sess, HOST_TYPE_CMD, frame.seq, cmd_id, result, extra_data, extra_len);

    return 0;
}

/*---------- end of file ----------*/
