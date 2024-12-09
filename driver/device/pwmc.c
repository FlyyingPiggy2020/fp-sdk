/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : pwmc.c
 * @Author       : lxf
 * @Date         : 2024-12-05 14:37:46
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-12-09 13:16:14
 * @Brief        :
 */

/*---------- includes ----------*/
#include "pwmc.h"
#include "driver.h"
#include "drv_err.h"
/*---------- macro ----------*/

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t pwmc_open(driver_t **pdrv);
static void pwmc_close(driver_t **pdrv);
static int32_t pwmc_ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static int32_t pwmc_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length);
///* ioctl callback function */
static int32_t _ioctl_enable(pwmc_describe_t *pdesc, void *args);
static int32_t _ioctl_disable(pwmc_describe_t *pdesc, void *args);
static int32_t _ioctl_get_freq(pwmc_describe_t *pdesc, void *args);
static int32_t _ioctl_set_freq(pwmc_describe_t *pdesc, void *args);
static int32_t _ioctl_get_number_of_channel(pwmc_describe_t *pdesc, void *args);
static int32_t _ioctl_get_duty(pwmc_describe_t *pdesc, void *args);
static int32_t _ioctl_set_duty(pwmc_describe_t *pdesc, void *args);
static int32_t _ioctl_get_duty_raw(pwmc_describe_t *pdesc, void *args);
static int32_t _ioctl_set_duty_raw(pwmc_describe_t *pdesc, void *args);
static int32_t _ioctl_enable_channel(pwmc_describe_t *pdesc, void *args);
static int32_t _ioctl_disable_channel(pwmc_describe_t *pdesc, void *args);
static int32_t _ioctl_set_irq(pwmc_describe_t *pdesc, void *args);
static int32_t _ioctl_get_duty_raw_max(pwmc_describe_t *pdesc, void *args);
/*---------- variable ----------*/
DRIVER_DEFINED(pwmc, pwmc_open, pwmc_close, NULL, NULL, pwmc_ioctl, pwmc_irq_handler);

static struct protocol_callback ioctl_cbs[] = {
    { IOCTL_PWMC_ENABLE, _ioctl_enable },
    { IOCTL_PWMC_DISABLE, _ioctl_disable },
    { IOCTL_PWMC_GET_FREQ, _ioctl_get_freq },
    { IOCTL_PWMC_SET_FREQ, _ioctl_set_freq },
    { IOCTL_PWMC_GET_NUMBER_OF_CHANNEL, _ioctl_get_number_of_channel },
    { IOCTL_PWMC_GET_DUTY, _ioctl_get_duty },
    { IOCTL_PWMC_SET_DUTY, _ioctl_set_duty },
    { IOCTL_PWMC_GET_DUTY_RAW, _ioctl_get_duty_raw },
    { IOCTL_PWMC_SET_DUTY_RAW, _ioctl_set_duty_raw },
    { IOCTL_PWMC_ENABLE_CHANNEL, _ioctl_enable_channel },
    { IOCTL_PWMC_DISABLE_CHANNEL, _ioctl_disable_channel },
    { IOCTL_PWMC_SET_IRQ_HANDLER, _ioctl_set_irq },
    { IOCTL_PWMC_GET_DUTY_RAW_MAX, _ioctl_get_duty_raw_max }
};
/*---------- function ----------*/
static int32_t pwmc_open(driver_t **pdrv)
{
    pwmc_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_WRONG_ARGS;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if (!pdesc) {
            break;
        }
        err = DRV_ERR_OK;
        if (pdesc->ops.init) {
            if (!pdesc->ops.init()) {
                err = -1;
            }
        }
    } while (0);

    return err;
}

static void pwmc_close(driver_t **pdrv)
{
    pwmc_describe_t *pdesc = NULL;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if (pdesc && pdesc->ops.deinit) {
        pdesc->ops.deinit();
    }
}

static int32_t pwmc_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    pwmc_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_WRONG_ARGS;
    int32_t (*cb)(pwmc_describe_t *, void *) = NULL;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if (!pdesc) {
            break;
        }
        cb = (int32_t(*)(pwmc_describe_t *, void *))protocol_callback_find(cmd, ioctl_cbs, ARRAY_SIZE(ioctl_cbs));
        if (!cb) {
            break;
        }
        err = cb(pdesc, args);
    } while (0);

    return err;
}

static int32_t pwmc_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length)
{
    pwmc_describe_t *pdesc = NULL;
    int32_t err = 0;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if (pdesc && pdesc->ops.irq_handler) {
        err = pdesc->ops.irq_handler(irq_handler, args, length);
    }

    return err;
}
///* ioctl callback function */

