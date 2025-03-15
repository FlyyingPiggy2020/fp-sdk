/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : half_duplex_bus.h
 * @Author       : lxf
 * @Date         : 2024-09-04 14:58:29
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-09-20 09:16:33
 * @Brief        :
 */

#ifndef HALF_DUPLEX_TRANSMITTER_H
#define HALF_DUPLEX_TRANSMITTER_H
/*---------- includes ----------*/

#include "clists.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
typedef struct half_duplex_bus_ops {
    /**
     * @brief 发送数据
     * @param {unsigned char} *buf 待发送数据指针
     * @param {unsigned short} len 待发送数据长度
     * @return {*} 实际发送的数据长度
     */
    int (*write_buf)(unsigned char *buf, unsigned short len);
    /**
     * @brief 接收数据
     * @param {unsigned char} *buf 待接收的数据指针
     * @param {unsigned short} len 希望接收的数据长度
     * @return {*} 实际接收的数据长度
     */
    int (*read_buf)(unsigned char *buf, unsigned short len);
    /**
     * @brief 是否允许发送，用于实现总线防冲突。用户自定义实现。
     * @param {void}
     * @return {*}
     */
    bool (*is_allowed_sending)(void);
    /**
     * @brief 是否允许接收，用于实现总线接收分帧超时。用户自定义实现。
     * @return {*}
     */
    bool (*is_allowed_phase)(void);
    /**
     * @brief 发送延时，用于半双工发送数据帧之间延时。用户自定义实现。
     * @param {unsigned short} ms 延时时间单位毫秒
     * @return {*}
     */
    void (*send_delay)(unsigned short ms);
    /**
     * @brief 数据解析，里面附带了当前发送的数据，可以实现重发停止的功能
     * @param {unsigned char} *recvbuf 接收的数据指针
     * @param {unsigned short} recvlen 接收的数据长度
     * @param {unsigned char} transbuf 正在发送的数据指针
     * @param {unsigned short} translen 正在发送的数据长度
     * @return {*}
     */
    void (*phase)(unsigned char *recvbuf, unsigned short recvlen, unsigned char *transbuf, unsigned short translen);
    /**
     * @brief 中断关闭，如果在中断里调用需要开关中断，里面有原子操作不允许被打断。
     * @return {*}
     */
    void (*lock)(void);
    /**
     * @brief 中断打开，如果在中断里调用需要开关中断，里面有原子操作不允许被打断。
     * @return {*}
     */
    void (*unlock)(void);
} half_duplex_bus_ops_t;
typedef struct half_duplex_bus_trans_node half_duplex_bus_trans_node_t;
typedef struct half_duplex_bus_trans_node {
    unsigned short len;
    unsigned char retrans_count;
    unsigned char retrans_max;
    /**
     * @brief 随机延时计算函数，里面可以自己实现延时的算法，针对不同情况实现不同的延时
     * @param {half_duplex_bus_trans_node_t} *self
     * @return {*}
     */
    unsigned short (*random_ms)(void);
    unsigned char priority; // 数值越大，优先级越高
    struct list_head node;
} half_duplex_bus_trans_node_t;
typedef struct half_duplex_bus_trans {
    struct list_head root;
    void *cur_node; // 当前正在发送的节点
} half_duplex_bus_trans_t;

typedef struct half_duplex_bus_receive {
    unsigned char *buf;
    unsigned short len;
    unsigned short capacity;
} half_duplex_bus_receive_t;

typedef struct half_duplex_bus {
    half_duplex_bus_trans_t trans;
    half_duplex_bus_receive_t receive;
    half_duplex_bus_ops_t *ops;
} half_duplex_bus_t;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
half_duplex_bus_t *half_duplex_bus_new(half_duplex_bus_ops_t *ops, unsigned short recv_capacity);
void half_duplex_bus_destory(half_duplex_bus_t *self);
void half_duplex_bus_transmitter_cache(half_duplex_bus_t *bus, unsigned char *buf, unsigned short len, unsigned char retrans, unsigned char priority, void *random_ms);
void half_duplex_bus_transmit_cache_complete(half_duplex_bus_t *bus);
void half_duplex_bus_handle(half_duplex_bus_t *bus);

/*---------- end of file ----------*/
#endif
