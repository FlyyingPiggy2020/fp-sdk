/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : lightc_map.c
 * @Author       : lxf
 * @Date         : 2025-03-18 09:12:48
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-05-27 15:28:04
 * @Brief        :
 */

#include "lightc_map.h"
#include "driver.h"
#include "drv_err.h"
#include "math.h"
/*---------- includes ----------*/
/*---------- macro ----------*/
#define LIGHT_MAP_FLOAT_POINT_TYPE float // float or double
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t _light_open(driver_t **pdrv);
static void _light_close(driver_t **pdrv);
static int32_t _light_ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static int32_t _light_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length);

static void _get_brightness_actual_from_brightness(lightc_map_describe_t *pdesc);
static void _get_pwm_duty_frequence(lightc_map_describe_t *pdesc);
static void _get_color_pwm(lightc_map_describe_t *pdesc);

/* lightc map control api */
static int32_t __light_cmd_off(lightc_map_describe_t *pdesc, void *args);
static int32_t __light_cmd_on(lightc_map_describe_t *pdesc, void *args);
static int32_t __light_set_brightness(lightc_map_describe_t *pdesc, void *args);
static int32_t __light_step_brightness_inc(lightc_map_describe_t *pdesc, void *args);
static int32_t __light_step_brightness_dec(lightc_map_describe_t *pdesc, void *args);
static int32_t __light_continue_brightness_inc(lightc_map_describe_t *pdesc, void *args);
static int32_t __light_continue_brightness_dec(lightc_map_describe_t *pdesc, void *args);
static int32_t __light_adjustment_finish(lightc_map_describe_t *pdesc, void *args);
static int32_t __loop_adjustment_start(lightc_map_describe_t *pdesc, void *args);
static int32_t __loop_adjustment_stop(lightc_map_describe_t *pdesc, void *args);
static int32_t __light_reverse(lightc_map_describe_t *pdesc, void *args);
static int32_t __light_set_brightness_by_time(lightc_map_describe_t *pdesc, void *args);
static int32_t __light_reverse_ext(lightc_map_describe_t *pdesc, void *args);
static int32_t __light_reverse_brightness(lightc_map_describe_t *pdesc, void *args);
static int32_t __light_start(lightc_map_describe_t *pdesc, void *args);
static int32_t __light_param_write(lightc_map_describe_t *pdesc, void *args);
static int32_t __light_adjustment_start_by_time(lightc_map_describe_t *pdesc, void *args);
static int32_t __light_get_brightness(lightc_map_describe_t *pdesc, void *args);
static int32_t __light_set_virtual_brightness(lightc_map_describe_t *pdesc, void *args);
static int32_t __light_set_color(lightc_map_describe_t *pdesc, void *args);

/*---------- variable ----------*/
DRIVER_DEFINED(lightc_map, _light_open, _light_close, NULL, NULL, _light_ioctl, _light_irq_handler);

