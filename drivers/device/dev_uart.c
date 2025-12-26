/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : dev_uart.c
 * @Author       : lxf
 * @Date         : 2025-12-24 09:08:16
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-01-22 11:04:59
 * @Brief        : UART设备驱动实现
 * @features     :
 *               - 非阻塞异步收发(基于HAL中断模式)
 *               - 乒乓缓冲区管理(支持连续收发)
 *               - RS485软件收发切换
 *               - IDLE中断接收(支持变长数据包)
 *               - 发送完成自动流控(支持队列发送)
 *               - 三种接收模式: 标准/零拷贝/流式 (互斥使用，自动冲突检测)
 *
 * @usage        :
 *               @code
 *               device_t *uart = device_open("usart2");
 *               if (!uart) {
 *                   return -1;
 *               }
 *
 *               // ========== 方式1: 标准模式(带拷贝,简单) ==========
 *               // 发送数据(非阻塞)
 *               uint8_t tx_buf[] = "Hello UART!";
 *               device_write(uart, tx_buf, 0, sizeof(tx_buf));
 *
 *               // 接收数据(非阻塞,自动释放缓冲区)
 *               uint8_t rx_buf[256];
 *               int32_t len = device_read(uart, rx_buf, 0, sizeof(rx_buf));
 *               if (len > 0) {
 *                   process_data(rx_buf, len);  // ✅ 无需调用 RX_DONE!
 *               }
 *
 *               // ========== 方式2: 零拷贝模式(高性能) ==========
 *               // 接收
 *               struct dev_uart_buf rx_buf;
 *               if (device_ioctl(uart, IOCTL_UART_GET_RX_BUF, &rx_buf) == E_OK) {
 *                   process_data(rx_buf.ptr, rx_buf.size);  // 直接使用数据
 *                   device_ioctl(uart, IOCTL_UART_RX_DONE, NULL);  // 释放缓冲区
 *               }
 *
 *               // 发送
 *               struct dev_uart_buf tx_buf;
 *               if (device_ioctl(uart, IOCTL_UART_GET_TX_BUF, &tx_buf) == E_OK) {
 *                   memcpy(tx_buf.ptr, my_data, my_len);  // 或直接配置 DMA
 *                   tx_msg->pos = my_len;  // 设置发送长度
 *                   device_ioctl(uart, IOCTL_UART_TX_DONE, NULL);  // 触发发送
 *               }
 *
 *               // ========== 方式3: 流式模式(协议栈适配,零拷贝) ==========
 *               // 逐字节读取,驱动层维护游标,跨包读取
 *               // 适用于 Modbus 等需要单字节接收的协议栈
 *               uint8_t byte;
 *               if (device_ioctl(uart, IOCTL_UART_GET_BYTE, &byte) == E_OK) {
 *                   // 成功读取一个字节
 *               }
 *
 *               device_close(uart);
 *               @endcode
 *
 * @warning  接收模式冲突检测 - 三种接收方式互斥，只能选择一种！
 *            混合使用会返回 E_BUSY / -1 错误。详见 dev_uart.h 文件头说明。
 *
 * @performance  :
 *               ┌─────────────────┬──────────────────┬─────────────────┐
 *               │     模式        │    拷贝次数      │    性能         │
 *               ├─────────────────┼──────────────────┼─────────────────┤
 *               │  标准模式(回显) │  2次(RX+TX各1次) │    一般         │
 *               │  零拷贝模式     │  1次(仅RX→TX)    │    优秀         │
 *               │  流式模式       │  0次(直接访问)   │    协议栈优化    │
 *               └─────────────────┴──────────────────┴─────────────────┘
 *
 *               标准模式: 数据流 硬件→rxpp→应用层→txpp→硬件 (2次memcpy)
 *               零拷贝模式: 数据流 硬件→rxpp→txpp→硬件 (1次memcpy)
 *               流式模式:   数据流 硬件→rxpp→协议栈 (0次memcpy,游标管理)
 *
 * @transmit_flow :
 *               device_write("ABC")
 *               └─> memcpy(拷贝到txpp乒乓缓冲) [第1次拷贝]
 *               └─> ptransmitter==NULL? 立即发送 : 排队等待
 *                   └─> HAL_UART_Transmit_IT("ABC")
 *                       └─> TC中断
 *                           └─> HAL_UART_TxCpltCallback()
 *                               └─> device_irq_process(TX_COMPLETE)
 *                                   └─> 释放缓冲区 + 启动下一次发送
 *
 * @receive_flow  :
 *               HAL_UARTEx_ReceiveToIdle_IT(启动接收)
 *               └─> 硬件接收数据到rxpp->pbuf
 *               └─> IDLE中断或缓冲区满
 *                   └─> HAL_UARTEx_RxEventCallback()
 *                       └─> device_irq_process(RX_COMPLETE)
 *                           └─> 切换乒乓缓冲区, 驱动层自动重启接收
 *                               └─> 应用层获取数据:
 *                                   ├─ 标准模式: device_read() memcpy到应用层 [第2次拷贝]
 *                                   ├─ 零拷贝模式: IOCTL_GET_RX_BUF 直接访问rxpp
 *                                   │           └─> device_ioctl(RX_DONE) 释放缓冲区
 *                                   └─ 流式模式: IOCTL_GET_BYTE 游标读取,跨包消费
 */

