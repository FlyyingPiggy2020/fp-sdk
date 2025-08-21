/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : roller_blind_control.c
 * @Author       : lxf
 * @Date         : 2025-05-14 15:36:33
 * @LastEditors: Lu Xianfan 154562451@qq.com
 * @LastEditTime: 2025-08-21 16:03:28
 * @Brief        : 卷帘,百叶帘的控制业务逻辑驱动程序
 * 该驱动需要保证 up 的行程值大于 down的行程值。open增加行程，close减少行程。两者才会不变。
 * 另外运行irq_handler的时基默认为1ms，暂时不支持修改
 * 依赖：butter.h
 * 电流检测部分运放比例为:3.02*adc=安培
 */

/*---------- includes ----------*/
#include "roller_blind_control.h"
#include "driver.h"
#include "drv_err.h"
#include "fpevent.h"
#include "options.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t _motor_open(driver_t **pdrv);
static void _motor_close(driver_t **pdrv);
static int32_t _motor_ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static int32_t _motor_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length);

// api
static int32_t _motor_to_open_up(roller_blind_control_describe_t *pdesc, void *args);
static int32_t _motor_to_close_down(roller_blind_control_describe_t *pdesc, void *args);
static int32_t _motor_stop(roller_blind_control_describe_t *pdesc, void *args);
static int32_t _motor_goto_xx_pre(roller_blind_control_describe_t *pdesc, void *args);
static int32_t _motor_set_up_route(roller_blind_control_describe_t *pdesc, void *args);
static int32_t _motor_set_down_route(roller_blind_control_describe_t *pdesc, void *args);
static int32_t _motor_clear_route(roller_blind_control_describe_t *pdesc, void *args);
static int32_t _motor_turn(roller_blind_control_describe_t *pdesc, void *args);

static int32_t _motor_get_status(roller_blind_control_describe_t *pdesc, void *args);
static int32_t _motor_is_free(roller_blind_control_describe_t *pdesc, void *args);
static int32_t _motor_get_route(roller_blind_control_describe_t *pdesc, void *args);

// priv
static int32_t __motor_run_inc(roller_blind_control_describe_t *pdesc, void *args);
static int32_t __motor_stop_without_logic(roller_blind_control_describe_t *pdesc);
static int32_t __motor_run_dec(roller_blind_control_describe_t *pdesc, void *args);

/*---------- variable ----------*/
DRIVER_DEFINED(rb_motorc, _motor_open, _motor_close, NULL, NULL, _motor_ioctl, _motor_irq_handler);

static struct protocol_callback ioctl_cbs[] = {
    { IOCTL_MOTORC_OPEN, _motor_to_open_up },
    { IOCTL_MOTORC_CLOSE, _motor_to_close_down },
    { IOCTL_MOTORC_STOP, _motor_stop },
    { IOCTL_MOTORC_GOTO_ROUTE, _motor_goto_xx_pre },
    { IOCTL_MOTORC_SET_UP_ROUTE, _motor_set_up_route },
    { IOCTL_MOTORC_SET_DOWN_ROUTE, _motor_set_down_route },
    { IOCTL_MOTORC_CLEAR_ROUTE, _motor_clear_route },
    { IOCTL_MOTORC_TURN, _motor_turn },
    { IOCTL_MOTORC_GET_STATUS, _motor_get_status },
    { IOCTL_MOTORC_ROUTE_IS_FREE, _motor_is_free },
    { IOCTL_MOTORC_GET_ROUTE, _motor_get_route },
};
/*---------- function ----------*/

/**
 * @brief 返回两个值之间的绝对差值
 * @param {int32} v1
 * @param {int32} v2
 * @return {*}
 */
static uint32_t abs32(int32_t v1, int32_t v2)
{
    int32_t v = v1;
    v -= v2;
    if (v < 0)
        v = 0 - v;
    return (v);
}

static int32_t _motor_open(driver_t **pdrv)
{
    roller_blind_control_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_WRONG_ARGS;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if (!pdesc) {
            break;
        }

        if (!pdesc->ops.save_config || !pdesc->ops.load_config) {
            break;
        }

        // 读取失败,重新初始化
        if (pdesc->ops.load_config() == false) {
            pdesc->config.route_curr = 0;
            pdesc->config.route_down = MOTOR_ROUTE_FREE;
            pdesc->config.route_up = MOTOR_ROUTE_FREE;
            pdesc->ops.save_config();
        }

        // 设置默认参数
        pdesc->param.space_min = MOTOR_SPACE_MIN;
        pdesc->param.time_stop_delay = MOTOR_SWITCH_TIME;
        if (pdesc->ops.init) {
            pdesc->ops.init();
        }
        err = DRV_ERR_EOK;
    } while (0);

    return err;
}

