/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : led.c
 * @Author       : lxf
 * @Date         : 2025-05-20 10:49:54
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-05-20 11:57:44
 * @Brief        : 
 */


/*---------- includes ----------*/

#include "led.h"
#include "driver.h"
#include "drv_err.h"
#include "heap.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t _ledf_open(driver_t **pdrv);
static void _ledf_close(driver_t **pdrv);
int32_t _ledf_ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static int32_t _ledf_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length);

/* api */
static int32_t _led_start_flash(ledf_describe_t *pdesc, void *args);
static int32_t _led_stop_flash(ledf_describe_t *pdesc, void *args);

/* priv */
static int32_t __led_stop_flash(ledf_describe_t * pdesc, uint8_t id, led_flash_layer_name_t layer);
/*---------- variable ----------*/
DRIVER_DEFINED(ledf, _ledf_open, _ledf_close, NULL, NULL, _ledf_ioctl, _ledf_irq_handler);

static struct protocol_callback ioctl_cbs[] = {
    {IOCTL_LEDF_START_FLASH, _led_start_flash},
    {IOCTL_LEDF_STOP_FLASH, _led_stop_flash},
};
/*---------- function ----------*/

static int32_t _ledf_open(driver_t **pdrv)
{
    ledf_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_WRONG_ARGS;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;

    do {
        if (!pdesc) {
            break;
        }

        if (pdesc->ops.init) {
            pdesc->ops.init();
        }

        if (pdesc->led_num) {
            pdesc->priv.led_flash_list = malloc(sizeof(led_flash_logic_t) * pdesc->led_num);
            if (pdesc->priv.led_flash_list == NULL) {
                err = DRV_ERR_NO_MEMORY;
                break;
            }
        }

        memset(pdesc->priv.led_flash_list, 0, sizeof(sizeof(led_flash_logic_t) * pdesc->led_num));

        err = DRV_ERR_EOK;
    } while (0);
    return err;
}
static void _ledf_close(driver_t **pdrv)
{
    ledf_describe_t *pdesc = NULL;
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;

    if (pdesc && pdesc->ops.deinit) {
        pdesc->ops.deinit();
    }
}
int32_t _ledf_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    ledf_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_WRONG_ARGS;
    int32_t (*cb)(ledf_describe_t *, void *) = NULL;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if (!pdesc) {
            break;
        }
        cb = (int32_t(*)(ledf_describe_t *, void *))protocol_callback_find(cmd, ioctl_cbs, ARRAY_SIZE(ioctl_cbs));
        if (!cb) {
            break;
        }
        err = cb(pdesc, args);
    } while (0);
    return err;
}

static int32_t _ledf_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length)
{
    ledf_describe_t *pdesc = NULL;
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    int32_t err = DRV_ERR_WRONG_ARGS;
    if (!pdesc) {
        return DRV_ERR_POINT_NONE;
    }

    if (!pdesc->priv.led_flash_list) {
        return DRV_ERR_POINT_NONE;
    }

    led_flash_logic_t *led_flash_list = pdesc->priv.led_flash_list;

    for (uint8_t i = 0; i < pdesc->led_num; i++) {
        if (led_flash_list[i].layer[LED_FLASH_HIGH_LAYER].dealy > 0) {
            led_flash_list[i].layer[LED_FLASH_HIGH_LAYER].dealy--;
            if (led_flash_list[i].layer[LED_FLASH_HIGH_LAYER].dealy == 0) { /* 每delay毫秒执行一次 */
                pdesc->ops.led_toggle(i);
                if (led_flash_list[i].layer[LED_FLASH_HIGH_LAYER].cnt != LED_FLASH_FOREVER) {
                    led_flash_list[i].layer[LED_FLASH_HIGH_LAYER].cnt--;
                }
                if (led_flash_list[i].layer[LED_FLASH_HIGH_LAYER].cnt != 0) {
                    led_flash_list[i].layer[LED_FLASH_HIGH_LAYER].dealy =
                        led_flash_list[i].layer[LED_FLASH_HIGH_LAYER].delay_reload;
                }
                else {
                    /* clear led list */
                    __led_stop_flash(pdesc, i, LED_FLASH_HIGH_LAYER);
                }
            }
        }
        else if (led_flash_list[i].layer[LED_FLASH_LOW_LAYER].dealy > 0) {
            // 低优先级的layer里面支持让某个灯一直亮
            if (led_flash_list[i].layer[LED_FLASH_LOW_LAYER].dealy != LED_FLASH_DELAY_FOREVER) {
                led_flash_list[i].layer[LED_FLASH_LOW_LAYER].dealy--;
            }
            else {
                pdesc->ops.led_on(i);
            }
            if (led_flash_list[i].layer[LED_FLASH_LOW_LAYER].dealy == 0) { /* 每delay毫秒执行一次 */
                pdesc->ops.led_toggle(i);
                if (led_flash_list[i].layer[LED_FLASH_LOW_LAYER].cnt != LED_FLASH_FOREVER) {
                    led_flash_list[i].layer[LED_FLASH_LOW_LAYER].cnt--;
                }
                if (led_flash_list[i].layer[LED_FLASH_LOW_LAYER].cnt != 0) {
                    led_flash_list[i].layer[LED_FLASH_LOW_LAYER].dealy =
                        led_flash_list[i].layer[LED_FLASH_LOW_LAYER].delay_reload;
                }
                else {
                    /* clear led list */
                    __led_stop_flash(pdesc, i, LED_FLASH_LOW_LAYER);
                }
            }
        }
    }
    return err;
}