/*---------- includes ----------*/
#include "options.h"
#include "dev_uart.h"
#include "driver.h"
#include "device.h"
/*---------- macro ----------*/

/* 默认配置 */
#if !defined(CONFIG_UART_BUF_SIZE_RX)
#define CONFIG_UART_BUF_SIZE_RX 256
#endif

#if !defined(CONFIG_UART_BUF_SIZE_TX)
#define CONFIG_UART_BUF_SIZE_TX 256
#endif
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static fp_err_t _dev_uart_open(driver_t **pdrv);
static void _dev_uart_close(driver_t **pdrv);
static int32_t _dev_uart_write(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len);
static int32_t _dev_uart_read(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len);
static fp_err_t _dev_uart_ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static fp_err_t _dev_uart_irq_handler(driver_t **pdrv, uint32_t irq_event, void *args, uint32_t length);

/* 内部辅助函数 - IOCTL 查表回调 */
static fp_err_t ioctl_get_rx_buf(struct dev_uart_describe *self, void *arg);
static fp_err_t ioctl_get_tx_buf(struct dev_uart_describe *self, void *arg);
static fp_err_t ioctl_rx_done(struct dev_uart_describe *self, void *arg);
static fp_err_t ioctl_tx_done(struct dev_uart_describe *self, void *arg);
static fp_err_t ioctl_get_byte(struct dev_uart_describe *self, void *arg);

/*---------- variable ----------*/
DRIVER_DEFINED(
    dev_uart, _dev_uart_open, _dev_uart_close, _dev_uart_write, _dev_uart_read, _dev_uart_ioctl, _dev_uart_irq_handler);

// clang-format off
static struct protocol_callback ioctl_cbs[] = {
    { IOCTL_UART_GET_RX_BUF, ioctl_get_rx_buf },
    { IOCTL_UART_GET_TX_BUF, ioctl_get_tx_buf },
    { IOCTL_UART_RX_DONE, ioctl_rx_done },
    { IOCTL_UART_TX_DONE, ioctl_tx_done },
    { IOCTL_UART_GET_BYTE, ioctl_get_byte },
};
// clang-format on

/*---------- function ----------*/

/**
 * @brief 打开UART设备,分配缓冲区并初始化硬件
 */