static void _motor_close(driver_t **pdrv)
{
    roller_blind_control_describe_t *pdesc = NULL;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if (!pdesc) {
            break;
        }

        if (pdesc->ops.deinit) {
            pdesc->ops.deinit();
        }
    } while (0);

    return;
}
static int32_t _motor_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    roller_blind_control_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_WRONG_ARGS;
    int32_t (*cb)(roller_blind_control_describe_t *, void *) = NULL;

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

static void _motor_route_control(roller_blind_control_describe_t *pdesc)
{
    bool isMustStop = false;
    uint8_t event = 0;
    if (pdesc->priv.control.target_route != MOTOR_ROUTE_FREE) {
        if (pdesc->priv.state.state_curr == MSTATE_RUN_INC) {
            if (pdesc->priv.control.target_route != MOTOR_ROUTE_INC_MAX) {
                if (pdesc->config.route_curr >= pdesc->priv.control.target_route) {
                    isMustStop = true;
                }
            }
        }
        if (pdesc->priv.state.state_curr == MSTATE_RUN_DEC) {
            if (pdesc->priv.control.target_route != MOTOR_ROUTE_DEC_MAX) {
                if (pdesc->config.route_curr <= pdesc->priv.control.target_route) {
                    isMustStop = true;
                }
            }
        }
    }

    if (isMustStop) {
        _motor_stop(pdesc, NULL);
        if (pdesc->cb.stop_cb) {
            pdesc->cb.stop_cb();
        }
    }
}

static void _motor_speed_control(roller_blind_control_describe_t *pdesc)
{
    float speed = 0;

    do {
        if (!pdesc->ops.set_speed) {
            break;
        }
        if (pdesc->priv.state.state_curr == MSTATE_STOP) {
            break;
        }

        // this is a route loop, us kp only.
        if (pdesc->priv.control.target_route != MOTOR_ROUTE_FREE) {
            int32_t route_e = abs32(pdesc->priv.control.target_route, pdesc->config.route_curr);
            speed = pdesc->priv.speed.kp * route_e;
            if (speed < pdesc->priv.speed.min) {
                speed = pdesc->priv.speed.min;
            } else if (speed > pdesc->priv.speed.max) {
                speed = pdesc->priv.speed.max;
            }
            pdesc->ops.set_speed(speed);
        } else {
            pdesc->ops.set_speed(pdesc->priv.speed.max);
        }
    } while (0);
}

static void _motor_resistance_control(roller_blind_control_describe_t *pdesc)
{
    if (++pdesc->priv.current.time >= __ticks2ms(50) && pdesc->ops.get_motor_current) {
        pdesc->priv.current.time = 0;
        pdesc->priv.current.current = pdesc->ops.get_motor_current();
        if (pdesc->priv.state.is_running == MSTATE_STOP) {
            pdesc->priv.current.idle = pdesc->priv.current.current;
        }
        pdesc->priv.current.load = pdesc->priv.current.current - pdesc->priv.current.idle;
    }

    if (pdesc->priv.state.is_running == MSTATE_STOP) {
        pdesc->priv.flag.is_resistance = false;
        return;
    }

    // 1.电流遇阻 TODO：这部分代码依赖于特定硬件需要解耦合
    if (pdesc->ops.get_motor_current) {
        if (pdesc->priv.current.mutex == 0) {
            if (pdesc->priv.current.load > 500) {
                pdesc->priv.current.mutex = 1;
                pdesc->priv.flag.is_resistance = true;
            }
        } else {
            if (pdesc->priv.current.load < 370) {
                pdesc->priv.current.mutex = 0;
            }
        }
    }

    // 2.遇阻停止
    if (pdesc->priv.flag.is_resistance == true) {
        _motor_stop(pdesc, NULL);
    }
}
static int32_t _motor_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length)
{
    roller_blind_control_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_WRONG_ARGS;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if (!pdesc) {
            break;
        }

        do {
            if (pdesc->priv.control.time_stop_delay) {
                pdesc->priv.control.time_stop_delay--;
                __motor_stop_without_logic(pdesc);
            } else if (pdesc->priv.state.state_curr == MSTATE_RUN_INC) {
                __motor_run_inc(pdesc, NULL);
                pdesc->priv.flag.run_time++;
            } else if (pdesc->priv.state.state_curr == MSTATE_RUN_DEC) {
                __motor_run_dec(pdesc, NULL);
                pdesc->priv.flag.run_time++;
            } else if (pdesc->priv.flag.run_time) {
                pdesc->priv.flag.run_time--;
            }
            _motor_route_control(pdesc);
            _motor_speed_control(pdesc);
            _motor_resistance_control(pdesc);

            // 运行保护
            if (pdesc->priv.flag.run_time > __ticks2ms(6 * 60 * 1000)) {
                pdesc->priv.flag.run_time = 0;
                _motor_stop(pdesc, NULL);
            }

            if (pdesc->priv.state.disable_excloop > 0) {
                pdesc->priv.state.disable_excloop--;
            }

            if (pdesc->priv.state.start_update_time > 0) {
                pdesc->priv.state.start_update_time--;
            }
        } while (0);

    } while (0);

    return err;
}

