/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : dev_can.c
 * @Author       : lxf
 * @Date         : 2025-12-11 14:26:04
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-12-15 14:20:34
 * @Brief        : CAN总线驱动
 */

/*---------- includes ----------*/
#include "options.h"
#include "dev_can.h"
#include "driver.h"
#include "device.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static fp_err_t _dev_can_open(driver_t **pdrv);
static void _dev_can_close(driver_t **pdrv);
static fp_err_t _dev_can_write(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len);
static fp_err_t _dev_can_read(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len);
static fp_err_t _dev_can_ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static fp_err_t _dev_can_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length);

static fp_err_t dev_can_rx_done(struct dev_can_describe *self, void *arg);

/*---------- variable ----------*/
DRIVER_DEFINED(
    dev_can, _dev_can_open, _dev_can_close, _dev_can_write, _dev_can_read, _dev_can_ioctl, _dev_can_irq_handler);

static struct protocol_callback ioctl_cbs[] = {
    { IOCTL_CAN_RXDONE, dev_can_rx_done },
};
/*---------- function ----------*/
static fp_err_t _dev_can_open(driver_t **pdrv)
{
    struct dev_can_describe *pdesc = NULL;
    fp_err_t err = E_WRONG_ARGS;

    assert(pdrv != NULL);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;

    do {
        if (!pdesc) {
            break;
        }
        if (!pdesc->config.msgboxsz) {
            break;
        }
        if (!pdesc->ops->init) {
            break;
        }
        memset(&pdesc->rx_msg, 0, sizeof(struct can_msg_list));
        pdesc->rx_msg.msg = malloc(sizeof(struct can_msg) * pdesc->config.msgboxsz);
        if (!pdesc->rx_msg.msg) {
            err = E_NO_MEMORY;
            break;
        } else {
            memset(pdesc->rx_msg.msg, 0, sizeof(struct can_msg) * pdesc->config.msgboxsz);
            pdesc->rx_msg.max = pdesc->config.msgboxsz;
        }
        err = pdesc->ops->init(pdesc, &pdesc->config);
    } while (0);

    if (err != E_OK && pdesc->rx_msg.msg) {
        free(pdesc->rx_msg.msg);
    }
    return err;
}
static void _dev_can_close(driver_t **pdrv)
{
    do {

    } while (0);
}
static fp_err_t _dev_can_write(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len)
{
    struct dev_can_describe *pdesc = NULL;
    fp_err_t err = E_WRONG_ARGS;

    assert(buf != NULL);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;

    do {
        if (!pdesc) {
            break;
        }
        if (!buf) {
            break;
        }

        if (pdesc->ops->sendmsg) {
            err = pdesc->ops->sendmsg(pdesc, buf);
        }
    } while (0);

    return err;
}
static fp_err_t _dev_can_read(driver_t **pdrv, void *buf, uint32_t addition, uint32_t len)
{
    struct dev_can_describe *pdesc = NULL;
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    fp_err_t err = E_WRONG_ARGS;
    do {
        if (!pdesc) {
            break;
        }
        if (!buf) {
            break;
        }
        if (pdesc->rx_msg.count == 0) {
            err = E_EMPTY;
            break;
        }
        memcpy(buf, &pdesc->rx_msg.msg[pdesc->rx_msg.read], sizeof(struct can_msg));
        pdesc->rx_msg.read++;
        pdesc->rx_msg.count--;
        if (pdesc->rx_msg.read == pdesc->rx_msg.max) {
            pdesc->rx_msg.read = 0;
        }
        err = E_OK;
    } while (0);
    return err;
}
static fp_err_t _dev_can_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    fp_err_t err = E_WRONG_ARGS;
    struct dev_can_describe *pdesc = NULL;
    fp_err_t (*cb)(struct dev_can_describe *, void *) = NULL;

    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if (!pdesc) {
            break;
        }
        cb = (int32_t (*)(struct dev_can_describe *, void *))protocol_callback_find(
            cmd, ioctl_cbs, ARRAY_SIZE(ioctl_cbs));
        if (!cb) {
            break;
        }
        err = cb(pdesc, args);
    } while (0);

    return err;
}
static fp_err_t _dev_can_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length)
{
    struct dev_can_describe *pdesc = NULL;
    fp_err_t err = E_WRONG_ARGS;
    assert(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if (pdesc) {
    }
    return err;
}

static fp_err_t dev_can_rx_done(struct dev_can_describe *self, void *arg)
{
    fp_err_t err = E_WRONG_ARGS;

    do {
        if (!arg) {
            break;
        }

        if (self->rx_msg.count == self->rx_msg.max) {
            err = E_FULL;
            break;
        }

        memcpy(&self->rx_msg.msg[self->rx_msg.write], arg, sizeof(struct can_msg));
        self->rx_msg.count++;
        self->rx_msg.write++;
        if (self->rx_msg.write == self->rx_msg.max) {
            self->rx_msg.write = 0;
        }
    } while (0);
    return err;
}
/*---------- end of file ----------*/