static fp_err_t _dev_uart_open(driver_t **pdrv)
{
    struct dev_uart_describe *pdesc = NULL;
    fp_err_t err = E_WRONG_ARGS;
    uint8_t *rxbuf = NULL, *txbuf = NULL;
    uint32_t rxwantsz = 0, txwantsz = 0;
    assert(pdrv != NULL);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;

    do {
        if (!pdesc) {
            break;
        }

        if (pdesc->config.flags & DEV_UART_FLAG_PKT_MODE) { /* 如果未设置缓冲区大小 */
            /* 1. 分配缓冲区 */
            err = E_NO_MEMORY;

            if (pdesc->config.bufsz_rx == 0 || pdesc->config.bufsz_tx == 0) {
                break;
            }

            /* 一次性分配两个 dev_uart_msg 结构体 */
            rxwantsz = (offsetof(struct dev_uart_msg, pbuf) + pdesc->config.bufsz_rx);
            txwantsz = (offsetof(struct dev_uart_msg, pbuf) + pdesc->config.bufsz_tx);
            rxbuf = malloc(rxwantsz * 2);
            txbuf = malloc(txwantsz * 2);
            if (!rxbuf || !txbuf) {
                free(rxbuf);
                free(txbuf);
                break;
            }

            /* 初始化乒乓缓冲，管理完整的 struct dev_uart_msg */
            pingpong_buffer_init(&pdesc->rxpp, rxbuf, rxbuf + rxwantsz);
            pingpong_buffer_init(&pdesc->txpp, txbuf, txbuf + txwantsz);
            /* preveiver 指向当前写入缓冲区 */
            pingpong_buffer_get_write_buf(&pdesc->rxpp, (void **)&pdesc->preveiver);

            /* 5. 调用底层硬件初始化 */
            if (pdesc->ops && pdesc->ops->init) {
                if (pdesc->ops->init(pdesc, &pdesc->config) != E_OK) {
                    err = E_ERROR;
                    break;
                }
            }
            if (pdesc->ops && pdesc->ops->start_receive) {
                if (pdesc->ops->start_receive(pdesc, pdesc->preveiver->pbuf, pdesc->config.bufsz_rx) != E_OK) {
                    err = E_ERROR;
                    break;
                }
            }
            err = E_OK;
        }

    } while (0);

    /* 失败时释放已分配的资源 */
    if (err != E_OK) {
        if (txbuf) {
            free(txbuf);
        }
        if (rxbuf) {
            free(rxbuf);
        }
    }
    return err;
}

/**
 * @brief 关闭UART设备,释放资源
 */
static void _dev_uart_close(driver_t **pdrv)
{
    struct dev_uart_describe *pdesc = NULL;
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;

    if (pdesc) {
        if (pdesc->ops->deinit) {
            pdesc->ops->deinit(pdesc);
        }
        /* 释放缓冲区 */
        if (pdesc->config.flags & DEV_UART_FLAG_PKT_MODE) {
            free(pdesc->txpp.buffer[0]);
            free(pdesc->rxpp.buffer[0]);
        }
    }
}

/**
 * @brief 串口发送
 */
static int32_t _dev_uart_write(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len)
{
    struct dev_uart_describe *pdesc = NULL;
    uint8_t *pdata = (uint8_t *)buf;
    uint32_t i;
    fp_err_t err = E_WRONG_ARGS;

    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;

    do {
        if (!pdesc || !buf || len == 0 || !pdesc->ops) {
            break;
        }

        if (len > pdesc->config.bufsz_tx) {
            break;
        }

        /* RS485模式:切换到发送 */
        if (pdesc->config.flags & DEV_UART_FLAG_SWRS485) {
            if (pdesc->ops->control) {
                pdesc->ops->control(pdesc, UART_CTRL_SET_RS485_TX, NULL);
            }
        }

        if (pdesc->config.tx_mode == UART_TX_MODE_SYNC) {
            /* DEV_UART_FLAG_PKT_MODE: 拷贝到乒乓缓冲并触发发送 */
            if (pdesc->config.flags & DEV_UART_FLAG_PKT_MODE) {
                if (pdesc->ops->start_transmit) {
                    pdesc->ops->start_transmit(pdesc, buf, len);
                }
            }
        } else if (pdesc->config.tx_mode == UART_TX_MODE_ASYNC) {
            /* DEV_UART_FLAG_PKT_MODE: 拷贝到乒乓缓冲并触发发送 */
            if (pdesc->config.flags & DEV_UART_FLAG_PKT_MODE) {
                struct dev_uart_msg *tx_msg = NULL;

                /* 1. 拷贝数据到发送缓冲区 */
                pingpong_buffer_get_write_buf(&pdesc->txpp, (void **)&tx_msg);
                if (tx_msg) {
                    tx_msg->pos = len;
                    memcpy(tx_msg->pbuf, buf, len);
                    pingpong_buffer_set_write_done(&pdesc->txpp);

                    /* 2. 如果发送空闲，立即启动发送 */
                    if (pdesc->ptransmitter == NULL) {
                        pingpong_buffer_get_read_buf(&pdesc->txpp, (void **)&pdesc->ptransmitter);
                        if (pdesc->ptransmitter && pdesc->ops->start_transmit) {
                            pdesc->ops->start_transmit(pdesc, pdesc->ptransmitter->pbuf, pdesc->ptransmitter->pos);
                        }
                    }
                }
            }
        }

        err = E_OK;
    } while (0);

    return E_OK;
}

