/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : half_duplex_bus.c
 * @Author       : lxf
 * @Date         : 2024-09-04 14:58:11
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-09-19 11:14:35
 * @Brief        : 半双工总线驱动模块
 * 1.支持总线防冲突，以及退避机制。
 * 2.支持重发。（填入几次就重发几次，比如填入3就重发3次。但是至少会发送一次，填入0也会发送1次）
 * 3.支持优先级（但是不支持抢断,如果有优先级更高的数据包，会在当前数据包发送完毕之后再去发送）
 * 4.支持半双工模式(发送时候会判断是否正在接收)
 *
 * 综上要求提供一个API：half_duplex_bus_transmitter_cache
 * 入参为待发送的数据，指针，以及重发次数和优先级
 *
 * 需要用户实现half_duplex_bus_ops_t里面的函数
 */

/*---------- includes ----------*/

#include "half_duplex_bus.h"
#include "stdbool.h"
#include "string.h"
#include "stdlib.h"

/*---------- macro ----------*/
#ifdef USE_HALF_DUPLEX_BUS_DEBUG
#define TRACE(fmt, ...) log_info(fmt, ##__VA_ARGS__)
// #define ASSERT(expr)
#define ASSERT(expr)                          \
    if (!(expr)) {                            \
        TRACE("Assertion failed: %s", #expr); \
        while (1)                             \
            ;                                 \
    }
#else
#define TRACE(...)
#define ASSERT(expr)
#endif
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/

/*---------- function ----------*/

/**
 * @brief 注册一条新的半双工总线
 * @param {half_duplex_bus_ops_t} *ops 注册的ops操作api
 * @param {unsigned short} recv_capacity 总线接收缓存
 * @return {*}
 */
half_duplex_bus_t *half_duplex_bus_new(half_duplex_bus_ops_t *ops, unsigned short recv_capacity)
{
    half_duplex_bus_t *self = NULL;
    do {
        if (ops == NULL) {
            TRACE("ops is NULL.");
            break;
        }
        self = malloc(sizeof(half_duplex_bus_t) + recv_capacity);
        if (self == NULL) {
            TRACE("malloc failed.Used memory= %d.Total memory size = %d.", heap_get_used_size(), MEMORY_POOL_SIZE);
            break;
        }
        memset(self, 0, sizeof(half_duplex_bus_t) + recv_capacity);
        self->receive.buf = (unsigned char *)self + sizeof(half_duplex_bus_t);
        self->receive.capacity = recv_capacity;
        self->ops = ops;
        INIT_LIST_HEAD(&self->trans.root);
        TRACE("malloc bus success. ptr:%p", self);
        TRACE("Used memory= %d.Total memory size = %d.", heap_get_used_size(), MEMORY_POOL_SIZE);
    } while (0);
    return self;
}

/**
 * @brief 销毁一条总线。该函数会遍历所有trans node的缓存并释放,最后会释放self
 * @param {half_duplex_bus_t} *self 总线的指针
 * @return {*}
 */
void half_duplex_bus_destory(half_duplex_bus_t *self)
{
    half_duplex_bus_trans_node_t *pos = NULL, *n = NULL;
    list_for_each_entry_safe(pos, n, half_duplex_bus_trans_node_t, &self->trans.root, node)
    {
        list_del(&n->node);
        free(n);
    }
    free(self);
    TRACE("bus%p destory.Used memory= %d.Total memory size = %d.", self, heap_get_used_size(), MEMORY_POOL_SIZE);
}

/**
 * @brief 需要在一个时间片中执行这个函数
 * @param {half_duplex_bus_t} *bus
 * @return {*}
 */
void half_duplex_bus_handle(half_duplex_bus_t *bus)
{
    ASSERT(bus);
    ASSERT(bus->ops);
    ASSERT(bus->ops->write_buf);
    ASSERT(bus->ops->read_buf);
    ASSERT(bus->ops->is_allowed_sending);
    ASSERT(bus->ops->is_allowed_phase);
    ASSERT(bus->ops->phase);

    /* 半双工接收 */
    do {
        /* 1.判断是否允许接收 */
        if (bus->ops->is_allowed_phase() != true) {
            break;
        }
        /* 2.接收数据 */
        bus->receive.len = bus->ops->read_buf(bus->receive.buf, bus->receive.capacity);
        if (bus->receive.buf != NULL && bus->receive.len != 0) {
            if (bus->ops->lock) {
                bus->ops->lock();
            }
            half_duplex_bus_trans_node_t *node = bus->trans.cur_node;
            unsigned char *transbuf = NULL;
            unsigned short translen = 0;
            if (node != NULL) {
                transbuf = (unsigned char *)node + sizeof(half_duplex_bus_trans_node_t);
                translen = node->len;
            }
            if (bus->ops->phase) {
                bus->ops->phase(bus->receive.buf, bus->receive.len, transbuf, translen);
            }
            if (bus->ops->unlock) {
                bus->ops->unlock();
            }
            return;
        }
    } while (0);
    /* 半双工发送 */
    do {
        /* 1.判断是否允许发送 */
        if (bus->ops->is_allowed_sending() != true) {
            break;
        }
        /* 1.如果当前未在发送，则查找优先级最高的报文 */
        if (bus->trans.cur_node == NULL && list_empty(&bus->trans.root) == false) {

            half_duplex_bus_trans_node_t *pos = NULL, *n = NULL;
            unsigned char priority = 0;
            list_for_each_entry_safe(pos, n, half_duplex_bus_trans_node_t, &bus->trans.root, node)
            {
                if (pos->priority > priority) {
                    priority = pos->priority;
                    bus->trans.cur_node = pos;
                }
            }
        }
        /* 2.根据重发次数发送数据 */
        if (bus->trans.cur_node != NULL) {
            if (bus->ops->lock) {
                bus->ops->lock();
            }
            half_duplex_bus_trans_node_t *node = bus->trans.cur_node;
            if (node->retrans_max > node->retrans_count) {
                node->retrans_count++;
            }
            unsigned char *transbuf = NULL;
            unsigned short translen = 0;

            transbuf = (unsigned char *)(node) + sizeof(half_duplex_bus_trans_node_t);
            translen = node->len;
            bus->ops->write_buf(transbuf, translen);
            if (node->random_ms != NULL) {
                unsigned short ms = node->random_ms(bus->trans.cur_node);
                bus->ops->send_delay(ms);
            } else {
                bus->ops->send_delay(60 + 40 * node->retrans_count);
            }
            if (node->retrans_count >= node->retrans_max) {
                list_del(&node->node);
                free(node);
                bus->trans.cur_node = NULL;
            }
            if (bus->ops->unlock) {
                bus->ops->unlock();
            }
        }
    } while (0);
}

/**
 * @brief 带重发和优先级的传输函数
 * @param {unsigned char} *buf
 * @param {unsigned short} len
 * @param {unsigned char} retrans
 * @param {unsigned char} priority
 * @return {*}
 */
void half_duplex_bus_transmitter_cache(half_duplex_bus_t *bus, unsigned char *buf, unsigned short len, unsigned char retrans, unsigned char priority)
{
    ASSERT(bus);
    ASSERT(buf);
    if (bus->ops->lock) {
        bus->ops->lock();
    }
    uint8_t *txbuf = malloc(len + sizeof(half_duplex_bus_trans_node_t)); // 申请一个内存地址，前面是node，后面是数据
    if (txbuf == NULL) {
        return;
    }
    memset(txbuf, 0, len + sizeof(half_duplex_bus_trans_node_t));
    ((half_duplex_bus_trans_node_t *)(txbuf))->random_ms = NULL;
    ((half_duplex_bus_trans_node_t *)(txbuf))->len = len;
    ((half_duplex_bus_trans_node_t *)(txbuf))->priority = priority;
    ((half_duplex_bus_trans_node_t *)(txbuf))->retrans_max = retrans;
    ((half_duplex_bus_trans_node_t *)(txbuf))->retrans_count = 0;
    list_add_tail(&((half_duplex_bus_trans_node_t *)(txbuf))->node, &bus->trans.root);
    if (bus->ops->unlock) {
        bus->ops->unlock();
    }

    memcpy(txbuf + sizeof(half_duplex_bus_trans_node_t), buf, len);
    TRACE("[half_duplex_bus_transmitter_cache]Used memory= %d.Total memory size = %d.", heap_get_used_size(), MEMORY_POOL_SIZE);
}

/**
 * @brief 传输完成，删除当前cache，用于判断ack回复包接收到之后停止重发的场景
 * @param {*}
 * @return {*}
 */
void half_duplex_bus_transmit_cache_complete(half_duplex_bus_t *bus)
{
    ASSERT(bus);
    if (bus->trans.cur_node != NULL) {
        if (bus->ops->lock) {
            bus->ops->lock();
        }
        half_duplex_bus_trans_node_t *node = bus->trans.cur_node;
        list_del(&node->node);
        free(node);
        bus->trans.cur_node = NULL;
        if (bus->ops->unlock) {
            bus->ops->unlock();
        }
    }
}

/*---------- end of file ----------*/
