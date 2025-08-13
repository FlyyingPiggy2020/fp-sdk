/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : hall.c
 * @Author       : lxf
 * @Date         : 2025-05-21 13:31:44
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-05-22 11:27:01
 * @Brief        : 霍尔传感器驱动程序
 * 需要把iqrhandler放入一个200us的中断内部执行
 */

/*---------- includes ----------*/
#include "hall.h"
#include "driver.h"
#include "drv_err.h"
#include "fpevent.h"

/*---------- macro ----------*/
/*---------- type define ----------*/
#define HALL1 (1 << 0) // 霍尔线1
#define HALL2 (1 << 1) // 霍尔线2
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t _hall_open(driver_t **pdrv);
static void _hall_close(driver_t **pdrv);
static int32_t _hall_ioctl(driver_t **pdrv, uint32_t cmd, void *args);
static int32_t _hall_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length);

static int32_t _hall_get_speed(hall_describe_t *pdesc, void *args);
static int32_t _hall_get_route(hall_describe_t *pdesc, void *args);

/*---------- variable ----------*/
DRIVER_DEFINED(hall, _hall_open, _hall_close, NULL, NULL, _hall_ioctl, _hall_irq_handler);

static struct protocol_callback ioctl_cbs[] = {
    { IOCTL_HALL_GET_SPEED, _hall_get_speed },
    { IOCTL_HALL_GET_ROUTE, _hall_get_route },
};
/*---------- function ----------*/

static int32_t _hall_open(driver_t **pdrv)
{
    hall_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_ERROR;

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

        if (!pdesc->ops.init()) {
            err = DRV_ERR_ERROR;
            break;
        }
        err = DRV_ERR_EOK;
    } while (0);

    return err;
}
static void _hall_close(driver_t **pdrv)
{
    hall_describe_t *pdesc = NULL;

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
}
static int32_t _hall_ioctl(driver_t **pdrv, uint32_t cmd, void *args)
{
    hall_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_WRONG_ARGS;
    int32_t (*cb)(hall_describe_t *, void *) = NULL;

    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;
    do {
        if (!pdesc) {
            break;
        }
        cb = (int32_t(*)(hall_describe_t *, void *))protocol_callback_find(cmd, ioctl_cbs, ARRAY_SIZE(ioctl_cbs));
        if (!cb) {
            break;
        }
        err = cb(pdesc, args);
    } while (0);

    return err;
}

/**
 * @brief 读取hall传感器的电平
 * @param {hall_describe_t} *pdesc
 * @return {*}
 */
static uint8_t get_hall_state(hall_describe_t *pdesc)
{
    uint8_t state = 0;
    if (!pdesc->ops.hall1_read || !pdesc->ops.hall2_read) {
        return state;
    }

    if (pdesc->ops.hall1_read() == false) {
        state |= HALL1;
    }
    if (pdesc->ops.hall2_read() == false) {
        state |= HALL2;
    }
    return state;
}

/**
 * @brief 判断相位
 * @param {hall_describe_t} *pdesc
 * @return {*}
 */
static bool hall_phase_detect(hall_describe_t *pdesc)
{
    uint16_t hall_change = 0;
    volatile uint8_t tmp = get_hall_state(pdesc);
    volatile uint8_t dir = MDIRECTION_DEC;

    if (tmp == pdesc->priv.hall_state) {
        hall_change = (pdesc->priv.hall_state ^ pdesc->priv.hall_state_last) & (HALL1 | HALL2);
        if (hall_change == 0) {
            return false;
        }
        pdesc->priv.dir = hall_change;
        pdesc->priv.hall_state_last = pdesc->priv.hall_state;

        // 识别方向
        if (hall_change == HALL1) {
            if (pdesc->priv.hall_state == 0x00 || pdesc->priv.hall_state == 0x03) {
                dir = MDIRECTION_INC;
            }
        } else if (hall_change == HALL2) {
            if (pdesc->priv.hall_state == 0x01 || pdesc->priv.hall_state == 0x02) {
                dir = MDIRECTION_INC;
            }
        } else {
            return false;
        }

        if (dir == MDIRECTION_DEC) {
            pdesc->route--;
            if (pdesc->ops.dec) {
                pdesc->ops.dec();
            }
        } else {
            pdesc->route++;
            if (pdesc->ops.add) {
                pdesc->ops.add();
            }
        }
    } else {
        pdesc->priv.hall_state = tmp;
        return false;
    }

    return true;
}
/**
 * @brief 每毫秒调用一次.计算速度
 * @param {hall_describe_t} *pdesc
 * @return {*}
 */
static void calcalateSpeed_ms(hall_describe_t *pdesc)
{
    uint16_t tmp = 0;
    if (pdesc->priv.pluse_width > 0) {
        pdesc->speed = 20000.0 / ((float)pdesc->priv.pluse_width);
        pdesc->priv.pluse_width = 0;
        pdesc->priv.no_pluse_time = 0;
    } else {
        if (pdesc->priv.no_pluse_time < 100) {
            pdesc->priv.no_pluse_time++;
        } else {
            pdesc->speed = 0;
        }
    }
}
#include "main.h"
/**
 * @brief 在200us的中断中执行，因为这里默认的时基是200us
 * @param {driver_t} *
 * @param {uint32_t} irq_handler
 * @param {void} *args
 * @param {uint32_t} length
 * @return {*}
 */
static int32_t _hall_irq_handler(driver_t **pdrv, uint32_t irq_handler, void *args, uint32_t length)
{
    hall_describe_t *pdesc = NULL;
    int32_t err = DRV_ERR_WRONG_ARGS;
    static uint8_t ms_count = 0;
    ASSERT(pdrv);
    pdesc = container_of(pdrv, device_t, pdrv)->pdesc;

    uint16_t tmp = 0;
    pdesc->priv.time_200us++;

    do {
        if (!pdesc) {
            break;
        }

        if (hall_phase_detect(pdesc)) {
            tmp = pdesc->priv.time_200us;
            switch (pdesc->priv.hall_state_last) {
                case HALL1:
                    pdesc->priv.pluse_width = tmp - pdesc->priv.p1_t0;
                    pdesc->priv.p1_t0 = tmp;
                    break;
                case HALL1 | HALL2:
                    pdesc->priv.pluse_width = tmp - pdesc->priv.p2_t0;
                    pdesc->priv.p2_t0 = tmp;
                    break;
                case HALL2:
                    pdesc->priv.pluse_width = tmp - pdesc->priv.p1_t1;
                    pdesc->priv.p1_t1 = tmp;
                    break;
                default:
                    pdesc->priv.pluse_width = tmp - pdesc->priv.p2_t1;
                    pdesc->priv.p2_t1 = tmp;
                    break;
            }
        }
    } while (0);

    if (ms_count < 4) {
        ms_count++;
    } else {
        ms_count = 0;
        calcalateSpeed_ms(pdesc);
    }
    return err;
}

static int32_t _hall_get_speed(hall_describe_t *pdesc, void *args)
{
    union hall_ioctl_param *param = (union hall_ioctl_param *)args;

    param->get.speed = pdesc->speed;

    return DRV_ERR_EOK;
}

static int32_t _hall_get_route(hall_describe_t *pdesc, void *args)
{
    union hall_ioctl_param *param = (union hall_ioctl_param *)args;

    param->get.route = pdesc->route;

    return DRV_ERR_EOK;
}
/*---------- end of file ----------*/