/**
 * @brief 从接收缓冲区读取数据(带拷贝模式)
 * @note  读取后自动释放缓冲区,无需调用 RX_DONE
 * @retval >0      读取的字节数
 * @retval 0       无数据
 * @retval -1      接收模式冲突(已使用其他接收模式)
 */
static int32_t _dev_uart_read(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len)
{
    struct dev_uart_describe *pdesc = NULL;
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    int32_t retval = 0;

    do {
        if (!pdesc || !buf) {
            break;
        }

        /* 检测接收模式冲突 */
        if (pdesc->config.rx_mode != UART_RX_MODE_NONE && pdesc->config.rx_mode != UART_RX_MODE_STANDARD) {
            retval = -1; /* 模式冲突 */
            break;
        }
        pdesc->config.rx_mode = UART_RX_MODE_STANDARD;

        /* PKT_MODE: 从乒乓缓冲区读取数据 */
        if (pdesc->config.flags & DEV_UART_FLAG_PKT_MODE) {
            struct dev_uart_msg *read_msg = NULL;

            /* 尝试获取读取缓冲区 */
            if (pingpong_buffer_get_read_buf(&pdesc->rxpp, (void **)&read_msg)) {
                /* 计算实际拷贝长度 */
                uint32_t copy_len = (read_msg->pos < len) ? read_msg->pos : len;

                /* 拷贝数据到应用层缓冲区 */
                memcpy(buf, read_msg->pbuf, copy_len);
                retval = copy_len;

                /* 自动释放已读缓冲区,无需应用层调用 RX_DONE */
                pingpong_buffer_set_read_done(&pdesc->rxpp);
            }
        }

    } while (0);

    return retval;
}

/**
 * @brief IOCTL命令处理
 */
static fp_err_t _dev_uart_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    fp_err_t err = E_WRONG_ARGS;
    struct dev_uart_describe *pdesc = NULL;
    fp_err_t (*cb)(struct dev_uart_describe *, void *) = NULL;

    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;

    do {
        if (!pdesc) {
            break;
        }

        /* 查表找到对应的回调函数 */
        cb = (fp_err_t (*)(struct dev_uart_describe *, void *))protocol_callback_find(
            cmd, ioctl_cbs, ARRAY_SIZE(ioctl_cbs));
        if (!cb) {
            break;
        }

        /* 执行回调 */
        err = cb(pdesc, args);

    } while (0);

    return err;
}

/**
 * @brief UART中断事件处理
 * @param irq_event 中断事件类型(UART_EVENT_XXX)
 */