static int32_t _ioctl_enable(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_ERROR;

    if (pdesc->ops.enable) {
        if (pdesc->ops.enable(true)) {
            err = DRV_ERR_OK;
        }
    }

    return err;
}

static int32_t _ioctl_disable(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_ERROR;

    if (pdesc->ops.enable) {
        if (pdesc->ops.enable(false)) {
            err = DRV_ERR_OK;
        }
    }

    return err;
}

static int32_t _ioctl_get_freq(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    uint32_t *freq = (uint32_t *)args;

    if (freq) {
        *freq = pdesc->freq;
        err = DRV_ERR_OK;
    }

    return err;
}

static int32_t _ioctl_set_freq(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    uint32_t *freq = (uint32_t *)args;

    if (freq && *freq != pdesc->freq) {
        pdesc->freq = *freq;
        if (pdesc->ops.deinit) {
            pdesc->ops.deinit();
        }
        if (pdesc->ops.init) {
            pdesc->ops.init();
        }
        err = DRV_ERR_OK;
    }

    return err;
}

static int32_t _ioctl_get_number_of_channel(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    uint32_t *num = (uint32_t *)args;

    if (num) {
        *num = pdesc->number_of_channel;
        err = DRV_ERR_OK;
    }

    return err;
}

static int32_t _ioctl_get_duty(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    uint32_t raw = 0;
    union pwmc_ioctl_param *param = (union pwmc_ioctl_param *)args;

    do {
        if (!param) {
            break;
        }
        if (!pdesc->ops.get_duty_raw) {
            break;
        }
        if (!pdesc->raw_max) {
            break;
        }
        raw = pdesc->ops.get_duty_raw(param->duty.channel);
        param->duty.duty = (float)raw / (float)pdesc->raw_max;
        err = DRV_ERR_OK;
    } while (0);

    return err;
}

static int32_t _ioctl_set_duty(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    uint32_t raw = 0;
    union pwmc_ioctl_param *param = (union pwmc_ioctl_param *)args;

    do {
        if (!param) {
            break;
        }
        if (!pdesc->ops.update_duty_raw) {
            break;
        }
        raw = (uint32_t)(pdesc->raw_max * param->duty.duty);
        pdesc->ops.update_duty_raw(param->duty.channel, raw);
        err = DRV_ERR_OK;
    } while (0);

    return err;
}

static int32_t _ioctl_get_duty_raw(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    union pwmc_ioctl_param *param = (union pwmc_ioctl_param *)args;

    do {
        if (!param) {
            break;
        }
        if (!pdesc->ops.get_duty_raw) {
            break;
        }
        param->duty_raw.raw = pdesc->ops.get_duty_raw(param->duty_raw.channel);
        err = DRV_ERR_OK;
    } while (0);

    return err;
}

static int32_t _ioctl_set_duty_raw(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    union pwmc_ioctl_param *param = (union pwmc_ioctl_param *)args;

    do {
        if (!param) {
            break;
        }
        if (!pdesc->ops.update_duty_raw) {
            break;
        }
        pdesc->ops.update_duty_raw(param->duty_raw.channel, param->duty_raw.raw);
        err = DRV_ERR_OK;
    } while (0);

    return err;
}

static int32_t _ioctl_enable_channel(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    uint32_t *channel = (uint32_t *)args;

    do {
        if (!channel) {
            break;
        }
        if (!pdesc->ops.channel_ctrl) {
            break;
        }
        if (!pdesc->ops.channel_ctrl(*channel, true)) {
            err = DRV_ERR_ERROR;
            break;
        }
        err = DRV_ERR_OK;
    } while (0);

    return err;
}

static int32_t _ioctl_disable_channel(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    uint32_t *channel = (uint32_t *)args;

    do {
        if (!channel) {
            break;
        }
        if (!pdesc->ops.channel_ctrl) {
            break;
        }
        if (!pdesc->ops.channel_ctrl(*channel, false)) {
            err = DRV_ERR_ERROR;
            break;
        }
        err = DRV_ERR_OK;
    } while (0);

    return err;
}

static int32_t _ioctl_set_irq(pwmc_describe_t *pdesc, void *args)
{
    pwmc_irq_handler_fn irq = (pwmc_irq_handler_fn)args;

    pdesc->ops.irq_handler = irq;

    return DRV_ERR_OK;
}

static int32_t _ioctl_get_duty_raw_max(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    uint32_t *raw = (uint32_t *)args;

    do {
        if (!raw) {
            break;
        }
        *raw = pdesc->raw_max;
        err = DRV_ERR_OK;
    } while (0);

    return err;
}
/*---------- end of file ----------*/
