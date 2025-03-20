/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : lightc_map.c
 * @Author       : lxf
 * @Date         : 2025-03-18 09:12:48
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2025-03-19 08:57:13
 * @Brief        :
 */

#include "lightc_map.h"
#include "driver.h"
#include "drv_err.h"
#include "math.h"
/*---------- includes ----------*/
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t _light_open(driver_t **pdrv);
static void _light_close(driver_t **pdrv);
int32_t _light_ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static int32_t _light_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length);

static void _get_brightness_total_from_brightness(lightc_map_describe_t *pdesc);
static void _get_pwm_duty_frequence(lightc_map_describe_t *pdesc);
/*---------- variable ----------*/
DRIVER_DEFINED(lightc_map, _light_open, _light_close, NULL, NULL, _light_ioctl, _light_irq_handler);

static struct protocol_callback ioctl_cbs[] = {
    { IOCTL_LIGHTC_SET_CURRENT_MODE, NULL },
    //    { IOCTL_LIGHTC_CMD_OFF, __light_cmd_off },
    //    { IOCTL_LIGHTC_CMD_ON, __light_cmd_on },
    //    { IOCTL_LIGHTC_SET_BRIGHTNESS_BY_TIME, __light_set_brightness_by_time },
};
/*---------- function ----------*/
static int32_t _light_open(driver_t **pdrv)
{
    lightc_map_describe_t *pdesc = NULL;
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
        // init priv param
        if (pdesc->param.dimming_end_point == pdesc->param.dimming_start_point) {
            err = DRV_ERR_WRONG_ARGS;
            break;
        }
        if (pdesc->param.start_delay) {
            pdesc->priv.brightness_step_1_percent_inc = 1 / (float)(pdesc->param.start_delay * 10);
        } else {
            pdesc->priv.brightness_step_1_percent_inc = 1;
        }
        if (pdesc->param.start_delay) {
            pdesc->priv.brightness_step_1_percent_dec = 1 / (float)(pdesc->param.stop_delay * 10);
        } else {
            pdesc->priv.brightness_step_1_percent_dec = 1;
        }
        pdesc->priv.brightness_step_1_to_100_inc = (float)(99) / pdesc->param.fade_in_time / 100;
        pdesc->priv.brightness_step_1_to_100_dec = (float)(99) / (pdesc->param.fade_out_time) / 100;
    } while (0);

    return err;
}

static void _light_close(driver_t **pdrv)
{
    lightc_map_describe_t *pdesc = NULL;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    if (pdesc && pdesc->ops.deinit) {
        pdesc->ops.deinit();
    }
}

static int32_t _light_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    lightc_map_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_WRONG_ARGS;
    int32_t (*cb)(lightc_map_describe_t *, void *) = NULL;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if (!pdesc) {
            break;
        }
        cb = (int32_t(*)(lightc_map_describe_t *, void *))protocol_callback_find(cmd, ioctl_cbs, ARRAY_SIZE(ioctl_cbs));
        if (!cb) {
            break;
        }
        err = cb(pdesc, args);
    } while (0);
    return err;
}

static int32_t _light_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length)
{
    lightc_map_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_WRONG_ARGS;
    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    float step = 0;
    if (pdesc) {
        do {
            if (pdesc->priv.brightness_position > pdesc->brightness) { // brightness inc
                if (pdesc->brightness < 1) {                           // start_delay
                    pdesc->brightness += pdesc->priv.brightness_step_1_percent_inc;
                } else if (pdesc->brightness < 100) { // fade_in_time
                    pdesc->priv.time_count++;
                    pdesc->brightness += pdesc->priv.brightness_step_1_to_100_dec;
                }
                if (pdesc->brightness > pdesc->priv.brightness_position) {
                    pdesc->brightness = pdesc->priv.brightness_position;
                }
            } else if (pdesc->priv.brightness_position < pdesc->brightness) { // brightness dec
                if (pdesc->brightness > 1) {                                  // fade_out_time
                    pdesc->brightness -= pdesc->priv.brightness_step_1_to_100_dec;
                } else { // stop_delay
                    pdesc->brightness -= pdesc->priv.brightness_step_1_percent_dec;
                }
                if (pdesc->brightness < pdesc->priv.brightness_position) {
                    pdesc->brightness = pdesc->priv.brightness_position;
                }
            }
            _get_brightness_total_from_brightness(pdesc);
        } while (0);
        do {
            _get_pwm_duty_frequence(pdesc);
            if (pdesc->ops.update_brightness) {
                pdesc->ops.update_brightness(pdesc->priv.frequence, pdesc->priv.duty);
            }
        } while (0);
    }
    return err;
}

/**
 * @brief make sure stop point is bigger than start point.
 * if brightness is large than 1%, brightness actual = start + (end - start) * (brightness - 1) / 99;
 * if brightness is small than 1%, brightness actual = zero + (start - zero) * brightness;
 * @param {lightc_map_describe_t} *pdesc
 * @return {*}
 */
static void _get_brightness_total_from_brightness(lightc_map_describe_t *pdesc)
{
    ASSERT(pdesc);
    float start_point, zero_point, stop_point;
    if (pdesc->param.dimming_start_point == 0) {
        start_point = (float)(pdesc->param.dimming_end_point - pdesc->param.dimming_start_point) / 100;
        stop_point = pdesc->param.dimming_end_point;
        zero_point = 0;
    } else {
        start_point = pdesc->param.dimming_start_point;
        stop_point = pdesc->param.dimming_end_point;
        zero_point = (float)(pdesc->param.dimming_start_point * pdesc->param.charge_duty) / 100;
    }

    if (pdesc->brightness < 1) {
        pdesc->priv.brightness_actual = zero_point + (start_point - zero_point) * pdesc->brightness;
    } else {
        if (stop_point > zero_point) {
            pdesc->priv.brightness_actual = start_point + ((stop_point - start_point) * (pdesc->brightness - 1) / 99);
        }
    }
}
/**
 * @brief linear interpolation
 * @param {lightc_map_describe_t} *pdesc
 * @param {uint32_t} *ret_frequence
 * @param {float} *ret_duty
 * @return {*}
 */
static void _get_pwm_duty_frequence(lightc_map_describe_t *pdesc)
{
    int table_size = pdesc->map->node_size;
    brightness_map_t *map = pdesc->map;
    double x1, y1, z1, x2, y2, z2;
    double brightness_actual = pdesc->priv.brightness_actual;
    for (int i = 0; i < table_size - 1; i++) {
        if (brightness_actual >= map->node[i].brightness && brightness_actual <= map->node[i + 1].brightness) {
            x1 = map->node[i].brightness;
            y1 = map->node[i].duty;
            z1 = map->node[i].frequence;
            x2 = map->node[i + 1].brightness;
            y2 = map->node[i + 1].duty;
            z2 = map->node[i + 1].frequence;
            pdesc->priv.duty = y1 + ((y2 - y1) * (brightness_actual - x1)) / (x2 - x1);
            pdesc->priv.frequence = z1 + ((z2 - z1) * (brightness_actual - x1)) / (x2 - x1);
        }
    }
}
/*---------- end of file ----------*/
