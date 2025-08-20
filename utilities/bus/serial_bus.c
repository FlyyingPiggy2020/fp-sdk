/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : serial_bus.c
 * @Author       : lxf
 * @Date         : 2025-08-02 15:03:39
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-08-02 15:04:23
 * @Brief        :
 */

/*---------- includes ----------*/
#include "serial_bus.h"

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

struct serial_bus_handle *serial_bus_new(struct serial_duplex_bus_ops *ops, struct serial_duplex_bus_cb *cb)
{
    struct serial_bus_handle *self = NULL;
    self = malloc(sizeof(struct serial_bus_handle));

    do {
        if (ops == NULL) {
            break;
        }

        if (self == NULL) {
            break;
        }
        memset(self, 0, sizeof(*self));
        self->ops = ops;
        self->cb = cb;
        self->tx_delay = 200; // default 200 ticks
        for (uint8_t i = 0; i < SERIAL_BUS_PRIORITY_MAX; i++) {
            INIT_LIST_HEAD(&self->tx_list[i]);
        }
    } while (0);

    return self;
}

void serial_bus_delete(struct serial_bus_handle *self)
{
    do {
        if (self == NULL) {
            break;
        }
        struct serial_bus_transport_node *pos = NULL, *n = NULL;
        for (uint8_t i = 0; i < SERIAL_BUS_PRIORITY_MAX; i++) {
            list_for_each_entry_safe(pos, n, struct serial_bus_transport_node, &self->tx_list[i], node)
            {
                list_del(&n->node);
                free(n);
            }
        }
        free(self);
    } while (0);
}

void serial_transport_poll(struct serial_bus_handle *self)
{
    do {
        if (self == NULL) {
            break;
        }
        if (self->tx_delay_count) {
            self->tx_delay_count--;
            break;
        }
        struct serial_bus_transport_node *n = NULL;
        uint8_t i = SERIAL_BUS_PRIORITY_MAX - 1;
        do {
            if (!list_empty(&self->tx_list[i])) {
                n = list_first_entry(&(self->tx_list[i]), struct serial_bus_transport_node, node);
                break;
            }
            i--;
        } while (i);

        if (n == NULL) {
            break;
        }

        if (self->ops && self->ops->get_delay_ms) {
            self->tx_delay_count = self->ops->get_delay_ms(*n);
        } else {
            self->tx_delay_count = self->tx_delay;
        }

        do {
            if (n->trans_cnt > 0) {
                n->trans_cnt--;
                self->ops->write_buf(n->payload, n->data_len);
                if (n->trans_cnt == 0) {
                    list_del(&n->node);
                    free(n);
                    n = NULL;
                }
                self->cur_tx_node = n; // 记录当前发送的node
                break;
            }
        } while (0);
    } while (0);
}

void serial_bus_send_to_cache(struct serial_bus_handle *self, const uint8_t *pdata, uint32_t length, uint8_t priority, uint8_t trans_cnt)
{
    do {
        if (self == NULL) {
            break;
        }

        if (pdata == NULL || length == 0) {
            break;
        }

        if (priority >= SERIAL_BUS_PRIORITY_MAX) {
            break;
        }

        if (trans_cnt == 0) {
            break;
        }

        struct serial_bus_transport_node *node = (struct serial_bus_transport_node *)malloc(sizeof(struct serial_bus_transport_node) + length);
        if (node == NULL) {
            break;
        }
        memset(node, 0, sizeof(struct serial_bus_transport_node) + length);
        node->trans_cnt = trans_cnt;
        node->data_len = length;
        memcpy(node->payload, pdata, length);
        list_add_tail(&node->node, &self->tx_list[priority]);
    } while (0);
}

void serial_bus_stop_retrans(struct serial_bus_handle *self)
{
    do {
        if (self == NULL) {
            break;
        }

        if (self->cur_tx_node != NULL) {
            struct serial_bus_transport_node *node = self->cur_tx_node;
            list_del(&node->node);
            free(node);
            self->cur_tx_node = NULL;
        }
    } while (0);
}

void serial_bus_receive(struct serial_bus_handle *self, const uint8_t *pdata, uint32_t length)
{
    if (self->cb && self->cb->recv_phare_callback) {
        self->cb->recv_phare_callback((uint8_t *)pdata, length, self->cur_tx_node->payload, self->cur_tx_node->data_len);
    }
}
/*---------- end of file ----------*/
