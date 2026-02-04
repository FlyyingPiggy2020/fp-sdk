/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : pwmc.c
 * @Author       : lxf
 * @Date         : 2024-12-05 14:37:46
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-09-22 19:55:07
 * @Brief        :
 * todo:以定时器为基本抽象，还是以单个通道为抽象？
 * 2025年3月17日 lxf 初版单通道独立频率的模型
 * 2025年9月22日 lxf 改为定时器+多通道的模型。
 */

/*---------- includes ----------*/
#include "pwmc.h"
#include "driver.h"
#include "drv_err.h"
/*---------- macro ----------*/

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t __update_prescaler_arr_by_freq(pwmc_describe_t *pdesc, uint32_t frequence);

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
static int32_t _ioctl_set_freq_duty(pwmc_describe_t *pdesc, void *args);
/*---------- variable ----------*/
DRIVER_DEFINED(pwmc, pwmc_open, pwmc_close, NULL, NULL, pwmc_ioctl, pwmc_irq_handler);

static struct protocol_callback ioctl_cbs[] = { { IOCTL_PWMC_ENABLE, _ioctl_enable },
                                                { IOCTL_PWMC_DISABLE, _ioctl_disable },
                                                { IOCTL_PWMC_GET_FREQ, _ioctl_get_freq },
                                                { IOCTL_PWMC_SET_FREQ, _ioctl_set_freq },
                                                { IOCTL_PWMC_GET_DUTY, _ioctl_get_duty },
                                                { IOCTL_PWMC_SET_DUTY, _ioctl_set_duty },
                                                { IOCTL_PWMC_GET_DUTY_RAW, _ioctl_get_duty_raw },
                                                { IOCTL_PWMC_SET_DUTY_RAW, _ioctl_set_duty_raw },
                                                { IOCTL_PWMC_SET_FREQ_DUTY, _ioctl_set_freq_duty } };
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
        if (!pdesc->ops.update_precaler_arr) {
            err = DRV_ERR_POINT_NONE;
            break;
        }
        __update_prescaler_arr_by_freq(pdesc, pdesc->frequence);

        if (!pdesc->ops.init()) {
            err = DRV_ERR_ERROR;
            break;
        }

        union pwmc_ioctl_param param;
        param.set.freq = pdesc->frequence;

        for (uint8_t i = 0; i < IOCTL_CONFIG_PWMC_CHANNEL; i++) {
            if (!pdesc->priv.channel[i].used) {
                continue;
            }
            param.set.channel = i;
            param.set.duty = pdesc->priv.channel[i].duty;
            _ioctl_set_freq_duty(pdesc, &param);
        }
        err = DRV_ERR_EOK;
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
        cb = (int32_t (*)(pwmc_describe_t *, void *))protocol_callback_find(cmd, ioctl_cbs, ARRAY_SIZE(ioctl_cbs));
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

static int32_t __update_prescaler_arr_by_freq(pwmc_describe_t *pdesc, uint32_t frequence)
{
    // this is depend on timer clock.
    if (frequence < 600) {
        pdesc->priv.prescaler = 10000 - 1;
        pdesc->priv.arr = (pdesc->clock / 10000) / frequence - 1;
    } else if (frequence < 1100) {
        pdesc->priv.prescaler = 2 - 1;
        pdesc->priv.arr = (pdesc->clock / 2) / frequence - 1;
    } else {
        pdesc->priv.prescaler = 0;
        pdesc->priv.arr = pdesc->clock / frequence - 1;
    }

    if (pdesc->ops.update_precaler_arr) {
        pdesc->ops.update_precaler_arr(pdesc->priv.prescaler, pdesc->priv.arr);
    }

    pdesc->frequence = frequence;
    return DRV_ERR_EOK;
}

static int32_t __update_crr_by_duty(pwmc_describe_t *pdesc)
{
    int32_t crr;
    for (uint8_t i = 0; i < IOCTL_CONFIG_PWMC_CHANNEL; i++) {
        if (pdesc->priv.channel[i].used == false) {
            continue;
        }
        crr = (uint32_t)(pdesc->priv.arr * pdesc->priv.channel[i].duty);
        pdesc->ops.update_crr(i, crr);
        pdesc->priv.channel[i].crr = crr;
    }
    return DRV_ERR_EOK;
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
    union pwmc_ioctl_param *param = (union pwmc_ioctl_param *)args;
    do {
        if (!param) {
            break;
        }
        param->get.freq = pdesc->frequence;
        err = DRV_ERR_EOK;
    } while (0);

    return err;
}