static struct protocol_callback ioctl_cbs[] = {
    { IOCTL_LIGHTC_SET_CURRENT_MODE, NULL },
    { IOCTL_LIGHTC_CMD_OFF, __light_cmd_off },
    { IOCTL_LIGHTC_CMD_ON, __light_cmd_on },
    { IOCTL_LIGHTC_SET_BRIGHTNESS, __light_set_brightness },
    { IOCTL_LIGHTC_STEP_BRIGHTNESS_INC, __light_step_brightness_inc },
    { IOCTL_LIGHTC_STEP_BRIGHTNESS_DEC, __light_step_brightness_dec },
    { IOCTL_LIGHTC_CONTINUE_BRIGHTNESS_INC, __light_continue_brightness_inc },
    { IOCTL_LIGHTC_CONTINUE_BRIGHTNESS_DEC, __light_continue_brightness_dec },
    { IOCTL_LIGHTC_LIGHT_ADJUSTMENT_FINISH, __light_adjustment_finish },
    { IOCTL_LIGHTC_LOOP_LIGHT_ADJ_START, __loop_adjustment_start },
    { IOCTL_LIGHTC_LOOP_LIGHT_ADJ_STOP, __loop_adjustment_stop },
    { IOCTL_LIGHTC_REVERSE, __light_reverse },
    { IOCTL_LIGHTC_SET_BRIGHTNESS_BY_TIME, __light_set_brightness_by_time },
    //    { IOCTL_LIGHTC_REVERSE_EXT, __light_reverse_ext },
    //    { IOCTL_LIGHTC_REVERSE_BRIGHTNESS, __light_reverse_brightness },
    { IOCTL_LIGHTC_START, __light_start },
    { IOCTL_LIGHTC_PARAM_WRITE, __light_param_write },
    { IOCTL_LIGHTC_LOOP_LIGHT_ADJ_START_BY_TIME, __light_adjustment_start_by_time },
    { IOCTL_LIGHTC_GET_BRIGHTNESS, __light_get_brightness },
    { IOCTL_LIGHTC_SET_VIRTUAL_BRIGHTNESS, __light_set_virtual_brightness },
    { IOCTL_LIGHTC_SET_COLOR, __light_set_color },
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
        if (pdesc->is_virtual) {
            break;
        }
        // default parameter
        pdesc->param.light_type = 0xff;
        pdesc->priv.remeber_brightness = 100;
        pdesc->param.dimming_start_point = 0;
        pdesc->param.dimming_end_point = 100;
        pdesc->param.cut_start_point = 0;
        pdesc->param.cut_end_pint = 0;
        pdesc->param.start_delay = 8;
        pdesc->param.stop_delay = 8;
        pdesc->param.charge_duty = 20;
        pdesc->param.fade_in_time = 8;
        pdesc->param.fade_out_time = 8;
        pdesc->param.start_state = 0;
        pdesc->priv.frequence = 1200;
        pdesc->color = 2700;
        pdesc->priv.color_postion = 2700;
        if (pdesc->ops.init) {
            if (!pdesc->ops.init()) {
                err = -1;
            }
        }
        // init priv param
        if (pdesc->param.dimming_end_point <= pdesc->param.dimming_start_point) {
            TRACE("(%s) dimming end point[%d] is bigger than dimming start point[%d].", pdrv[0]->drv_name, pdesc->param.dimming_end_point, pdesc->param.dimming_start_point);
            err = DRV_ERR_WRONG_ARGS;
            break;
        }
        if (pdesc->param.start_delay) {
            pdesc->priv.brightness_step_1_percent_inc = 1 * 10 / (LIGHT_MAP_FLOAT_POINT_TYPE)(pdesc->param.start_delay * pdesc->time_slice_frequence);
        } else {
            pdesc->priv.brightness_step_1_percent_inc = 1;
        }
        if (pdesc->param.start_delay) {
            pdesc->priv.brightness_step_1_percent_dec = 1 * 10 / (LIGHT_MAP_FLOAT_POINT_TYPE)(pdesc->param.stop_delay * pdesc->time_slice_frequence);
        } else {
            pdesc->priv.brightness_step_1_percent_dec = 1;
        }
        pdesc->priv.brightness_step_1_to_100_inc = 99 / (LIGHT_MAP_FLOAT_POINT_TYPE)(pdesc->time_slice_frequence * pdesc->param.fade_in_time);
        pdesc->priv.brightness_step_1_to_100_dec = 99 / (LIGHT_MAP_FLOAT_POINT_TYPE)(pdesc->time_slice_frequence * pdesc->param.fade_out_time);
        pdesc->priv.color_step_inc = (5600 - 2700) / (LIGHT_MAP_FLOAT_POINT_TYPE)(pdesc->time_slice_frequence * pdesc->param.fade_in_time);
        pdesc->priv.color_step_dec = (5600 - 2700) / (LIGHT_MAP_FLOAT_POINT_TYPE)(pdesc->time_slice_frequence * pdesc->param.fade_out_time);
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
    LIGHT_MAP_FLOAT_POINT_TYPE step = 0;
    LIGHT_MAP_FLOAT_POINT_TYPE color_step = 0;
    if (pdesc) {

        do {
            // 虚拟化情况下直接退出
            if (pdesc->is_virtual == true) {
                // 询问灯光模组相关信息
                break;
            }
            /* 1.update brightness control status */
            if (pdesc->priv.brightness_position > pdesc->brightness) {
                pdesc->priv.status = LIGHTC_MAP_STATUS_INC;
            } else if (pdesc->priv.brightness_position < pdesc->brightness) {
                pdesc->priv.status = LIGHTC_MAP_STATUS_DEC;
            } else if (pdesc->priv.status != LIGHTC_MAP_STATUS_STOP) {
                pdesc->priv.last_status = pdesc->priv.status;
                pdesc->priv.last_brightness_postion = pdesc->brightness;
                pdesc->priv.status = LIGHTC_MAP_STATUS_STOP;
                if (pdesc->priv.mode == LIGHTC_MAP_MODE_SET_BRIGHTNESS_BY_TIME) {
                    pdesc->priv.mode = LIGHTC_MAP_MODE_NORMAL;
                }
                if (pdesc->brightness >= 1) {
                    pdesc->priv.remeber_brightness = pdesc->brightness;
                    pdesc->status.is_off = false;
                } else if (pdesc->brightness == 0) {
                    pdesc->status.is_off = true;
                }
                if (pdesc->cb.lightc_stop_callback) {
                    pdesc->cb.lightc_stop_callback();
                }
            }
            // 2. color
            if (pdesc->priv.color_postion > pdesc->color) {
                pdesc->priv.color_status = LIGHTC_MAP_STATUS_INC;
            } else if (pdesc->priv.color_postion < pdesc->color) {
                pdesc->priv.color_status = LIGHTC_MAP_STATUS_DEC;
            } else if (pdesc->priv.color_status != LIGHTC_MAP_STATUS_STOP) {
                pdesc->priv.color_status = LIGHTC_MAP_STATUS_STOP;
            }
        } while (0);
        /* 2.calculate inc/dec step in differnet control mode*/
        do {
            if (pdesc->priv.mode == LIGHTC_MAP_MODE_NORMAL || pdesc->priv.mode == LIGHTC_MAP_MODE_LOOP) {
                if (pdesc->priv.status == LIGHTC_MAP_STATUS_INC) {
                    if (pdesc->brightness < 1) {
                        step = pdesc->priv.brightness_step_1_percent_inc;
                    } else if (pdesc->brightness < 100) { // fade_in_time
                        step = pdesc->priv.brightness_step_1_to_100_inc;
                    }
                } else if (pdesc->priv.status == LIGHTC_MAP_STATUS_DEC) {
                    if (pdesc->brightness > 1) {
                        step = -pdesc->priv.brightness_step_1_to_100_dec;
                    } else {
                        step = -pdesc->priv.brightness_step_1_percent_dec;
                    }
                }
            } else if (pdesc->priv.mode == LIGHTC_MAP_MODE_SET_BRIGHTNESS_BY_TIME || pdesc->priv.mode == LIGHTC_MAP_MODE_LOOP_BY_TIME) {
                if (pdesc->priv.status == LIGHTC_MAP_STATUS_INC) {
                    if (pdesc->brightness < 1) {
                        step = pdesc->priv.brightness_step_1_percent_inc;
                    } else if (pdesc->brightness < 100) { // fade_in_time
                        step = pdesc->priv.step_temp;
                    }
                } else if (pdesc->priv.status == LIGHTC_MAP_STATUS_DEC) {
                    if (pdesc->brightness > 1) {
                        step = -pdesc->priv.step_temp;
                    } else {
                        step = -pdesc->priv.brightness_step_1_to_100_dec;
                    }
                }
            }

            if (pdesc->priv.color_mode == LIGHTC_MAP_MODE_NORMAL) {
                if (pdesc->priv.color_status == LIGHTC_MAP_STATUS_INC) {
                    color_step = pdesc->priv.color_step_inc;
                } else if (pdesc->priv.color_status == LIGHTC_MAP_STATUS_DEC) {
                    color_step = -pdesc->priv.color_step_dec;
                }
            }
        } while (0);
        /* 3.update brightness */
        do {
            pdesc->brightness += step;
            if (pdesc->priv.status == LIGHTC_MAP_STATUS_INC) {
                if (pdesc->brightness > pdesc->priv.brightness_position) {
                    if (pdesc->priv.mode == LIGHTC_MAP_MODE_NORMAL) {
                        pdesc->brightness = pdesc->priv.brightness_position;
                    }
                    if (pdesc->priv.mode == LIGHTC_MAP_MODE_LOOP || pdesc->priv.mode == LIGHTC_MAP_MODE_LOOP_BY_TIME) {
                        pdesc->priv.brightness_position = 0;
                    }
                }
            } else if (pdesc->priv.status == LIGHTC_MAP_STATUS_DEC) {
                if (pdesc->brightness < pdesc->priv.brightness_position) {
                    if (pdesc->priv.mode == LIGHTC_MAP_MODE_NORMAL) {
                        pdesc->brightness = pdesc->priv.brightness_position;
                    }
                    if (pdesc->priv.mode == LIGHTC_MAP_MODE_LOOP || pdesc->priv.mode == LIGHTC_MAP_MODE_LOOP_BY_TIME) {
                        pdesc->priv.brightness_position = 100;
                    }
                }
            }
            _get_brightness_actual_from_brightness(pdesc);
            _get_pwm_duty_frequence(pdesc);
            if (pdesc->ops.update_brightness) {
                pdesc->ops.update_brightness(pdesc->priv.frequence, pdesc->priv.duty, pdesc->priv.iadj);
            }
        } while (0);
        do {
            pdesc->color += color_step;
            if (pdesc->priv.color_status == LIGHTC_MAP_STATUS_INC) {
                if (pdesc->color > pdesc->priv.color_postion) {
                    if (pdesc->priv.color_mode == LIGHTC_MAP_MODE_NORMAL) {
                        pdesc->color = pdesc->priv.color_postion;
                    }
                }
            } else if (pdesc->priv.color_status == LIGHTC_MAP_STATUS_DEC) {
                if (pdesc->color < pdesc->priv.color_postion) {
                    if (pdesc->priv.color_mode == LIGHTC_MAP_MODE_NORMAL) {
                        pdesc->color = pdesc->priv.color_postion;
                    }
                }
            }
            _get_color_pwm(pdesc);
            if (pdesc->ops.update_color) {
                pdesc->ops.update_color(pdesc->priv.color_duty);
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
static void _get_brightness_actual_from_brightness(lightc_map_describe_t *pdesc)
{
    ASSERT(pdesc);
    LIGHT_MAP_FLOAT_POINT_TYPE start_point, zero_point, stop_point;
    if (pdesc->param.dimming_start_point == 0) {
        start_point = (LIGHT_MAP_FLOAT_POINT_TYPE)(pdesc->param.dimming_end_point - pdesc->param.dimming_start_point) / 100;
        stop_point = pdesc->param.dimming_end_point;
        zero_point = 0;
    } else {
        start_point = pdesc->param.dimming_start_point;
        stop_point = pdesc->param.dimming_end_point;
        zero_point = (LIGHT_MAP_FLOAT_POINT_TYPE)(pdesc->param.dimming_start_point * pdesc->param.charge_duty) / 100;
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
 * @param {LIGHT_MAP_FLOAT_POINT_TYPE} *ret_duty
 * @return {*}
 */
static void _get_pwm_duty_frequence(lightc_map_describe_t *pdesc)
{
    brightness_map_t *bmap = pdesc->bmap;
    frequenct_map_t *fmap = pdesc->fmap;
    iadj_map_t *imap = pdesc->imap;

    LIGHT_MAP_FLOAT_POINT_TYPE x1, y1, x2, y2;
    LIGHT_MAP_FLOAT_POINT_TYPE brightness_actual = pdesc->priv.brightness_actual;

    if (bmap != NULL && bmap->node_size >= 2) {
        for (int i = 0; i < pdesc->bmap->node_size - 1; i++) {
            if (brightness_actual >= bmap->node[i].brightness && brightness_actual <= bmap->node[i + 1].brightness) {
                x1 = bmap->node[i].brightness;
                y1 = bmap->node[i].duty;
                x2 = bmap->node[i + 1].brightness;
                y2 = bmap->node[i + 1].duty;
                pdesc->priv.duty = y1 + ((y2 - y1) * (brightness_actual - x1)) / (x2 - x1);
            }
        }
    }
    if (fmap != NULL && fmap->node_size >= 2) {
        for (int i = 0; i < pdesc->fmap->node_size - 1; i++) {
            if (brightness_actual >= fmap->node[i].brightness && brightness_actual <= fmap->node[i + 1].brightness) {
                x1 = fmap->node[i].brightness;
                y1 = fmap->node[i].frequence;
                x2 = fmap->node[i + 1].brightness;
                y2 = fmap->node[i + 1].frequence;
                pdesc->priv.frequence = (uint32_t)(y1 + ((y2 - y1) * (brightness_actual - x1)) / (x2 - x1));
            }
        }
    } else {
        pdesc->priv.frequence = 1200; // default freq is 1200hz
    }

    if (imap != NULL && imap->node_size >= 2) {
        for (int i = 0; i < pdesc->imap->node_size - 1; i++) {
            if (brightness_actual >= imap->node[i].brightness && brightness_actual <= imap->node[i + 1].brightness) {
                x1 = imap->node[i].brightness;
                y1 = imap->node[i].duty;
                x2 = imap->node[i + 1].brightness;
                y2 = imap->node[i + 1].duty;
                pdesc->priv.iadj = y1 + ((y2 - y1) * (brightness_actual - x1)) / (x2 - x1);
            }
        }
    }
}

static void _get_color_pwm(lightc_map_describe_t *pdesc)
{

    LIGHT_MAP_FLOAT_POINT_TYPE x1, y1, x2, y2;
    LIGHT_MAP_FLOAT_POINT_TYPE color = pdesc->color;

    x1 = 2700;
    y1 = 0;
    x2 = 5600;
    y2 = 1;
    pdesc->priv.color_duty = y1 + ((y2 - y1) * (color - x1)) / (x2 - x1);
}

/* light brightness change api */
static int32_t __light_cmd_off(lightc_map_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;

    do {
        if (pdesc->is_virtual == true) {
            pdesc->virtual_brightness = 0;
            if (pdesc->xfer.lightc_cmd_off) {
                pdesc->xfer.lightc_cmd_off();
            }
            err = DRV_ERR_EOK;
            break;
        }

        if (pdesc->is_virtual == false) {
            pdesc->status.is_off = true;
            pdesc->priv.mode = LIGHTC_MAP_MODE_NORMAL;
            pdesc->priv.brightness_position = 0;
            err = DRV_ERR_EOK;
            break;
        }

    } while (0);

    return err;
}
static int32_t __light_cmd_on(lightc_map_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    double brightness = pdesc->priv.remeber_brightness;

    do {
        if (pdesc->is_virtual == true) {
            pdesc->virtual_brightness = brightness;
            if (pdesc->xfer.lightc_cmd_on) {
                pdesc->xfer.lightc_cmd_on();
            }
            err = DRV_ERR_EOK;
            break;
        }

        if (pdesc->is_virtual == false) {
            pdesc->status.is_off = false;
            pdesc->priv.mode = LIGHTC_MAP_MODE_NORMAL;
            pdesc->priv.brightness_position = brightness;
            err = DRV_ERR_EOK;
            break;
        }
    } while (0);

    return err;
}
static int32_t __light_set_brightness(lightc_map_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    union lightc_map_param *param = (union lightc_map_param *)args;
    double brightness = param->set.brightness;

    do {
        if (!args) {
            err = DRV_ERR_POINT_NONE;
            break;
        }
        if (brightness != 0) {
            pdesc->status.is_off = false;
        } else {
            pdesc->status.is_off = true;
        }

        if (pdesc->is_virtual == true) {
            pdesc->virtual_brightness = brightness;
            if (brightness != 0) {
                pdesc->priv.remeber_brightness = brightness;
            }
            if (pdesc->xfer.lightc_cmd_set_brightness) {
                pdesc->xfer.lightc_cmd_set_brightness(brightness);
            }
            err = DRV_ERR_EOK;
            break;
        }

        if (pdesc->is_virtual == false) {
            pdesc->priv.mode = LIGHTC_MAP_MODE_NORMAL;
            pdesc->priv.brightness_position = brightness;
            err = DRV_ERR_EOK;
        }
    } while (0);

    return err;
}
static int32_t __light_step_brightness_inc(lightc_map_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    double brightness = 0;

    do {
        if (pdesc->is_virtual == true) {
            brightness = pdesc->virtual_brightness + 5;
            if (brightness > 100) {
                brightness = 100;
            }
            pdesc->virtual_brightness = brightness;
            if (brightness != 0) {
                pdesc->priv.remeber_brightness = brightness;
            }
            if (pdesc->xfer.lightc_cmd_step_brightness_inc) {
                pdesc->xfer.lightc_cmd_step_brightness_inc();
            }
            err = DRV_ERR_EOK;
            break;
        }
        if (pdesc->is_virtual == false) {
            brightness = pdesc->priv.brightness_position + 5;
            if (brightness > 100) {
                brightness = 100;
            }
            pdesc->status.is_off = false;
            pdesc->priv.mode = LIGHTC_MAP_MODE_NORMAL;
            pdesc->priv.brightness_position = brightness;
        }
    } while (0);

    return err;
}
static int32_t __light_step_brightness_dec(lightc_map_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    double brightness = 0;
    do {
        if (pdesc->is_virtual == true) {
            brightness = pdesc->virtual_brightness - 5;
            if (brightness < 0) {
                brightness = 0;
            }
            pdesc->virtual_brightness = brightness;
            if (brightness != 0) {
                pdesc->priv.remeber_brightness = brightness;
            }
            if (pdesc->xfer.lightc_cmd_step_brightness_dec) {
                pdesc->xfer.lightc_cmd_step_brightness_dec();
            }
            err = DRV_ERR_EOK;
            break;
        }

        if (pdesc->is_virtual == false) {
            brightness = pdesc->priv.brightness_position - 5;
            if (brightness < 0) {
                brightness = 0;
            }
            if (brightness != 0) {
                pdesc->status.is_off = false;
            } else {
                pdesc->status.is_off = true;
            }
            pdesc->priv.mode = LIGHTC_MAP_MODE_NORMAL;
            pdesc->priv.brightness_position = brightness;
            err = DRV_ERR_OK;
        }
    } while (0);

    return err;
}
static int32_t __light_continue_brightness_inc(lightc_map_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_EOK;
    double brightness = 100;

    do {
        if (pdesc->is_virtual == true) {
            pdesc->brightness = brightness;
            if (pdesc->xfer.lightc_cmd_continue_brightness_inc) {
                pdesc->xfer.lightc_cmd_continue_brightness_inc();
            }
            break;
        }
        pdesc->priv.mode = LIGHTC_MAP_MODE_NORMAL;
        pdesc->priv.brightness_position = brightness;
    } while (0);

    return err;
}
static int32_t __light_continue_brightness_dec(lightc_map_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_EOK;
    double brightness = 0;

    do {
        if (pdesc->is_virtual == true) {
            pdesc->brightness = brightness;
            if (pdesc->xfer.lightc_cmd_continue_brightness_dec) {
                pdesc->xfer.lightc_cmd_continue_brightness_dec();
            }
            break;
        }
        pdesc->priv.mode = LIGHTC_MAP_MODE_NORMAL;
        pdesc->priv.brightness_position = brightness;
    } while (0);

    return err;
}
static int32_t __light_adjustment_finish(lightc_map_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_EOK;
    double brightness = pdesc->brightness;

    do {
        if (pdesc->is_virtual == true) {
            pdesc->brightness = brightness;
            if (pdesc->xfer.lightc_cmd_continue_brightness_finish) {
                pdesc->xfer.lightc_cmd_continue_brightness_finish();
            }
            break;
        }
        pdesc->priv.mode = LIGHTC_MAP_MODE_NORMAL;
        pdesc->priv.brightness_position = brightness;
    } while (0);

    return err;
}
static int32_t __loop_adjustment_start(lightc_map_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_EOK;
    do {
        if (pdesc->is_virtual == true) {
            if (pdesc->xfer.lightc_cmd_loop_light_adj_start) {
                pdesc->xfer.lightc_cmd_loop_light_adj_start();
            }
            break;
        }
        pdesc->priv.mode = LIGHTC_MAP_MODE_LOOP;
        if (pdesc->brightness == 100) {
            pdesc->priv.brightness_position = 0;
        } else {
            pdesc->priv.brightness_position = 100;
        }
    } while (0);

    return err;
}
static int32_t __loop_adjustment_stop(lightc_map_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_EOK;

    do {
        if (pdesc->is_virtual == true) {
            if (pdesc->xfer.lightc_cmd_loop_light_adj_stop) {
                pdesc->xfer.lightc_cmd_loop_light_adj_stop();
            }
            break;
        }
        double brightness = pdesc->brightness;
        pdesc->priv.mode = LIGHTC_MAP_MODE_NORMAL;
        pdesc->priv.brightness_position = brightness;
    } while (0);

    return err;
}
static int32_t __light_reverse(lightc_map_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_EOK;

    do {
        if (pdesc->priv.status == LIGHTC_MAP_STATUS_STOP && pdesc->brightness != 0) {
            __light_cmd_off(pdesc, args);
        } else if (pdesc->priv.status == LIGHTC_MAP_STATUS_STOP && pdesc->brightness == 0) {
            __light_cmd_on(pdesc, args);
        } else {
            if (pdesc->priv.status == LIGHTC_MAP_STATUS_INC) {
                __light_cmd_off(pdesc, args);
            } else if (pdesc->priv.status == LIGHTC_MAP_STATUS_DEC) {
                __light_cmd_on(pdesc, args);
            }
        }

        if (pdesc->is_virtual == true) {
            if (pdesc->xfer.lightc_cmd_reverse) {
                pdesc->xfer.lightc_cmd_reverse();
            }
        }
    } while (0);

    return err;
}
static int32_t __light_set_brightness_by_time(lightc_map_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_EOK;
    union lightc_map_param *param = (union lightc_map_param *)args;
    double brightness = param->set.brightness;

    if (brightness != 0) {
        pdesc->status.is_off = false;
    } else {
        pdesc->status.is_off = true;
    }

    pdesc->priv.brightness_position = brightness;
    pdesc->priv.mode = LIGHTC_MAP_MODE_SET_BRIGHTNESS_BY_TIME;
    pdesc->priv.step_temp = 99 / (LIGHT_MAP_FLOAT_POINT_TYPE)(pdesc->time_slice_frequence * param->set.move_time);
    return err;
}
static int32_t __light_reverse_ext(lightc_map_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_EOK;
    if (pdesc->priv.status == LIGHTC_MAP_STATUS_STOP && pdesc->brightness != 0) {
        __light_continue_brightness_dec(pdesc, args);
    } else if (pdesc->priv.status == LIGHTC_MAP_STATUS_STOP && pdesc->brightness == 0) {
        __light_continue_brightness_inc(pdesc, args);
    } else {
        if (pdesc->priv.status == LIGHTC_MAP_STATUS_INC) {
            __light_continue_brightness_dec(pdesc, args);
        } else if (pdesc->priv.status == LIGHTC_MAP_STATUS_DEC) {
            __light_continue_brightness_inc(pdesc, args);
        }
    }
    return err;
}
static int32_t __light_reverse_brightness(lightc_map_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_EOK;
    if (pdesc->priv.status == LIGHTC_MAP_STATUS_STOP && pdesc->brightness != 100) {
        __light_continue_brightness_inc(pdesc, args);
    } else if (pdesc->priv.status == LIGHTC_MAP_STATUS_STOP && pdesc->brightness == 100) {
        __light_continue_brightness_dec(pdesc, args);
    } else {
        if (pdesc->priv.status == LIGHTC_MAP_STATUS_INC) {
            __light_continue_brightness_dec(pdesc, args);
        } else if (pdesc->priv.status == LIGHTC_MAP_STATUS_DEC) {
            __light_continue_brightness_inc(pdesc, args);
        }
    }
    return err;
}

static int32_t __light_start(lightc_map_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_EOK;
    // init priv param
    if (pdesc->param.dimming_end_point <= pdesc->param.dimming_start_point) {
        err = DRV_ERR_WRONG_ARGS;
        return err;
    }
    if (pdesc->param.start_delay) {
        pdesc->priv.brightness_step_1_percent_inc = 1 * 10 / (LIGHT_MAP_FLOAT_POINT_TYPE)(pdesc->param.start_delay * pdesc->time_slice_frequence);
    } else {
        pdesc->priv.brightness_step_1_percent_inc = 1;
    }
    if (pdesc->param.start_delay) {
        pdesc->priv.brightness_step_1_percent_dec = 1 * 10 / (LIGHT_MAP_FLOAT_POINT_TYPE)(pdesc->param.stop_delay * pdesc->time_slice_frequence);
    } else {
        pdesc->priv.brightness_step_1_percent_dec = 1;
    }
    pdesc->priv.brightness_step_1_to_100_inc = 99 / (LIGHT_MAP_FLOAT_POINT_TYPE)(pdesc->time_slice_frequence * pdesc->param.fade_in_time);
    pdesc->priv.brightness_step_1_to_100_dec = 99 / (LIGHT_MAP_FLOAT_POINT_TYPE)(pdesc->time_slice_frequence * pdesc->param.fade_out_time);
    if (pdesc->param.start_state == 1) {
        __light_cmd_on(pdesc, NULL);
    }
    return err;
}

static int32_t __light_param_write(lightc_map_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_EOK;
    union lightc_map_param *param = (union lightc_map_param *)args;

    uint8_t start = param->param.start;
    uint8_t size = param->param.size;

    for (uint8_t i = 0; i < size; i++) {
        if (((uint8_t *)(&pdesc->param.light_type))[start + i] == ((uint8_t *)(&param->param.param.light_type))[start + i]) {
            continue;
        }
        ((uint8_t *)(&pdesc->param.light_type))[start + i] = ((uint8_t *)(&param->param.param.light_type))[start + i];
        // param check
        if (start + i == offsetof(union lightc_map_param, param.param.dimming_start_point)) {
            if (pdesc->param.dimming_start_point > pdesc->param.dimming_end_point) {
                pdesc->param.dimming_start_point = 0;
            }
        } else if (start + i == offsetof(union lightc_map_param, param.param.dimming_end_point)) {
            if (pdesc->param.dimming_end_point < pdesc->param.dimming_start_point) {
                pdesc->param.dimming_end_point = 100;
            }
        }
    }
    if (pdesc->param.start_delay) {
        pdesc->priv.brightness_step_1_percent_inc = 1 * 10 / (LIGHT_MAP_FLOAT_POINT_TYPE)(pdesc->param.start_delay * pdesc->time_slice_frequence);
    } else {
        pdesc->priv.brightness_step_1_percent_inc = 1;
    }
    if (pdesc->param.start_delay) {
        pdesc->priv.brightness_step_1_percent_dec = 1 * 10 / (LIGHT_MAP_FLOAT_POINT_TYPE)(pdesc->param.stop_delay * pdesc->time_slice_frequence);
    } else {
        pdesc->priv.brightness_step_1_percent_dec = 1;
    }
    pdesc->priv.brightness_step_1_to_100_inc = 99 / (LIGHT_MAP_FLOAT_POINT_TYPE)(pdesc->time_slice_frequence * pdesc->param.fade_in_time);
    pdesc->priv.brightness_step_1_to_100_dec = 99 / (LIGHT_MAP_FLOAT_POINT_TYPE)(pdesc->time_slice_frequence * pdesc->param.fade_out_time);
    return err;
}

static int32_t __light_adjustment_start_by_time(lightc_map_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_EOK;
    pdesc->priv.mode = LIGHTC_MAP_MODE_LOOP_BY_TIME;
    if (pdesc->brightness == 100) {
        pdesc->priv.brightness_position = 0;
    } else {
        pdesc->priv.brightness_position = 100;
    }
    union lightc_map_param *param = (union lightc_map_param *)args;
    pdesc->priv.step_temp = 99 / (LIGHT_MAP_FLOAT_POINT_TYPE)(pdesc->time_slice_frequence * param->set.move_time);
    return err;
}

static int32_t __light_get_brightness(lightc_map_describe_t *pdesc, void *args)
{
    union lightc_map_param *param = (union lightc_map_param *)args;
    if (pdesc->is_virtual == true) {
        param->get.brightness = pdesc->virtual_brightness;
    } else {
        param->get.brightness = pdesc->priv.brightness_position;
    }
    return DRV_ERR_EOK;
}

static int32_t __light_set_virtual_brightness(lightc_map_describe_t *pdesc, void *args)
{
    union lightc_map_param *param = (union lightc_map_param *)args;
    if (pdesc->is_virtual == true) {
        pdesc->virtual_brightness = param->set.brightness;
    } else {
        param->get.brightness = pdesc->priv.brightness_position;
    }

    return DRV_ERR_EOK;
}

static int32_t __light_set_color(lightc_map_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    union lightc_map_param *param = (union lightc_map_param *)args;
    double color = param->set.color;

    do {
        if (!args) {
            err = DRV_ERR_POINT_NONE;
            break;
        }

        if (pdesc->is_virtual == true) {
            err = DRV_ERR_EOK;
            break;
        }

        if (param->set.color > 5600 || param->set.color < 2700) {
            break;
        }

        if (pdesc->is_virtual == false) {
            pdesc->priv.color_mode = LIGHTC_MAP_MODE_NORMAL;
            pdesc->priv.color_postion = color;
            err = DRV_ERR_EOK;
        }
    } while (0);

    return err;
}
/*---------- end of file ----------*/