static fp_err_t _dev_uart_irq_handler(driver_t **pdrv, uint32_t irq_event, void *args, uint32_t length)
{
    struct dev_uart_describe *pdesc = NULL;
    fp_err_t err = E_WRONG_ARGS;

    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;

    do {
        if (!pdesc) {
            break;
        }

        /* 接收完成(所有数据接收完成或缓冲区满) */
        if (irq_event == DEV_UART_EVENT_RX_COMPLETE) {
            /* 切换乒乓缓冲区 */
            if (pdesc->config.flags & DEV_UART_FLAG_PKT_MODE) {
                pdesc->preveiver->pos = length;
                pingpong_buffer_set_write_done(&pdesc->rxpp);
                pingpong_buffer_get_write_buf(&pdesc->rxpp, (void **)&pdesc->preveiver);

                /* 驱动层主动重启接收 */
                if (pdesc->ops->start_receive) {
                    pdesc->ops->start_receive(pdesc, pdesc->preveiver->pbuf, pdesc->config.bufsz_rx);
                }
            }
        } else if (irq_event == DEV_UART_EVENT_RX_ERROR) {
            /* 接收错误: 重启接收(不切换缓冲区) */
            if (pdesc->config.flags & DEV_UART_FLAG_PKT_MODE) {
                if (pdesc->ops->start_receive) {
                    pdesc->ops->start_receive(pdesc, pdesc->preveiver->pbuf, pdesc->config.bufsz_rx);
                }
            }
        } else if (irq_event == DEV_UART_EVENT_TX_COMPLETE) {
            /* 发送完成: 释放已发送缓冲区并启动下一次发送 */
            if (pdesc->config.flags & DEV_UART_FLAG_PKT_MODE) {
                /* 1. 释放已发送的缓冲区 */
                pingpong_buffer_set_read_done(&pdesc->txpp);
                pdesc->ptransmitter = NULL;

                /* 2. RS485模式: 切换回接收 */
                if (pdesc->config.flags & DEV_UART_FLAG_SWRS485) {
                    if (pdesc->ops->control) {
                        pdesc->ops->control(pdesc, UART_CTRL_SET_RS485_RX, NULL);
                    }
                }

                /* 3. 自动启动下一次发送(如果有待发送数据) */
                pingpong_buffer_get_read_buf(&pdesc->txpp, (void **)&pdesc->ptransmitter);
                if (pdesc->ptransmitter && pdesc->ops->start_transmit) {
                    /* RS485模式: 再次切换到发送 */
                    if (pdesc->config.flags & DEV_UART_FLAG_SWRS485) {
                        if (pdesc->ops->control) {
                            pdesc->ops->control(pdesc, UART_CTRL_SET_RS485_TX, NULL);
                        }
                    }
                    pdesc->ops->start_transmit(pdesc, pdesc->ptransmitter->pbuf, pdesc->ptransmitter->pos);
                }
            }
        }

        err = E_OK;
    } while (0);

    return err;
}

/**
 * @brief IOCTL: 获取接收缓冲区(零拷贝模式)
 * @retval E_OK     成功
 * @retval E_WRONG_ARGS 参数错误
 * @retval E_BUSY   接收模式冲突(已使用其他接收模式)
 */
static fp_err_t ioctl_get_rx_buf(struct dev_uart_describe *self, void *arg)
{
    fp_err_t err = E_WRONG_ARGS;

    do {
        if (!self || !arg) {
            break;
        }

        /* 检测接收模式冲突 */
        if (self->config.rx_mode != UART_RX_MODE_NONE && self->config.rx_mode != UART_RX_MODE_ZERO_COPY) {
            err = E_BUSY;
            break;
        }
        self->config.rx_mode = UART_RX_MODE_ZERO_COPY;

        if (self->config.flags & DEV_UART_FLAG_PKT_MODE) {
            struct dev_uart_msg *read_msg = NULL;
            if (pingpong_buffer_get_read_buf(&self->rxpp, (void **)&read_msg)) {
                struct dev_uart_buf *buf = (struct dev_uart_buf *)arg;
                buf->ptr = read_msg->pbuf;
                buf->size = read_msg->pos;
                err = E_OK;
            }
        }
    } while (0);

    return err;
}

/**
 * @brief IOCTL: 获取发送缓冲区(零拷贝模式)
 */
static fp_err_t ioctl_get_tx_buf(struct dev_uart_describe *self, void *arg)
{
    fp_err_t err = E_WRONG_ARGS;

    do {
        if (!self || !arg) {
            break;
        }

        if (self->config.flags & DEV_UART_FLAG_PKT_MODE) {
            struct dev_uart_msg *tx_msg = NULL;
            if (pingpong_buffer_get_write_buf(&self->txpp, (void **)&tx_msg)) {
                struct dev_uart_buf *buf = (struct dev_uart_buf *)arg;
                buf->ptr = tx_msg->pbuf;
                buf->size = self->config.bufsz_tx;
                err = E_OK;
            }
        }
    } while (0);

    return err;
}