static int32_t _ioctl_set_freq(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    union pwmc_ioctl_param *param = (union pwmc_ioctl_param *)args;
    uint32_t crr;
    do {
        if (!param) {
            break;
        }

        if (!pdesc->ops.update_precaler_arr) {
            err = DRV_ERR_POINT_NONE;
        }

        if (param->set.channel > IOCTL_CONFIG_PWMC_CHANNEL) {
            break;
        }

        if (pdesc->priv.channel[param->set.channel].used == false) {
            break;
        }
        __update_prescaler_arr_by_freq(pdesc, param->set.freq);
        __update_crr_by_duty(pdesc);
        err = DRV_ERR_OK;
    } while (0);

    return err;
}

static int32_t _ioctl_get_duty(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    uint32_t crr = 0;
    union pwmc_ioctl_param *param = (union pwmc_ioctl_param *)args;

    do {
        if (!param) {
            break;
        }
        if (param->get.channel > IOCTL_CONFIG_PWMC_CHANNEL) {
            break;
        }
        if (pdesc->priv.channel[param->get.channel].used == false) {
            break;
        }
        param->get.duty = pdesc->priv.channel[param->get.channel].duty;
        err = DRV_ERR_OK;
    } while (0);

    return err;
}

static int32_t _ioctl_set_duty(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    uint32_t crr = 0;
    union pwmc_ioctl_param *param = (union pwmc_ioctl_param *)args;

    do {
        if (!param) {
            break;
        }
        if (param->set.channel > IOCTL_CONFIG_PWMC_CHANNEL) {
            break;
        }
        if (pdesc->priv.channel[param->set.channel].used == false) {
            break;
        }
        if (param->set.duty > 1 || param->set.duty < 0) {
            break;
        }
        crr = (uint32_t)(pdesc->priv.arr * param->set.duty);
        pdesc->ops.update_crr(param->set.channel, crr);
        pdesc->priv.channel[param->set.channel].crr = crr;
        pdesc->priv.channel[param->set.channel].duty = param->set.duty;
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

        if (param->get.channel > IOCTL_CONFIG_PWMC_CHANNEL) {
            break;
        }

        if (pdesc->priv.channel[param->get.channel].used == false) {
            break;
        }

        param->get.crr = pdesc->priv.channel[param->get.channel].crr;
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
        if (param->set.channel > IOCTL_CONFIG_PWMC_CHANNEL) {
            break;
        }
        if (pdesc->priv.channel[param->set.channel].used == false) {
            break;
        }
        if (!pdesc->ops.update_crr) {
            break;
        }
        pdesc->ops.update_crr(param->set.channel, param->set.crr);
        pdesc->priv.channel[param->set.channel].duty = (float)param->set.crr / (float)pdesc->priv.arr;
        pdesc->priv.channel[param->set.channel].crr = param->set.crr;
        err = DRV_ERR_OK;
    } while (0);

    return err;
}

static int32_t _ioctl_set_freq_duty(pwmc_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    int32_t crr;
    union pwmc_ioctl_param *param = (union pwmc_ioctl_param *)args;
    do {
        if (!param) {
            break;
        }
        if (!pdesc->ops.update_precaler_arr) {
            err = DRV_ERR_POINT_NONE;
        }
        if (!pdesc->ops.update_crr) {
            err = DRV_ERR_POINT_NONE;
            break;
        }
        if (param->set.channel > IOCTL_CONFIG_PWMC_CHANNEL) {
            break;
        }
        if (pdesc->priv.channel[param->set.channel].used == false) {
            break;
        }
        if (param->set.duty > 1 || param->set.duty < 0) {
            break;
        }
        pdesc->priv.channel[param->set.channel].duty = param->set.duty;
        __update_prescaler_arr_by_freq(pdesc, param->set.freq);
        __update_crr_by_duty(pdesc);
        err = DRV_ERR_OK;
    } while (0);

    return err;
}
/*---------- end of file ----------*/
