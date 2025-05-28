/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : pwmc.c
 * @Author       : lxf
 * @Date         : 2024-12-05 14:37:46
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2025-03-25 09:20:49
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
static int32_t _ioctl_get_duty(pwmc_describe_t *pdesc, void *args);
static int32_t _ioctl_set_duty(pwmc_describe_t *pdesc, void *args);
static int32_t _ioctl_get_duty_raw(pwmc_describe_t *pdesc, void *args);
static int32_t _ioctl_set_duty_raw(pwmc_describe_t *pdesc, void *args);
static int32_t _ioctl_set_irq(pwmc_describe_t *pdesc, void *args);
static int32_t _ioctl_set_freq_duty(pwmc_describe_t *pdesc, void *args);
/*---------- variable ----------*/
DRIVER_DEFINED(pwmc, pwmc_open, pwmc_close, NULL, NULL, pwmc_ioctl, pwmc_irq_handler);

static struct protocol_callback ioctl_cbs[] = {
    { IOCTL_PWMC_ENABLE, _ioctl_enable },
    { IOCTL_PWMC_DISABLE, _ioctl_disable },
    { IOCTL_PWMC_GET_FREQ, _ioctl_get_freq },
    { IOCTL_PWMC_SET_FREQ, _ioctl_set_freq },
    { IOCTL_PWMC_GET_DUTY, _ioctl_get_duty },
    { IOCTL_PWMC_SET_DUTY, _ioctl_set_duty },
    { IOCTL_PWMC_GET_DUTY_RAW, _ioctl_get_duty_raw },
    { IOCTL_PWMC_SET_DUTY_RAW, _ioctl_set_duty_raw },
    { IOCTL_PWMC_SET_IRQ_HANDLER, _ioctl_set_irq },
    { IOCTL_PWMC_SET_FREQ_DUTY, _ioctl_set_freq_duty }
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
        if (!pdesc->ops.init) {
            err = DRV_ERR_POINT_NONE;
            break;
        }
        if (!pdesc->ops.update_frequence_arr) {
            err = DRV_ERR_POINT_NONE;
            break;
        }
        if (pdesc->frequence < 100) {
            pdesc->priv.prescaler = 10000 - 1;
            pdesc->priv.arr = (pdesc->clock / 10000) / pdesc->frequence - 1;
        } else if (pdesc->frequence < 1100) {
            pdesc->priv.prescaler = 1;
            pdesc->priv.arr = (pdesc->clock / 100) / pdesc->frequence - 1;
        } else {
            pdesc->priv.prescaler = 0;
            pdesc->priv.arr = pdesc->clock / pdesc->frequence - 1;
        }
        if (!pdesc->ops.init()) {
            err = DRV_ERR_ERROR;
            break;
        }
        struct pwmc_ioctl_param param;
        param.freq = pdesc->frequence;
        param.duty = pdesc->duty;
        err = _ioctl_set_freq_duty(pdesc, &param);
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

static int32_t _ioctl_enable(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_ERROR;

    if (pdesc->ops.enable) {
        if (pdesc->ops.enable(true)) {
            pdesc->is_enable = true;
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
            pdesc->is_enable = false;
            err = DRV_ERR_OK;
        }
    }

    return err;
}

static int32_t _ioctl_get_freq(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    struct pwmc_ioctl_param *param = (struct pwmc_ioctl_param *)args;
    do {
        if (!param) {
            break;
        }
        param->freq = pdesc->frequence;
        err = DRV_ERR_EOK;
    } while (0);

    return err;
}

static int32_t _ioctl_set_freq(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    struct pwmc_ioctl_param *param = (struct pwmc_ioctl_param *)args;
    uint32_t crr;
    do {
        if (!param) {
            break;
        }
        if (!pdesc->ops.update_frequence_arr) {
            err = DRV_ERR_POINT_NONE;
        }
        if (param->freq < 100) {
            pdesc->priv.prescaler = 10000 - 1;
            pdesc->priv.arr = (pdesc->clock / 10000) / param->freq - 1;
        } else if (param->freq < 3000) {
            pdesc->priv.prescaler = 100 - 1;
            pdesc->priv.arr = (pdesc->clock / 100) / param->freq - 1;
        } else {
            pdesc->priv.prescaler = 0;
            pdesc->priv.arr = pdesc->clock / param->freq - 1;
        }
        err = pdesc->ops.update_frequence_arr(pdesc->priv.prescaler, pdesc->priv.arr);
        if (err != DRV_ERR_EOK) {
            break;
        }
        crr = (uint32_t)(pdesc->priv.arr * pdesc->duty);
        err = pdesc->ops.update_duty_crr(crr);
        if (err != DRV_ERR_EOK) {
            break;
        }
        crr = (uint32_t)(pdesc->priv.arr * pdesc->duty);
        err = pdesc->ops.update_duty_crr(crr);
        pdesc->priv.crr = crr;
        pdesc->frequence = param->freq;
    } while (0);

    return err;
}

static int32_t _ioctl_get_duty(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    uint32_t crr = 0;
    struct pwmc_ioctl_param *param = (struct pwmc_ioctl_param *)args;

    do {
        if (!param) {
            break;
        }
        if (!pdesc->ops.get_duty_crr) {
            break;
        }
        crr = pdesc->ops.get_duty_crr();
        param->duty = (float)crr / (float)pdesc->priv.arr;
        err = DRV_ERR_OK;
    } while (0);

    return err;
}

static int32_t _ioctl_set_duty(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    uint32_t crr = 0;
    struct pwmc_ioctl_param *param = (struct pwmc_ioctl_param *)args;

    do {
        if (!param) {
            break;
        }
        if (!pdesc->ops.update_duty_crr) {
            break;
        }
        crr = (uint32_t)(pdesc->priv.arr * param->duty);
        err = pdesc->ops.update_duty_crr(crr);
        if (err != DRV_ERR_EOK) {
            break;
        }
        pdesc->priv.crr = crr;
        pdesc->duty = param->duty;
    } while (0);

    return err;
}

static int32_t _ioctl_get_duty_raw(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    struct pwmc_ioctl_param *param = (struct pwmc_ioctl_param *)args;

    do {
        if (!param) {
            break;
        }
        if (!pdesc->ops.get_duty_crr) {
            break;
        }
        param->crr = pdesc->ops.get_duty_crr();
        err = DRV_ERR_OK;
    } while (0);

    return err;
}

static int32_t _ioctl_set_duty_raw(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    struct pwmc_ioctl_param *param = (struct pwmc_ioctl_param *)args;

    do {
        if (!param) {
            break;
        }
        if (!pdesc->ops.update_duty_crr) {
            break;
        }
        pdesc->ops.update_duty_crr(param->crr);
        pdesc->duty = (float)param->crr / (float)pdesc->priv.arr;
        pdesc->priv.crr = param->crr;
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
static int32_t _ioctl_set_freq_duty(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    struct pwmc_ioctl_param *param = (struct pwmc_ioctl_param *)args;
    do {
        if (!param) {
            break;
        }
        if (!pdesc->ops.update_frequence_arr) {
            err = DRV_ERR_POINT_NONE;
        }
        if (!pdesc->ops.update_duty_crr) {
            err = DRV_ERR_POINT_NONE;
            break;
        }
        pdesc->duty = param->duty;
        if (param->duty == 0 || param->duty == 1) {
            pdesc->priv.crr = (uint32_t)(pdesc->priv.arr * param->duty);
            pdesc->ops.update_duty_crr(pdesc->priv.crr);
            pdesc->frequence = param->freq;
            err = DRV_ERR_EOK;
            break;
        }
        // this is depend on timer clock.
        if (param->freq < 600) {
            pdesc->priv.prescaler = 10000 - 1;
            pdesc->priv.arr = (pdesc->clock / 10000) / param->freq - 1;
        } else if (param->freq < 1100) {
            pdesc->priv.prescaler = 2 - 1;
            pdesc->priv.arr = (pdesc->clock / 2) / param->freq - 1;
        } else {
            pdesc->priv.prescaler = 0;
            pdesc->priv.arr = pdesc->clock / param->freq - 1;
        }
        err = pdesc->ops.update_frequence_arr(pdesc->priv.prescaler, pdesc->priv.arr);
        if (err != DRV_ERR_EOK) {
            break;
        }
        pdesc->frequence = param->freq;
        pdesc->priv.crr = (uint32_t)(pdesc->priv.arr * param->duty);;
        err = pdesc->ops.update_duty_crr(pdesc->priv.crr);
        if (err != DRV_ERR_EOK) {
            break;
        }
    } while (0);

    return err;
}
/*---------- end of file ----------*/