/**
 * @brief 电机停止,但是逻辑上不停止(用于正反转切换时，不立即切换，而是先停止这个逻辑)
 * @param {roller_blind_control_describe_t} *pdesc
 * @param {void} *args
 * @return {*}
 */
static int32_t __motor_stop_without_logic(roller_blind_control_describe_t *pdesc)
{
    int32_t ret = DRV_ERR_ERROR;
    if (pdesc->ops.motor_stop) {
        pdesc->ops.motor_stop();
        ret = DRV_ERR_EOK;
    }

    if (pdesc->priv.state.is_running != MSTATE_STOP) {
        pdesc->priv.state.is_running = MSTATE_STOP;
    }
    return ret;
}

/**
 * @brief 电机正转
 * @param {roller_blind_control_describe_t} *pdesc
 * @param {void} *args
 * @return {*}
 */
static int32_t __motor_run_inc(roller_blind_control_describe_t *pdesc, void *args)
{
    int32_t ret = DRV_ERR_ERROR;
    if (pdesc->priv.state.is_running != MSTATE_RUN_INC) {
        pdesc->ops.motor_inc();
        ret = DRV_ERR_EOK;
    }
    pdesc->priv.state.is_running = MSTATE_RUN_INC;
    return ret;
}

/**
 * @brief 电机反转
 * @param {roller_blind_control_describe_t} *pdesc
 * @param {void} *args
 * @return {*}
 */
static int32_t __motor_run_dec(roller_blind_control_describe_t *pdesc, void *args)
{
    int32_t ret = DRV_ERR_ERROR;
    if (pdesc->priv.state.is_running != MSTATE_RUN_DEC) {
        pdesc->ops.motor_dec();
        ret = DRV_ERR_EOK;
    }
    pdesc->priv.state.is_running = MSTATE_RUN_DEC;
    return ret;
}

static bool __motor_goto_route(roller_blind_control_describe_t *pdesc, int32_t route)
{
    bool ret = false;
    do {
        if (route == MOTOR_ROUTE_FREE) {
            break;
        }

        if (abs32(pdesc->config.route_curr, route) <= MOTOR_SPACE_MIN) {
            break;
        }

        if (!pdesc->ops.motor_stop) {
            break;
        }

        pdesc->priv.control.target_route = route;
        pdesc->priv.control.route_start = route;
        if (pdesc->priv.control.target_route > pdesc->config.route_curr) {
            if (pdesc->priv.state.state_curr == MSTATE_RUN_DEC) {
                pdesc->priv.control.time_stop_delay = MOTOR_SWITCH_TIME;
                __motor_stop_without_logic(pdesc);
            }
            pdesc->priv.state.state_curr = MSTATE_RUN_INC;
        } else if (pdesc->priv.control.target_route < pdesc->config.route_curr) {
            if (pdesc->priv.state.state_curr == MSTATE_RUN_INC) {
                pdesc->priv.control.time_stop_delay = MOTOR_SWITCH_TIME;
                __motor_stop_without_logic(pdesc);
            }
            pdesc->priv.state.state_curr = MSTATE_RUN_DEC;
        }
        ret = true;
    } while (0);

    return ret;
}

/**
 * @brief 电机停止-api
 * @param {roller_blind_control_describe_t} *pdesc
 * @param {void} *args
 * @return {*}
 */
static int32_t _motor_stop(roller_blind_control_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_ERROR;

    do {
        pdesc->priv.state.state_curr = MSTATE_STOP;
        pdesc->priv.control.target_route = MOTOR_ROUTE_FREE;

        if (!pdesc->ops.motor_stop) {
            err = DRV_ERR_POINT_NONE;
            break;
        }

        pdesc->ops.motor_stop();

        if (pdesc->priv.state.is_running != MSTATE_STOP) {
            pdesc->priv.state.is_running = MSTATE_STOP;
        }

        err = DRV_ERR_OK;
    } while (0);

    return err;
}

