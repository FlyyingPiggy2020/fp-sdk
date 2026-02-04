/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : serial_bus.c
 * @Author       : lxf
 * @Date         : 2025-08-02 15:03:39
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-01-28 16:00:00
 * @Brief        : 串行总线统一接口实现
 * 1. 统一的API, 通过 write_buf 的不同实现支持全双工和半双工
 * 2. 支持优先级队列、重发机制、ACK停止重发
 * 3. 支持总线忙避让 (write_buf 返回 <0 时不推进发送状态)
 */

/*---------- includes ----------*/
#include "serial_bus.h"
#include "stdlib.h"
#include "string.h"
#include "options.h"
#include "clists.h"

/*---------- macro ----------*/

/*---------- type define ----------*/

/**
 * @brief 发送节点结构
 */
struct serial_bus_trans_node {
    struct list_head node; /* 链表节点 */
    uint8_t trans_cnt;     /* 当前发送次数 */
    uint8_t trans_max_cnt; /* 最大发送次数 */
    uint16_t data_len;     /* 数据长度 */
    uint8_t payload[];     /* 柔性数组,存储实际数据 */
};

/**
 * @brief 串行总线结构体 (不透明指针)
 */
struct serial_bus {
    int (*write_buf)(uint8_t *buf, uint16_t len);      /* 发送函数 */
    struct serial_bus_cb *cb;                          /* 回调函数 */
    struct serial_bus_ops *ops;                        /* 操作接口指针 */
    struct list_head tx_list[SERIAL_BUS_PRIORITY_MAX]; /* 发送队列 */
    struct serial_bus_trans_node *cur_tx_node;         /* 当前发送节点 */
    uint16_t tx_delay;                                 /* 默认发送延时 */
    uint16_t tx_delay_count;                           /* 延时计数器 */
};

/*---------- variable prototype ----------*/

/*---------- function prototype ----------*/

/*---------- variable ----------*/

/*---------- function ----------*/

/**
 * @brief 创建串行总线实例
 * @param ops: 操作接口
 * @param cb: 回调函数
 * @param default_delay: 默认发送延时(ms)
 * @return: 总线句柄
 */
serial_bus_t *serial_bus_new(struct serial_bus_ops *ops, struct serial_bus_cb *cb, uint16_t default_delay)
{
    struct serial_bus *self = NULL;

    do {
        if (ops == NULL || ops->write_buf == NULL) {
            xlog_count("serial_bus: ops is NULL or write_buf is NULL.");
            break;
        }

        self = (struct serial_bus *)malloc(sizeof(struct serial_bus));
        if (self == NULL) {
            xlog_count("serial_bus: malloc failed.");
            break;
        }

        memset(self, 0, sizeof(struct serial_bus));
        self->write_buf = ops->write_buf;
        self->cb = cb;
        self->ops = ops; /* 保存 ops 指针, 确保生命周期 */
        self->tx_delay = default_delay;

        /* 初始化发送队列 */
        for (uint8_t i = 0; i < SERIAL_BUS_PRIORITY_MAX; i++) {
            INIT_LIST_HEAD(&self->tx_list[i]);
        }

        xlog_count("serial_bus: created success. ptr=%p", self);
    } while (0);

    return self;
}

/**
 * @brief 销毁总线实例
 * @param self: 总线句柄
 */
void serial_bus_delete(serial_bus_t *self)
{
    if (self == NULL) {
        return;
    }

    /* 清空所有发送队列 */
    serial_bus_clear_txlist(self);

    free(self);
    xlog_count("serial_bus: deleted.");
}

/**
 * @brief 发送数据到缓存
 * @param self: 总线句柄
 * @param buf: 待发送数据
 * @param len: 数据长度
 * @param priority: 优先级
 * @param trans_cnt: 重发次数
 * @return: 0=成功, <0=失败
 */
int serial_bus_send(serial_bus_t *self, const uint8_t *buf, uint16_t len, uint8_t priority, uint8_t trans_cnt)
{
    int ret = -1;

    do {
        if (self == NULL) {
            break;
        }

        if (buf == NULL || len == 0) {
            break;
        }

        if (priority >= SERIAL_BUS_PRIORITY_MAX) {
            xlog_count("serial_bus: priority out of range.");
            break;
        }

        if (trans_cnt == 0) {
            xlog_count("serial_bus: trans_cnt is 0, no send.");
            break;
        }

        /* 分配发送节点内存 */
        struct serial_bus_trans_node *node =
            (struct serial_bus_trans_node *)malloc(sizeof(struct serial_bus_trans_node) + len);
        if (node == NULL) {
            xlog_count("serial_bus: malloc node failed.");
            break;
        }

        memset(node, 0, sizeof(struct serial_bus_trans_node) + len);
        node->trans_cnt = 0;
        node->trans_max_cnt = trans_cnt;
        node->data_len = len;
        memcpy(node->payload, buf, len);

        /* 添加到对应优先级队列尾部 */
        list_add_tail(&node->node, &self->tx_list[priority]);

        ret = 0;
    } while (0);

    return ret;
}

