/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : analog.c
 * @Author       : lxf
 * @Date         : 2025-05-23 09:49:24
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-05-23 10:14:39
 * @Brief        : adc抽象驱动框架
 */

/*---------- includes ----------*/
#include "drv_err.h"
#include "driver.h"
#include "analog.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t analog_open(driver_t **pdrv);
static void analog_close(driver_t **pdrv);
static int32_t analog_irq_handler(driver_t **pdrv, uint32_t irq, void *args, uint32_t length);
static int32_t analog_ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static int32_t _ioctl_enable(analog_describe_t *pdesc, void *args);
static int32_t _ioctl_disable(analog_describe_t *pdesc, void *args);
static int32_t _ioctl_get(analog_describe_t *pdesc, void *args);
static int32_t _ioctl_set_irq_handler(analog_describe_t *pdesc, void *args);
/*---------- variable ----------*/
DRIVER_DEFINED(analog, analog_open, analog_close, NULL, NULL, analog_ioctl, analog_irq_handler);
const static struct protocol_callback ioctl_cbs[] = {
    { IOCTL_ANALOG_ENABLE, _ioctl_enable },
    { IOCTL_ANALOG_DISABLE, _ioctl_disable },
    { IOCTL_ANALOG_GET, _ioctl_get },
    { IOCTL_ANALOG_SET_IRQ_HANDLER, _ioctl_set_irq_handler },
};
/*---------- function ----------*/
static int32_t analog_open(driver_t **pdrv)
{
    analog_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_WRONG_ARGS;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if (pdesc) {
        err = DRV_ERR_OK;
        if (pdesc->ops.init) {
            err = (pdesc->ops.init() ? DRV_ERR_OK : DRV_ERR_ERROR);
        }
    }

    return err;
}

static void analog_close(driver_t **pdrv)
{
    analog_describe_t *pdesc = NULL;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if (pdesc && pdesc->ops.deinit) {
        pdesc->ops.deinit();
    }
}

static int32_t analog_irq_handler(driver_t **pdrv, uint32_t irq, void *args, uint32_t length)
{
    analog_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_EOK;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if (pdesc && pdesc->ops.irq_handler) {
        err = pdesc->ops.irq_handler(irq, args, length);
    }

    return err;
}

static int32_t _ioctl_enable(analog_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_ERROR;

    if (pdesc->ops.enable) {
        err = (pdesc->ops.enable(true) ? DRV_ERR_OK : DRV_ERR_ERROR);
    }

    return err;
}

static int32_t _ioctl_disable(analog_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_ERROR;

    if (pdesc->ops.enable) {
        err = (pdesc->ops.enable(false) ? DRV_ERR_OK : DRV_ERR_ERROR);
    }

    return err;
}

static int32_t _ioctl_get(analog_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_ERROR;
    union analog_ioctl_param *param = (union analog_ioctl_param *)args;

    do {
        if (!param) {
            break;
        }
        if (param->get.channel >= pdesc->number_of_channels) {
            break;
        }
        if (!pdesc->ops.get) {
            break;
        }
        param->get.data = pdesc->ops.get(param->get.channel);
        err = DRV_ERR_OK;
    } while (0);

    return err;
}

static int32_t _ioctl_set_irq_handler(analog_describe_t *pdesc, void *args)
{
    pdesc->ops.irq_handler = (int32_t(*)(uint32_t, void *, uint32_t))args;

    return DRV_ERR_OK;
}

static int32_t analog_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    analog_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_WRONG_ARGS;
    int32_t (*cb)(analog_describe_t *, void *) = NULL;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if (!pdesc) {
            break;
        }
        cb = protocol_callback_find(cmd, (void *)ioctl_cbs, ARRAY_SIZE(ioctl_cbs));
        if (!cb) {
            break;
        }
        err = cb(pdesc, args);
    } while (0);

    return err;
}
/*---------- end of file ----------*/