/**
 * @brief 电机上行-api
 * @param {roller_blind_control_describe_t} *pdesc
 * @param {void} *args
 * @return {*}
 */
static int32_t _motor_to_open_up(roller_blind_control_describe_t *pdesc, void *args)
{
    union roller_blind_control_param *param = args;
    bool ret = false;
    uint8_t mode;

    if (!args) {
        return DRV_ERR_POINT_NONE;
    }

    mode = param->run.mode;
    // 同方向遇阻不再运行

    if (EVENT_IS_AND_BIT_SET(pdesc->priv.flag.state, MFLAG_RESISTANCE_INC)) {
        return DRV_ERR_ERROR;
    }

    pdesc->priv.state.last_event = MSTATE_RUN_INC;

    if ((pdesc->config.route_up == MOTOR_ROUTE_FREE) || (mode & MMODE_OVERROUTE)) {
        if ((mode & 0x0f) & MMODE_SSTEP) {
            // 点动模式
            ret = __motor_goto_route(pdesc, pdesc->config.route_curr + JOG_HALL);
        } else {
            // 正常模式
            ret = __motor_goto_route(pdesc, MOTOR_ROUTE_INC_MAX);
        }
    } else {
        // 防止点动超出限位
        if (((mode & 0x0f) & MMODE_SSTEP) && (pdesc->config.route_curr < pdesc->config.route_up - JOG_HALL)) {
            // 点动模式
            ret = __motor_goto_route(pdesc, pdesc->config.route_curr + JOG_HALL);
        } else {
            // 正常模式
            ret = __motor_goto_route(pdesc, pdesc->config.route_up);
        }
    }

    if (ret == true) {
        return DRV_ERR_EOK;
    }
    return DRV_ERR_ERROR;
}

static int32_t _motor_to_close_down(roller_blind_control_describe_t *pdesc, void *args)
{
    union roller_blind_control_param *param = args;

    bool ret = false;
    // 同方向遇阻不再运行
    if (!args) {
        return DRV_ERR_POINT_NONE;
    }
    uint8_t mode = param->run.mode;
    if (EVENT_IS_AND_BIT_SET(pdesc->priv.flag.state, MSTATE_RUN_DEC)) {
        return DRV_ERR_ERROR;
    }

    pdesc->priv.state.last_event = MSTATE_RUN_DEC;

    if ((pdesc->config.route_down == MOTOR_ROUTE_FREE) || (mode & MMODE_OVERROUTE)) {
        if ((mode & 0x0f) & MMODE_SSTEP) {
            // 点动模式
            ret = __motor_goto_route(pdesc, pdesc->config.route_curr - JOG_HALL);
        } else {
            // 正常模式
            ret = __motor_goto_route(pdesc, MOTOR_ROUTE_DEC_MAX);
        }
    } else {
        if (((mode & 0x0f) & MMODE_SSTEP) && pdesc->config.route_curr > pdesc->config.route_down + JOG_HALL) {
            // 点动模式
            ret = __motor_goto_route(pdesc, pdesc->config.route_curr - JOG_HALL);
        } else {
            // 正常模式
            ret = __motor_goto_route(pdesc, pdesc->config.route_down);
        }
    }

    if (ret == true) {
        return DRV_ERR_EOK;
    }
    return DRV_ERR_ERROR;
}

static int32_t _motor_get_status(roller_blind_control_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    union roller_blind_control_param *param = (union roller_blind_control_param *)args;

    do {
        if (!param) {
            break;
        }
        param->get.status = pdesc->priv.state.state_curr;
        err = DRV_ERR_EOK;
    } while (0);
    return err;
}

static int32_t _motor_is_free(roller_blind_control_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    union roller_blind_control_param *param = (union roller_blind_control_param *)args;

    do {
        if (!param) {
            break;
        }

        if (pdesc->config.route_up == MOTOR_ROUTE_FREE) {
            param->get.is_route_free = true;
            break;
        }

        if (pdesc->config.route_down == MOTOR_ROUTE_FREE) {
            param->get.is_route_free = true;
            break;
        }
        param->get.is_route_free = false;
        err = DRV_ERR_EOK;
    } while (0);
    return err;
}