/**
 * @brief 停止当前重发
 * @param self: 总线句柄
 */
void serial_bus_stop_retrans(serial_bus_t *self)
{
    if (self == NULL) {
        return;
    }

    if (self->cur_tx_node != NULL) {
        struct serial_bus_trans_node *node = self->cur_tx_node;
        list_del(&node->node);
        free(node);
        self->cur_tx_node = NULL;
    }
}

/**
 * @brief 接收数据(在中断中调用)
 * @param self: 总线句柄
 * @param buf: 接收到的数据
 * @param len: 数据长度
 */
void serial_bus_receive(serial_bus_t *self, const uint8_t *buf, uint16_t len)
{
    do {
        if (self == NULL) {
            break;
        }

        if (buf == NULL || len == 0) {
            break;
        }

        if (self->cb == NULL || self->cb->recv_callback == NULL) {
            break;
        }

        /* 调用用户接收回调 */
        /* trans参数传NULL, trans_len传0 (统一使用中断接收) */
        self->cb->recv_callback((uint8_t *)buf, len, NULL, 0);

    } while (0);
}

/**
 * @brief 周期性处理函数
 * @param self: 总线句柄
 * @note  处理发送队列和重发逻辑, 支持总线忙避让
 */
void serial_bus_poll(serial_bus_t *self)
{
    do {
        if (self == NULL) {
            break;
        }

        /* 延时计数 */
        if (self->tx_delay_count > 0) {
            self->tx_delay_count--;
            break;
        }

        /* 查找最高优先级的待发送节点(支持抢占) */
        struct serial_bus_trans_node *node = NULL;
        uint8_t i = SERIAL_BUS_PRIORITY_MAX - 1;

        do {
            if (!list_empty(&self->tx_list[i])) {
                node = list_first_entry(&self->tx_list[i], struct serial_bus_trans_node, node);
                break;
            }
            i--;
        } while (i > 0);

        if (node == NULL) {
            break;
        }

        /* 发送数据, 只有 write_buf 返回 0 才推进状态 */
        if (node->trans_cnt < node->trans_max_cnt || node->trans_max_cnt == SERIAL_BUS_TRANS_FOREVER) {
            int ret = self->write_buf(node->payload, node->data_len);

            if (ret == 0) {
                /* 发送成功, 推进状态 */
                node->trans_cnt++;
                self->cur_tx_node = node;

                /* 计算下次发送延时 */
                if (self->cb && self->cb->get_delay_ms) {
                    self->tx_delay_count = self->cb->get_delay_ms(node->trans_cnt, node->trans_max_cnt);
                } else if (self->ops && self->ops->send_delay) {
                    /* 使用用户提供的延时函数 */
                    self->ops->send_delay(self->tx_delay);
                } else {
                    /* 使用内部计数器 */
                    self->tx_delay_count = self->tx_delay;
                }

                /* 检查是否完成重发 */
                if (node->trans_cnt >= node->trans_max_cnt && node->trans_max_cnt != SERIAL_BUS_TRANS_FOREVER) {
                    list_del(&node->node);
                    free(node);
                    self->cur_tx_node = NULL;
                }
            } else {
                /* 发送失败(总线忙), 不推进状态, 下次 poll 继续尝试 */
                if (self->ops && self->ops->send_delay) {
                    self->ops->send_delay(1); /* 短暂延时后重试 */
                }
                /* tx_delay_count 保持为 0, 下次 poll 立即重试 */
            }
        }
    } while (0);
}

/**
 * @brief 清空发送队列
 * @param self: 总线句柄
 */
void serial_bus_clear_txlist(serial_bus_t *self)
{
    if (self == NULL) {
        return;
    }

    struct serial_bus_trans_node *pos = NULL, *n = NULL;

    /* 清空所有优先级队列 */
    for (uint8_t i = 0; i < SERIAL_BUS_PRIORITY_MAX; i++) {
        list_for_each_entry_safe(pos, n, struct serial_bus_trans_node, &self->tx_list[i], node)
        {
            list_del(&pos->node);
            free(pos);
        }
    }

    /* 清空当前发送节点 */
    if (self->cur_tx_node != NULL) {
        struct serial_bus_trans_node *node = self->cur_tx_node;
        list_del(&node->node);
        free(node);
        self->cur_tx_node = NULL;
    }
}

/*---------- end of file ----------*/