/**
 * @brief IOCTL: 接收完成(零拷贝模式-释放缓冲区)
 */
static fp_err_t ioctl_rx_done(struct dev_uart_describe *self, void *arg)
{
    fp_err_t err = E_WRONG_ARGS;

    do {
        if (!self) {
            break;
        }

        if (self->config.flags & DEV_UART_FLAG_PKT_MODE) {
            pingpong_buffer_set_read_done(&self->rxpp);
            err = E_OK;
        }
    } while (0);

    return err;
}

/**
 * @brief IOCTL: 发送完成(零拷贝模式-触发发送)
 */
static fp_err_t ioctl_tx_done(struct dev_uart_describe *self, void *arg)
{
    fp_err_t err = E_WRONG_ARGS;

    do {
        if (!self) {
            break;
        }

        if (self->config.flags & DEV_UART_FLAG_PKT_MODE) {
            struct dev_uart_msg *tx_msg = NULL;
            pingpong_buffer_get_write_buf(&self->txpp, (void **)&tx_msg);
            if (tx_msg) {
                /* 标记写入完成 */
                pingpong_buffer_set_write_done(&self->txpp);

                /* 启动发送 */
                if (self->ptransmitter == NULL) {
                    pingpong_buffer_get_read_buf(&self->txpp, (void **)&self->ptransmitter);
                    if (self->ptransmitter && self->ops->start_transmit) {
                        self->ops->start_transmit(self, self->ptransmitter->pbuf, self->ptransmitter->pos);
                    }
                }
                err = E_OK;
            }
        }
    } while (0);

    return err;
}

/**
 * @brief IOCTL: 获取单字节(流式模式-用于协议栈)
 * @note   驱动层维护游标，跨包读取，适合 Modbus 等需要单字节接收的协议栈
 * @retval E_OK     成功读取一个字节
 * @retval E_EMPTY   无数据可读
 * @retval E_BUSY   接收模式冲突(已使用其他接收模式)
 */
static fp_err_t ioctl_get_byte(struct dev_uart_describe *self, void *arg)
{
    fp_err_t err = E_EMPTY;
    struct dev_uart_msg *msg = NULL;
    uint8_t *byte = (uint8_t *)arg;

    do {
        if (!self || !byte) {
            err = E_WRONG_ARGS;
            break;
        }

        /* 检测接收模式冲突 */
        if (self->config.rx_mode != UART_RX_MODE_NONE && self->config.rx_mode != UART_RX_MODE_STREAM) {
            err = E_BUSY;
            break;
        }
        self->config.rx_mode = UART_RX_MODE_STREAM;

        if (!(self->config.flags & DEV_UART_FLAG_PKT_MODE)) {
            break;
        }

        /* 1. 如果没有当前消息，尝试获取一个 */
        if (self->stream.current_msg == NULL) {
            if (!pingpong_buffer_get_read_buf(&self->rxpp, (void **)&msg)) {
                err = E_EMPTY; /* 无数据 */
                break;
            }
            self->stream.current_msg = msg;
            self->stream.read_pos = 0;
        }

        /* 2. 检查当前消息是否读完 */
        if (self->stream.read_pos >= self->stream.current_msg->pos) {
            /* 读完这个包，释放并尝试获取下一个 */
            pingpong_buffer_set_read_done(&self->rxpp);
            self->stream.current_msg = NULL;

            if (!pingpong_buffer_get_read_buf(&self->rxpp, (void **)&msg)) {
                err = E_EMPTY; /* 无更多数据 */
                break;
            }
            self->stream.current_msg = msg;
            self->stream.read_pos = 0;
        }

        /* 3. 读取一个字节，游标后移 */
        *byte = self->stream.current_msg->pbuf[self->stream.read_pos++];
        err = E_OK;

    } while (0);

    return err;
}

/*---------- end of file ----------*/
