/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : duplex_bus.h
 * @Author       : lxf
 * @Date         : 2025-07-22 11:54:17
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-07-22 11:55:19
 * @Brief        : 用户需要实现ops，并且设置tx_delay，这个参数是2帧数据之间间隔时间。
 * 用get_delay_ms这个函数实现随机间隔，递增间隔等个性化的需求
 *
 */

#ifndef __DUPLEX_BUS_H__
#define __DUPLEX_BUS_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdint.h>
#include <string.h>
#include "clists.h"
#include "heap.h"
/*---------- macro ----------*/
enum {
    SERIAL_BUS_PRIORITY_IDLE = 0,
    SERIAL_BUS_PRIORITY_NORMAL,
    SERIAL_BUS_PRIORITY_REGISTER,
    SERIAL_BUS_PRIORITY_ACK,
    SERIAL_BUS_PRIORITY_MAX
};

struct serial_bus_transport_node {
    struct list_head node;
    uint8_t trans_cnt;
    uint8_t data_len;
    uint8_t payload[];
};

struct serial_duplex_bus_ops {
    int32_t (*write_buf)(uint8_t *buf, uint16_t len);
    // 获取延时时间(有的时候有修改两帧之间的延时的需求)例如随着发送的次数，延时越来越大。例如间隔在40~100ms之间随机。
    // 如果这个函数为NULL，那么默认的发送延时为tx_delay.
    int32_t (*get_delay_ms)(struct serial_bus_transport_node n); //
};

struct serial_duplex_bus_cb {
    int32_t (*recv_phare_callback)(uint8_t *recv, uint16_t recv_len, uint8_t *trans, uint16_t trans_len);
};

struct serial_bus_handle {
    struct serial_duplex_bus_ops *ops;                 // 操作函数
    struct serial_duplex_bus_cb *cb;                   // 回调函数
    struct list_head tx_list[SERIAL_BUS_PRIORITY_MAX]; // 发送队列
    uint16_t tx_delay;                                 // 默认发送延时
    uint16_t tx_delay_count;                           // 发送计数值
    struct serial_bus_transport_node *cur_tx_node;     // 当前发送的node节点
};

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
struct serial_bus_handle *serial_bus_new(struct serial_duplex_bus_ops *ops, struct serial_duplex_bus_cb *cb);
void serial_bus_delete(struct serial_bus_handle *self);
void serial_transport_poll(struct serial_bus_handle *self);
void serial_bus_send_to_cache(struct serial_bus_handle *self, const uint8_t *pdata, uint32_t length, uint8_t priority, uint8_t trans_cnt);
void serial_bus_stop_retrans(struct serial_bus_handle *self);
void serial_bus_receive(struct serial_bus_handle *self, const uint8_t *pdata, uint32_t length);

/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif // !__DUPLEX_BUS_H__