static int32_t _motor_get_route(roller_blind_control_describe_t *pdesc, void *args)
{
    int32_t err = DRV_ERR_WRONG_ARGS;
    union roller_blind_control_param *param = (union roller_blind_control_param *)args;

    do {
        if (!param) {
            break;
        }

        _motor_is_free(pdesc, param);

        if (param->get.is_route_free == true) {
            err = DRV_ERR_EOK;
            param->get.route = 0xff;
            break;
        }

        int32_t max, min, x;
        max = pdesc->config.route_up;
        min = pdesc->config.route_down;
        x = max - min;

        int32_t y = pdesc->config.route_curr;
        y -= min;

        x = y * (2 * 100) / x;
        x /= 2;
        if (x < 0) {
            x = 0;
        }
        if (x > 100) {
            x = 100;
        }
        param->get.route = x;
        err = DRV_ERR_EOK;
    } while (0);
    return err;
}

static int32_t _motor_goto_xx_pre(roller_blind_control_describe_t *pdesc, void *args)
{
    union roller_blind_control_param *param = (union roller_blind_control_param *)args;
    int32_t ret = DRV_ERR_ERROR;

    do {
        if (!pdesc || !args) {
            break;
        }

        if (param->run.route > 100) {
            break;
        }

        if (pdesc->config.route_up == MOTOR_ROUTE_FREE) {
            break;
        }

        if (pdesc->config.route_down == MOTOR_ROUTE_FREE) {
            break;
        }

        int32_t route = 0;
        int32_t max;
        int32_t min;
        int32_t x;
        uint8_t pre = param->run.route;

        max = pdesc->config.route_up;
        min = pdesc->config.route_down;

        do {
            if (pre == 0) {
                route = min;
                break;
            } else if (pre == 100) {
                route = max;
                break;
            }
            x = max - min;
            x *= 2 * pre + 1;
            x /= (2 * 100);
            x += min;
            route = x;
        } while (0);
        __motor_goto_route(pdesc, route);
        ret = DRV_ERR_EOK;
    } while (0);
    return ret;
}

static int32_t _motor_set_up_route(roller_blind_control_describe_t *pdesc, void *args)
{
    int32_t ret = DRV_ERR_ERROR;
    _motor_stop(pdesc, NULL);

    do {
        if (pdesc->config.route_up != MOTOR_ROUTE_FREE) {
            break;
        }
        if (pdesc->config.route_down != MOTOR_ROUTE_FREE) {
            if (pdesc->config.route_curr - pdesc->config.route_down < MOTOR_ROUTE_MIN) {
                break;
            }
        }
        pdesc->config.route_up = pdesc->config.route_curr;
        ret = DRV_ERR_EOK;
    } while (0);

    return ret;
}
static int32_t _motor_set_down_route(roller_blind_control_describe_t *pdesc, void *args)
{
    int32_t ret = DRV_ERR_ERROR;
    _motor_stop(pdesc, NULL);

    do {
        if (pdesc->config.route_down != MOTOR_ROUTE_FREE) {
            break;
        }
        if (pdesc->config.route_up != MOTOR_ROUTE_FREE) {
            if (pdesc->config.route_up - pdesc->config.route_curr < MOTOR_ROUTE_MIN) {
                break;
            }
        }
        pdesc->config.route_down = pdesc->config.route_curr;
        ret = DRV_ERR_EOK;
    } while (0);

    return ret;
}
static int32_t _motor_clear_route(roller_blind_control_describe_t *pdesc, void *args)
{
    int32_t ret = DRV_ERR_ERROR;
    _motor_stop(pdesc, NULL);

    do {
        pdesc->config.route_up = MOTOR_ROUTE_FREE;
        pdesc->config.route_down = MOTOR_ROUTE_FREE;
        ret = DRV_ERR_EOK;
    } while (0);

    return ret;
}

static int32_t _motor_turn(roller_blind_control_describe_t *pdesc, void *args)
{
    int32_t ret = DRV_ERR_ERROR;
    union roller_blind_control_param param;
    param.run.mode = MMODE_NORMAL;
    do {
        if (pdesc->priv.state.disable_excloop) {
            break;
        }
        if (pdesc->config.route_up == MOTOR_ROUTE_FREE) {
            break;
        }
        if (pdesc->config.route_down == MOTOR_ROUTE_FREE) {
            break;
        }

        pdesc->priv.state.disable_excloop = 500;

        if (pdesc->priv.state.state_curr != MSTATE_STOP) {
            _motor_stop(pdesc, NULL);
        } else if (pdesc->priv.state.last_event == MSTATE_RUN_DEC) {
            _motor_to_open_up(pdesc, &param);
        } else {
            _motor_to_close_down(pdesc, &param);
        }
        ret = DRV_ERR_EOK;
    } while (0);

    return ret;
}

/*---------- end of file ----------*/
