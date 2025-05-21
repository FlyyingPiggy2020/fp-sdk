/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : roller_blind_control.c
 * @Author       : lxf
 * @Date         : 2025-05-14 15:36:33
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-05-20 10:06:20
 * @Brief        : 卷帘,百叶帘的控制业务逻辑驱动程序
 */

/*---------- includes ----------*/
#include "roller_blind_control.h"
#include "driver.h"
#include "drv_err.h"
#include "fpevent.h"

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t _motor_open(driver_t **pdrv);
static int32_t _motor_close(driver_t **pdrv);
static int32_t _motor_ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static int32_t _motor_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length);

// api
static int32_t _motor_stop(roller_blind_control_describe_t *pdesc, void *args);

// priv
static int32_t __motor_run_inc(roller_blind_control_describe_t *pdesc, void *args);
static int32_t __motor_stop_without_logic(roller_blind_control_describe_t *pdesc);
static int32_t __motor_run_dec(roller_blind_control_describe_t *pdesc, void *args);
/*---------- variable ----------*/
DRIVER_DEFINED(rb_motorc, _motor_open, _motor_close, NULL, NULL, _motor_ioctl, _motor_irq_handler);

static struct protocol_callback ioctl_cbs[] = {
    { IOCTL_MOTORC_OPEN, NULL },
    { IOCTL_MOTORC_CLOSE, NULL },
    { IOCTL_MOTORC_STOP, _motor_stop },
    { IOCTL_MOTORC_GOTO_ROUTE, NULL },
    { IOCTL_MOTORC_SET_UP_ROUTE, NULL },
    { IOCTL_MOTORC_SET_DOWN_ROUTE, NULL },
    { IOCTL_MOTORC_CLEAR_ROUTE, NULL },
    { IOCTL_MOTORC_TURN, NULL },
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
    } while (0);

    return err;
}
static int32_t _motor_close(driver_t **pdrv)
{
    roller_blind_control_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_WRONG_ARGS;

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

    return err;
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
        cb = (int32_t(*)(roller_blind_control_describe_t *, void *))protocol_callback_find(cmd, ioctl_cbs, ARRAY_SIZE(ioctl_cbs));
        if (!cb) {
            break;
        }
        err = cb(pdesc, args);
    } while (0);

    return err;
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
    if (pdesc->ops.motor_stop) {
        pdesc->ops.motor_stop();
    }

    if (pdesc->priv.state.is_running != MSTATE_STOP) {
        pdesc->priv.state.is_running = MSTATE_STOP;
    }
}

/**
 * @brief 电机正转
 * @param {roller_blind_control_describe_t} *pdesc
 * @param {void} *args
 * @return {*}
 */
static int32_t __motor_run_inc(roller_blind_control_describe_t *pdesc, void *args)
{
    if (pdesc->priv.state.is_running != MSTATE_RUN_INC) {
        pdesc->ops.motor_inc();
    }
    pdesc->priv.state.is_running = MSTATE_RUN_INC;
}

/**
 * @brief 电机反转
 * @param {roller_blind_control_describe_t} *pdesc
 * @param {void} *args
 * @return {*}
 */
static int32_t __motor_run_dec(roller_blind_control_describe_t *pdesc, void *args)
{
    if (pdesc->priv.state.is_running != MSTATE_RUN_DEC) {
        pdesc->ops.motor_dec();
    }
    pdesc->priv.state.is_running = MSTATE_RUN_DEC;
}

static bool __motor_goto_route(roller_blind_control_describe_t *pdesc, int32_t route)
{
    bool ret = false;
    do {
        if (route == MOTOR_ROUTE_FREE) {
            break;
        }

        if (abs32(pdesc->config.route_curr, route) <= pdesc->param.space_min) {
            break;
        }

        if (!pdesc->ops.motor_stop) {
            break;
        }

        pdesc->priv.control.target_route = route;
        if (pdesc->priv.control.target_route > pdesc->config.route_curr) {
            if (pdesc->priv.state.state_curr == MSTATE_RUN_DEC) {
                pdesc->priv.control.time_stop_delay = pdesc->param.time_stop_delay;
                __motor_stop_without_logic(pdesc);
            }

        } else if (pdesc->priv.control.target_route < pdesc->config.route_curr) {
        }
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
    // 同方向遇阻不再运行
    uint8_t mode = (uint8_t *)args;

    if (EVENT_IS_AND_BIT_SET(pdesc->priv.flag.state, MFLAG_RESISTANCE_INC)) {
        return DRV_ERR_ERROR;
    }

    if ((pdesc->config.route_up == MOTOR_ROUTE_FREE) || (mode & MMODE_OVERROUTE)) {
        if ((mode & 0x0f) & MMODE_SSTEP) {
        }
    }
}
static int32_t _motor_to_close_down(roller_blind_control_describe_t *pdesc, void *args)
{
}
/*---------- end of file ----------*/