/**
 * @brief led闪烁;如果之前的闪烁还未结束就重复调用，则新的闪烁配置会覆盖掉之前的闪烁配置。
 * @param {uint8_t} id led名称
 * @param {uint8_t} cnt 闪烁次数（1亮1灭为1次闪烁;0XFFFF为永远闪烁;小于[FFF0]
 * @param {uint16_t} delay（闪烁的延时);0XFFFF为常亮。
 * @param {led_flash_layer_name_t} layer 控制逻辑优先级
 * @return {*}
 */
int32_t __led_start_flash(ledf_describe_t * pdesc, uint8_t id, uint16_t cnt, uint16_t delay, led_flash_layer_name_t layer)
{
    if (!pdesc->priv.led_flash_list) {
        return DRV_ERR_POINT_NONE;
    }

    led_flash_logic_t *led_flash_list = pdesc->priv.led_flash_list;

    if (cnt == LED_FLASH_FOREVER) {
        led_flash_list[id].layer[layer].cnt = LED_FLASH_FOREVER;
        led_flash_list[id].layer[layer].cnt_reload = LED_FLASH_FOREVER;
    }
    else if (cnt < (UINT16_MAX / 2) - 1) {
        led_flash_list[id].layer[layer].cnt = cnt * 2;
        led_flash_list[id].layer[layer].cnt_reload = cnt * 2;
    }
    led_flash_list[id].layer[layer].dealy = 1;
    led_flash_list[id].layer[layer].delay_reload = delay;
    return DRV_ERR_EOK;
}

/**
 * @brief led停止闪烁
 * @param {uint8_t} id : LED序号
 * @param {led_flash_layer_name_t} layer : 图层
 * @return {*}
 */
static int32_t __led_stop_flash(ledf_describe_t * pdesc, uint8_t id, led_flash_layer_name_t layer)
{
    if (!pdesc->priv.led_flash_list) {
        return DRV_ERR_POINT_NONE;
    }
    if (!pdesc->ops.led_off) {
        return DRV_ERR_POINT_NONE;
    }

    led_flash_logic_t *led_flash_list = pdesc->priv.led_flash_list;
    pdesc->ops.led_off(id);
    led_flash_list[id].layer[layer].dealy = 0;
    led_flash_list[id].layer[layer].delay_reload = 0;
    led_flash_list[id].layer[layer].cnt = 0;
    led_flash_list[id].layer[layer].cnt_reload = 0;
    return DRV_ERR_EOK;
}

static int32_t _led_start_flash(ledf_describe_t *pdesc, void *args)
{
    if (!pdesc || !args) {
        return DRV_ERR_POINT_NONE;
    }
    ledf_api_param_t *param = args;
    return __led_start_flash(pdesc, param->start.id, param->start.cnt, param->start.delay, param->start.layer);
}

static int32_t _led_stop_flash(ledf_describe_t *pdesc, void *args)
{
    if (!pdesc || !args) {
        return DRV_ERR_POINT_NONE;
    }
    ledf_api_param_t *param = args;
    
    return __led_stop_flash(pdesc, param->stop.id, param->stop.layer);
}
/*---------- end of file ----------*/


